#include "udp_client.h"
#include "tsc_clock.h"

#include <cstring>
#include <stdexcept>
#include <cerrno>

extern "C"
{
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
}

using namespace std::string_literals;

extern TSCTimestamp TimestampIn;

UDPClient::UDPClient(std::string group, uint16_t port, IUDPClientHandler& handler) :
	mHandler(handler)
{
	if ((mSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		throw std::runtime_error("socket() failed: "s + std::strerror(errno));
	}

	Address address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = ::inet_addr(group.data());

	if ((::bind(mSocket, (struct sockaddr *)&address, sizeof(address))) < 0)
	{
		throw std::runtime_error("bind() failed: "s + std::strerror(errno));
	}
}

bool UDPClient::Poll()
{
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(mSocket, &readfds);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 1;

	const int res = ::select(mSocket + 1, &readfds, nullptr, nullptr, &timeout);
	if (res == -1)
	{
		throw std::runtime_error("select() failed: "s + std::strerror(errno));
	}

	if (res == 0 || !FD_ISSET(mSocket, &readfds))
	{
		return false;
	}

	Address clientAddress;
	std::memset(&clientAddress, 0, sizeof(clientAddress));
	socklen_t addressLength = sizeof(clientAddress);

	char buffer[1024];
	const auto bytes_received = ::recvfrom(mSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddress, &addressLength);
	if (bytes_received < 0)
	{
		throw std::runtime_error("recvfrom() failed:"s + std::strerror(errno));
	}

	///////////////////////
	// NOTE: do NOT edit or move this line
	TimestampIn = TSCTimestamp{TSCClock::Now()};
	//////////////////////

	const std::string message{buffer, static_cast<std::size_t>(bytes_received)};
	mHandler.OnMulticastMessage(clientAddress, message);

	return true;
}
