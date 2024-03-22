#include "tls_layer.h"
#include "botan/tls_server.h"

#include "iostream"

namespace nimlib::Server::Protocols
{
	TlsLayer::TlsLayer(std::stringstream& in, std::stringstream& out, std::shared_ptr<ProtocolInterface> next)
		: ProtocolInterface {in, out, next}
	{
		tls_server = nimlib::Server::Protocols::BotanSpec::get_tls_server(in, out);
	}

	TlsLayer::~TlsLayer() = default;

	ParseResult TlsLayer::parse()
	{
		try
		{
			std::string s{ in.str() };
			auto a = reinterpret_cast<uint8_t*>(s.data());
			auto b = tls_server->received_data(a, s.size());
			return ParseResult::WRITE_AND_WAIT;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			return ParseResult::WRITE_AND_WAIT;
		}
	}
}
