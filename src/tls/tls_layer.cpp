#include "tls_layer.h"
#include "botan/botan_tls_server.h"

namespace nimlib::Server::Handlers
{
	TlsLayer::TlsLayer(std::shared_ptr<Handler> next) : next{ next } {}

	void TlsLayer::notify(Connection& connection, StreamsProvider& streams)
	{
		try
		{
			if (!tls_server)
			{
				tls_server = nimlib::Server::Handlers::BotanSpec::get_tls_server(connection, *this, next, streams, *this);
			}

			std::stringstream& encrypted_input{ streams.source() };
			std::string in_string{ encrypted_input.str() };
			if (in_string.size() > 0)
			{
				encrypted_input.str("");
				auto in_string_ptr = reinterpret_cast<uint8_t*>(in_string.data());
				auto bytes_needed = tls_server->received_data(in_string_ptr, in_string.size());
				tls_continue = bytes_needed > 0;
				connection.notify(*this);
			}
			else
			{
				next->notify(*this, connection, *this);
			}
		}
		catch (const std::exception& e)
		{
			// TODO: connection needs to be set in error mode in this case and closed.
			connection.notify(*this);
		}
	}

	void TlsLayer::notify(Handler& handler, Connection& connection, StreamsProvider& streams)
	{
		if (handler.wants_to_write() || handler.wants_to_live() || handler.wants_to_be_calledback())
		{
			tls_server->send(streams.sink().str());
		}

		tls_server->close();
	}

	bool TlsLayer::wants_more_bytes() { return tls_continue || next->wants_more_bytes(); }

	bool TlsLayer::wants_to_write() { return tls_continue || next->wants_to_write(); }

	bool TlsLayer::wants_to_live() { return tls_continue || next->wants_to_live(); }

	bool TlsLayer::wants_to_be_calledback() { return next->wants_to_be_calledback(); }

	HandlerState TlsLayer::get_state() { return state_manager.get_state(); }

	std::stringstream& TlsLayer::source() { return decrypted_input; }

	std::stringstream& TlsLayer::sink() { return decrypted_output; }
}
