#pragma once

#include "order_message.h"
#include "types.h"

#include <string>
#include <cstdint>

class ExecutionClient
{
public:
	ExecutionClient(std::string, uint16_t);

	void Send(OrderMessage);

private:
	void SendSerializedMessage(std::string);

	int mFD;
	Address mRemote;
};
