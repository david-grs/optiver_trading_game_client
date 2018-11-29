#pragma once

extern "C"
{
#include <netinet/in.h>
}

#include <cstdint>

using Address = sockaddr_in;

struct TSCTimestamp { uint64_t mValue; };
struct Price { double mValue; };
struct Volume { int mValue; };

enum struct Side { Bid, Ask };

struct TopLevel
{
	TopLevel(Price bidPrice, Volume bidVolume, Price askPrice, Volume askVolume) :
		mBidPrice(bidPrice), mBidVolume(bidVolume), mAskPrice(askPrice), mAskVolume(askVolume)
	{}

	Price mBidPrice;
	Volume mBidVolume;
	Price mAskPrice;
	Volume mAskVolume;
};

