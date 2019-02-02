#pragma once

#include "types.h"

#include <string>
#include <cstdint>

class IUDPClientHandler
{
public:
	virtual ~IUDPClientHandler() =default;
	virtual void OnMulticastMessage(Address, std::string) =0;
};

class UDPClient
{
public:
	UDPClient(uint16_t localPort, std::string remoteHost, uint16_t remotePort, IUDPClientHandler& handler);
	virtual ~UDPClient();

	bool Poll();
	void Send(std::string);

private:
	IUDPClientHandler& mHandler;
	Address mRemote;
	int mSocket;
};
