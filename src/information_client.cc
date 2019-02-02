#include "information_client.h"

#include <cstring>
#include <cerrno>
#include <stdexcept>

extern "C"
{
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
}

using namespace std::string_literals;

InformationClient::InformationClient(uint16_t localPort, std::string remoteHost, uint16_t remotePort)
{
	if ((mFD = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		throw std::runtime_error("information client: socket() failed: "s + std::strerror(errno));
	}

	Address sourceAddr;
	std::memset(&sourceAddr, 0, sizeof(sourceAddr));
	sourceAddr.sin_family = AF_INET;
	sourceAddr.sin_port = htons(localPort);
	sourceAddr.sin_addr.s_addr =  htonl(INADDR_ANY);

	if (::bind(mFD, (sockaddr*)&sourceAddr, sizeof(sourceAddr)) < 0)
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

void InformationClient::Subscribe()
{
	const std::string message = "TYPE=SUBSCRIPTION_REQUEST";

	if (::sendto(mFD, message.data(), message.size(), 0, (sockaddr*)&mRemote, sizeof(mRemote)) < 0)
	{
		throw std::runtime_error("information client: sendto() failed:"s + std::strerror(errno));
	}
}
