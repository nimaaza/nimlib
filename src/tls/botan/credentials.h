#pragma once

#include <optional>

#include <botan/credentials_manager.h>

namespace nimlib::Server::Handlers::BotanSpec
{
	class CredentialsProvider : public Botan::Credentials_Manager
	{
	public:
		CredentialsProvider(
			bool use_system_store,
			const std::string& ca_path,
			std::optional<std::string> client_cred = std::nullopt,
			std::optional<std::string> client_key = std::nullopt,
			std::optional<Botan::secure_vector<uint8_t>> psk = std::nullopt,
			std::optional<std::string> psk_identity = std::nullopt,
			const std::optional<std::string>& psk_prf = std::nullopt);

		CredentialsProvider(
			const std::string& server_cred,
			const std::string& server_key,
			std::optional<Botan::secure_vector<uint8_t>> psk = std::nullopt,
			std::optional<std::string> psk_identity = std::nullopt,
			const std::optional<std::string>& psk_prf = std::nullopt);

		std::vector<Botan::Certificate_Store*> trusted_certificate_authorities(
			const std::string& type,
			const std::string& /*hostname*/) override;

		std::vector<Botan::X509_Certificate> find_cert_chain(
			const std::vector<std::string>& algos,
			const std::vector<Botan::AlgorithmIdentifier>& cert_signature_schemes,
			const std::vector<Botan::X509_DN>& acceptable_cas,
			const std::string& type,
			const std::string& hostname) override;

		std::shared_ptr<Botan::Public_Key> find_raw_public_key(
			const std::vector<std::string>& algos,
			const std::string& type,
			const std::string& hostname) override;

		std::shared_ptr<Botan::Private_Key> private_key_for(
			const Botan::X509_Certificate& cert,
			const std::string& /*type*/,
			const std::string& /*context*/) override;

		std::shared_ptr<Botan::Private_Key> private_key_for(
			const Botan::Public_Key& raw_public_key,
			const std::string& /*type*/,
			const std::string& /*context*/) override;

		std::string psk_identity(
			const std::string& type,
			const std::string& context,
			const std::string& identity_hint) override;

		std::vector<Botan::TLS::ExternalPSK> find_preshared_keys(
			std::string_view host,
			Botan::TLS::Connection_Side peer_type,
			const std::vector<std::string>& ids = {},
			const std::optional<std::string>& prf = std::nullopt) override;

	protected:
		void load_credentials(const std::string& cred, const std::string& key);

	private:
		struct Certificate_Info
		{
			std::vector<Botan::X509_Certificate> certs;
			std::shared_ptr<Botan::Private_Key> key;
		};

		struct RawPublicKey_Info
		{
			std::shared_ptr<Botan::Private_Key> private_key;
			std::shared_ptr<Botan::Public_Key> public_key;
		};

		std::vector<Certificate_Info> m_certs;
		RawPublicKey_Info m_raw_pubkey;

		std::vector<std::shared_ptr<Botan::Certificate_Store>> m_certstores;
		std::optional<Botan::secure_vector<uint8_t>> m_psk;
		std::optional<std::string> m_psk_identity;
		std::string m_psk_prf;
	};
}
