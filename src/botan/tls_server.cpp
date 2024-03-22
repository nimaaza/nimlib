#include "tls_server.h"

#include <botan/auto_rng.h>
#include <botan/tls_session_manager_memory.h>

#include "credentials.h"
#include "tls_policy.h"
#include "callbacks.h"

namespace nimlib::Server::Protocols::BotanSpec
{
	std::unique_ptr<Botan::TLS::Server> get_tls_server(const std::stringstream& in, std::stringstream& out)
	{
		const std::string crt_path = "../cert/server.crt";
		const std::string key_path = "../cert/server.key";

		auto callbacks = std::make_shared<Callbacks>(in, out);
		auto rng = std::make_shared<Botan::AutoSeeded_RNG>();
		auto session_mgr = std::make_shared<Botan::TLS::Session_Manager_In_Memory>(rng);
		auto credentials = std::make_shared<CredentialsProvider>(crt_path, key_path);
		auto policy = std::make_shared<TLS_All_Policy>();
		return std::move(std::make_unique<Botan::TLS::Server>(callbacks, session_mgr, credentials, policy, rng));
	}
}
