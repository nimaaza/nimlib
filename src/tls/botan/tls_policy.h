#pragma once

#include <botan/tls_policy.h>

namespace nimlib::Server::Handlers::BotanSpec
{
	class TLS_All_Policy final : public Botan::TLS::Policy
	{
	public:
		std::vector<std::string> allowed_ciphers() const override
		{
			return std::vector<std::string>{
				"ChaCha20Poly1305",
					"AES-256/OCB(12)",
					"AES-128/OCB(12)",
					"AES-256/GCM",
					"AES-128/GCM",
					"AES-256/CCM",
					"AES-128/CCM",
					"AES-256/CCM(8)",
					"AES-128/CCM(8)",
					"Camellia-256/GCM",
					"Camellia-128/GCM",
					"ARIA-256/GCM",
					"ARIA-128/GCM",
					"AES-256",
					"AES-128",
					"Camellia-256",
					"Camellia-128",
					"SEED",
					"3DES"
			};
		}

		std::vector<std::string> allowed_key_exchange_methods() const override
		{
			return { "ECDHE_PSK", "DHE_PSK", "PSK", "ECDH", "DH", "RSA" };
		}

		std::vector<std::string> allowed_signature_methods() const override
		{
			return { "ECDSA", "RSA", "DSA", "IMPLICIT" };
		}

		bool allow_tls12() const override
		{
			return true;
		}
	};
};
