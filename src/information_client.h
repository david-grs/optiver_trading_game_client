#pragma once

#include "types.h"

#include <string>
#include <cstdint>

class InformationClient
{
public:
	InformationClient(uint16_t localPort, std::string remoteHost, uint16_t remotePort);

	void Subscribe();

private:
	int mFD;
	Address mRemote;
};
