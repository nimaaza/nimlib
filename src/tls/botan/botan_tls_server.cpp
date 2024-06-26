#include "botan_tls_server.h"

#include <botan/auto_rng.h>
#include <botan/tls_session_manager_memory.h>

#include "credentials.h"
#include "tls_policy.h"
#include "callbacks.h"

namespace nimlib::Server::Handlers::BotanSpec
{
    using nimlib::Server::Handlers::BotanSpec::Callbacks;
    using nimlib::Server::Handlers::BotanSpec::CredentialsProvider;
    using nimlib::Server::Handlers::BotanSpec::TLS_All_Policy;

    std::unique_ptr<Botan::TLS::Server> get_tls_server(
        Connection& connection,
        Handler& tls_layer,
        std::shared_ptr<Handler> next,
        StreamsProvider& encrypted_streams,
        StreamsProvider& decrypted_streams
    )
    {
        static const std::string crt_path = "../cert/server.crt";
        static const std::string key_path = "../cert/server.key";

        static auto rng = std::make_shared<Botan::AutoSeeded_RNG>();
        static auto session_mgr = std::make_shared<Botan::TLS::Session_Manager_In_Memory>(rng);
        static auto credentials = std::make_shared<CredentialsProvider>(crt_path, key_path);
        static auto policy = std::make_shared<TLS_All_Policy>();

        auto callbacks = std::make_shared<Callbacks>(connection, tls_layer, next, encrypted_streams, decrypted_streams);
        return std::move(std::make_unique<Botan::TLS::Server>(callbacks, session_mgr, credentials, policy, rng));
    }
}
