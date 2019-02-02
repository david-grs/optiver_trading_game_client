#pragma once

#include "order_message.h"
#include "types.h"

#include <string>
#include <cstdint>

class ExecutionClient
{
public:
	ExecutionClient(uint16_t localPort, std::string remoteHost, uint16_t remotePort);

	void Send(OrderMessage);

private:
	void SendSerializedMessage(std::string);

	int mFD;
	Address mRemote;
};
