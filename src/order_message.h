#pragma once

#include "types.h"

#include <string>

struct OrderMessage
{
	TSCTimestamp mTimestampIn;
	std::string mUsername;
	std::string mFeedcode;
	std::string mAction;
	Price mPrice;
	Volume mVolume;
};
