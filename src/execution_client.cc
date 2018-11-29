#include "execution_client.h"
#include "tsc_clock.h"

#include <cstring>
#include <stdexcept>

extern "C"
{
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
}

#include <iostream>

using namespace std::string_literals;

ExecutionClient::ExecutionClient(std::string address, uint16_t port)
{
	if ((mFD = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		throw std::runtime_error("socket() failed: "s + std::strerror(errno));
	}

	std::memset(&mRemote, 0, sizeof(mRemote));
	mRemote.sin_family = AF_INET;
	mRemote.sin_port = htons(port);
	mRemote.sin_addr.s_addr = inet_addr(address.data());

	if (mRemote.sin_addr.s_addr == INADDR_NONE)
	{
		throw std::runtime_error("inet_addr() failed: "s + std::strerror(errno));
	}
}

void ExecutionClient::Send(OrderMessage order)
{
	std::cout << "sending order, uername=" << order.mUsername
			  << " feedcode=" << order.mFeedcode
			  << " " << order.mAction << " " << order.mVolume.mValue << "@" << order.mPrice.mValue << std::endl;

	std::string message;
	message = "TYPE=ORDER|USERNAME=";
	message += order.mUsername;
	message += "|FEEDCODE=";
	message += order.mFeedcode;
	message += "|ACTION=";
	message += order.mAction;
	message += "|PRICE=";
	message += std::to_string(order.mPrice.mValue);
	message += "|VOLUME=";
	message += std::to_string(order.mVolume.mValue);

	SendSerializedMessage(order.mTimestampIn, message);
}

void ExecutionClient::SendSerializedMessage(TSCTimestamp timestampIn, std::string data)
{
	///////////////////////
	// NOTE: this block can be modified, as long as the timestamp measurement + latency
	// measurement calculation stay the LAST operations before sending the message!
	{
		const TSCTimestamp timestampOut{TSCClock::Now()};
		data += "|LATENCY=" + std::to_string(TSCClock::FromCycles(timestampOut.mValue - timestampIn.mValue).count());
	}
	//////////////////////

	if (::sendto(mFD, data.data(), data.size(), 0, (sockaddr*)&mRemote, sizeof(mRemote)) < 0)
	{
		throw std::runtime_error("sendto() failed:"s + std::strerror(errno));
	}
}
