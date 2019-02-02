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

InformationClient::InformationClient(std::string address, uint16_t port)
{
	if ((mFD = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		throw std::runtime_error("information client: socket() failed: "s + std::strerror(errno));
	}

	std::memset(&mRemote, 0, sizeof(mRemote));
	mRemote.sin_family = AF_INET;
	mRemote.sin_port = htons(port);
	mRemote.sin_addr.s_addr = ::inet_addr(address.data());

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
