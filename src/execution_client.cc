#include "execution_client.h"
#include "tsc_clock.h"

#include <cstring>
#include <cerrno>
#include <stdexcept>

extern "C"
{
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
}

#include <iostream>

using namespace std::string_literals;

extern TSCTimestamp TimestampIn;

///////////////////////
// NOTE: do NOT edit this line
static const std::string Username = "USERNAME";
///////////////////////

ExecutionClient::ExecutionClient(uint16_t localPort,
								 std::string remoteHost,
								 uint16_t remotePort,
								 IUDPClientHandler& handler) :
	UDPClient(localPort, remoteHost, remotePort, handler)
{}

void ExecutionClient::SendOrder(OrderMessage order)
{
	std::cout << "sending order, " << order.mAction << " "
			  << order.mVolume.mValue << "@" << order.mPrice.mValue << " "
			  << order.mFeedcode << std::endl;

	std::string message;
	message = "TYPE=ORDER|USERNAME=";
	message += "Team01";
	message += "|PASSWORD=";
	message += "KQMCxSEd";
	message += "|FEEDCODE=";
	message += order.mFeedcode;
	message += "|ACTION=";
	message += order.mAction;
	message += "|PRICE=";
	message += std::to_string(order.mPrice.mValue);
	message += "|VOLUME=";
	message += std::to_string(order.mVolume.mValue);

	SendSerializedMessage(message);
}

void ExecutionClient::SendSerializedMessage(std::string data)
{
	///////////////////////
	// NOTE: this block can be modified, as long as the timestamp measurement stay
	// the LAST operation before calling sendto()
	//{
	//	const TSCTimestamp timestampOut{TSCClock::Now()};
	//	data += "|LATENCY=" + std::to_string(TSCClock::FromCycles(timestampOut.mValue - TimestampIn.mValue).count());
	//}
	//////////////////////

	Send(data);
}
