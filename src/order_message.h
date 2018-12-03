#pragma once

#include "types.h"

#include <string>

struct OrderMessage
{
	std::string mFeedcode;
	std::string mAction;
	Price mPrice;
	Volume mVolume;
};
