#include "credentials.h"


#include <botan/data_src.h>
#include <botan/pkcs8.h>
#include <botan/x509_key.h>
#include <botan/auto_rng.h>
#include <botan/tls_policy.h>
#include <botan/certstor_system.h>

#include <vector>

namespace nimlib::Server::Protocols::BotanSpec
{
	inline bool value_exists(const std::vector<std::string>& vec, const std::string& val)
	{
		for (size_t i = 0; i != vec.size(); ++i)
		{
			if (vec[i] == val)
			{
				return true;
			}
		}
		return false;
	}

	CredentialsProvider::CredentialsProvider(bool use_system_store,
		const std::string& ca_path,
		std::optional<std::string> client_cred,
		std::optional<std::string> client_key,
		std::optional<Botan::secure_vector<uint8_t>> psk,
		std::optional<std::string> psk_identity,
		const std::optional<std::string>& psk_prf)
		:
		m_psk(std::move(psk)),
		m_psk_identity(std::move(psk_identity)),

		// RFC 8446 4.2.11
		//    the Hash algorithm MUST be set when the PSK is established or
		//    default to SHA-256 if no such algorithm is defined.
		m_psk_prf(psk_prf.value_or("SHA-256"))
	{
		if (ca_path.empty() == false)
		{
			m_certstores.push_back(std::make_shared<Botan::Certificate_Store_In_Memory>(ca_path));
		}

		BOTAN_ARG_CHECK(client_cred.has_value() == client_key.has_value(),
			"either provide both client certificate and key or neither");

		if (client_cred.has_value() && client_key.has_value())
		{
			load_credentials(client_cred.value(), client_key.value());
		}

#if defined(BOTAN_HAS_CERTSTOR_SYSTEM)
		if (use_system_store)
		{
			m_certstores.push_back(std::make_shared<Botan::System_Certificate_Store>());
		}
#else
		BOTAN_UNUSED(use_system_store);
#endif
	}

	CredentialsProvider::CredentialsProvider(const std::string& server_cred,
		const std::string& server_key,
		std::optional<Botan::secure_vector<uint8_t>> psk,
		std::optional<std::string> psk_identity,
		const std::optional<std::string>& psk_prf)
		:
		m_psk(std::move(psk)),
		m_psk_identity(std::move(psk_identity)),

		// RFC 8446 4.2.11
		//    the Hash algorithm MUST be set when the PSK is established or
		//    default to SHA-256 if no such algorithm is defined.
		m_psk_prf(psk_prf.value_or("SHA-256"))
	{
		load_credentials(server_cred, server_key);
	}

	std::vector<Botan::Certificate_Store*> CredentialsProvider::trusted_certificate_authorities(const std::string& type,
		const std::string& /*hostname*/)
	{
		std::vector<Botan::Certificate_Store*> v;

		// don't ask for client certs
		if (type == "tls-server")
		{
			return v;
		}

		for (const auto& cs : m_certstores)
		{
			v.push_back(cs.get());
		}

		return v;
	}

	std::vector<Botan::X509_Certificate> CredentialsProvider::find_cert_chain(
		const std::vector<std::string>& algos,
		const std::vector<Botan::AlgorithmIdentifier>& cert_signature_schemes,
		const std::vector<Botan::X509_DN>& acceptable_cas,
		const std::string& type,
		const std::string& hostname)
	{
		BOTAN_UNUSED(cert_signature_schemes);

		if (type == "tls-client")
		{
			for (const auto& dn : acceptable_cas)
			{
				for (const auto& cred : m_certs)
				{
					if (dn == cred.certs[0].issuer_dn())
					{
						return cred.certs;
					}
				}
			}
		}
		else if (type == "tls-server")
		{
			for (const auto& i : m_certs)
			{
				if (std::find(algos.begin(), algos.end(), i.key->algo_name()) == algos.end())
				{
					continue;
				}

				if (!hostname.empty() && !i.certs[0].matches_dns_name(hostname))
				{
					continue;
				}

				return i.certs;
			}
		}

		return {};
	}

	std::shared_ptr<Botan::Public_Key> CredentialsProvider::find_raw_public_key(const std::vector<std::string>& algos,
		const std::string& type,
		const std::string& hostname)
	{
		BOTAN_UNUSED(type, hostname);

		return (algos.empty() || value_exists(algos, m_raw_pubkey.public_key->algo_name())) ? m_raw_pubkey.public_key
			: nullptr;
	}

	std::shared_ptr<Botan::Private_Key> CredentialsProvider::private_key_for(const Botan::X509_Certificate& cert,
		const std::string& /*type*/,
		const std::string& /*context*/)
	{
		for (const auto& i : m_certs)
		{
			if (cert == i.certs[0])
			{
				return i.key;
			}
		}

		return nullptr;
	}

	std::shared_ptr<Botan::Private_Key> CredentialsProvider::private_key_for(const Botan::Public_Key& raw_public_key,
		const std::string& /*type*/,
		const std::string& /*context*/)
	{
		return (m_raw_pubkey.public_key->fingerprint_public() == raw_public_key.fingerprint_public())
			? m_raw_pubkey.private_key
			: nullptr;
	}

	std::string CredentialsProvider::psk_identity(const std::string& type,
		const std::string& context,
		const std::string& identity_hint)
	{
		return (m_psk_identity && (type == "tls-client" || type == "tls-server"))
			? m_psk_identity.value()
			: Botan::Credentials_Manager::psk_identity(type, context, identity_hint);
	}

	std::vector<Botan::TLS::ExternalPSK> CredentialsProvider::find_preshared_keys(
		std::string_view host,
		Botan::TLS::Connection_Side peer_type,
		const std::vector<std::string>& ids,
		const std::optional<std::string>& prf)
	{
		if (!m_psk.has_value() || !m_psk_identity.has_value())
		{
			return Botan::Credentials_Manager::find_preshared_keys(host, peer_type, ids, prf);
		}

		std::vector<Botan::TLS::ExternalPSK> psks;

		const bool prf_matches = !prf.has_value() || m_psk_prf == prf.value();

		const bool id_matches = ids.empty() || std::find(ids.begin(), ids.end(), m_psk_identity.value()) != ids.end();

		if (prf_matches && id_matches)
		{
			psks.emplace_back(m_psk_identity.value(), m_psk_prf, m_psk.value());
		}

		return psks;
	}

	void CredentialsProvider::load_credentials(const std::string& cred, const std::string& key)
	{
		Botan::DataSource_Stream key_in(key);
		auto privkey = Botan::PKCS8::load_key(key_in);

		// first try to read @p cred as a public key
		try
		{
			auto pubkey = Botan::X509::load_key(cred);
			m_raw_pubkey = { std::exchange(privkey, {}), std::move(pubkey) };
			return;
		}
		catch (const Botan::Decoding_Error&)
		{
		}

		// ... then try again assuming that @p cred contains a certificate chain
		BOTAN_ASSERT_NONNULL(privkey);
		Certificate_Info cert;
		cert.key = std::move(privkey);

		Botan::DataSource_Stream in(cred);
		while (!in.end_of_data())
		{
			try
			{
				cert.certs.push_back(Botan::X509_Certificate(in));
			}
			catch (std::exception&)
			{
			}
		}

		BOTAN_ARG_CHECK(!cert.certs.empty(), "Provided certificate chain file cannot be loaded");

		// TODO: attempt to validate chain ourselves

		m_certs.push_back(cert);
	}
}
