#pragma once

#include "order_message.h"
#include "types.h"

#include <string>
#include <cstdint>

extern "C"
{
#include <netinet/in.h>
}

class ExecutionClient
{
public:
	ExecutionClient(std::string, uint16_t);

	void Send(OrderMessage);

private:
	void SendSerializedMessage(TSCTimestamp, std::string);

	int mFD;
	sockaddr_in mRemote;
};
