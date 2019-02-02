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

UDPClient::UDPClient(uint16_t localPort, std::string remoteHost, uint16_t remotePort, IUDPClientHandler& handler) :
	mHandler(handler)
{
	if ((mSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		throw std::runtime_error("information client: socket() failed: "s + std::strerror(errno));
	}

	Address sourceAddr;
	std::memset(&sourceAddr, 0, sizeof(sourceAddr));
	sourceAddr.sin_family = AF_INET;
	sourceAddr.sin_port = htons(localPort);
	sourceAddr.sin_addr.s_addr =  htonl(INADDR_ANY);

	if (::bind(mSocket, (sockaddr*)&sourceAddr, sizeof(sourceAddr)) < 0)
	{
		throw std::runtime_error("information client: bind() failed: "s + std::strerror(errno));
	}

	std::memset(&mRemote, 0, sizeof(mRemote));
	mRemote.sin_family = AF_INET;
	mRemote.sin_port = htons(remotePort);
	mRemote.sin_addr.s_addr = ::inet_addr(remoteHost.data());

	if (mRemote.sin_addr.s_addr == INADDR_NONE)
	{
		throw std::runtime_error("information client: inet_addr() failed: "s + std::strerror(errno));
	}
}

UDPClient::~UDPClient()
{}

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
		throw std::runtime_error("udp client: select() failed: "s + std::strerror(errno));
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
		throw std::runtime_error("udp client: recvfrom() failed:"s + std::strerror(errno));
	}

	///////////////////////
	// NOTE: do NOT edit or move this line
	TimestampIn = TSCTimestamp{TSCClock::Now()};
	//////////////////////

	const std::string message{buffer, static_cast<std::size_t>(bytes_received)};
	mHandler.OnMulticastMessage(clientAddress, message);

	return true;
}

void UDPClient::Send(std::string data)
{
	if (::sendto(mSocket, data.data(), data.size(), 0, (sockaddr*)&mRemote, sizeof(mRemote)) < 0)
	{
		throw std::runtime_error("udp client: sendto() failed:"s + std::strerror(errno));
	}
}

