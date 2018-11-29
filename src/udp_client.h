#pragma once

#include "types.h"

#include <string>
#include <cstdint>

class IUDPClientHandler
{
public:
	virtual ~IUDPClientHandler() =default;

	virtual void OnMulticastMessage(TSCTimestamp, Address, std::string) =0;
};

class UDPClient
{
public:
	UDPClient(const std::string&, uint16_t, IUDPClientHandler&);

	bool Poll();

private:
	IUDPClientHandler& mHandler;
	int mSocket;
};
