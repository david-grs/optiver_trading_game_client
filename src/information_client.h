#pragma once

#include "types.h"

#include <string>
#include <cstdint>

class InformationClient
{
public:
	InformationClient(std::string, uint16_t);

	void Subscribe();

private:
	int mFD;
	Address mRemote;
};
