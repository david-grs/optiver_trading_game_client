#pragma once

#include "udp_client.h"
#include "order_message.h"

#include <string>
#include <cstdint>

class ExecutionClient : public UDPClient
{
public:
	ExecutionClient(uint16_t localPort,
					std::string remoteHost,
					uint16_t remotePort,
					IUDPClientHandler& handler);

	void SendOrder(OrderMessage);

private:
	void SendSerializedMessage(std::string);

	int mFD;
	Address mRemote;
};
