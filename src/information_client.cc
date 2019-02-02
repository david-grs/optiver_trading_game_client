#include "information_client.h"

InformationClient::InformationClient(uint16_t localPort,
									 std::string remoteHost,
									 uint16_t remotePort,
									 IUDPClientHandler& handler) :
	UDPClient(localPort, remoteHost, remotePort, handler)
{}

void InformationClient::Subscribe()
{
	const std::string message = "TYPE=SUBSCRIPTION_REQUEST";
	Send(message);
}
