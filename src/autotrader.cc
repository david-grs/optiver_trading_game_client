#include "autotrader.h"
#include "pnl.h"
#include "vwap.h"
#include "types.h"

#include <iostream>

void Autotrader::Run()
{
	while (true)
	{
		while (mInfoReceiver.Poll());
		while (mExecReceiver.Poll());
	}
}

double CalcVWAPChange(TopLevel level, Side tradedSide, Volume tradedVolume)
{
	const Price vwap = CalcVWAP(level);

	int& volume = tradedSide == Side::Bid ? level.mBidVolume.mValue : level.mAskVolume.mValue;
	volume = std::max(1, volume - tradedVolume.mValue);

	const Price newVWAP = CalcVWAP(level);
	return newVWAP.mValue - vwap.mValue;
}

double CalcVWAPPrediction(const TopLevel& level, Side tradedSide, Volume tradedVolume)
{
	return CalcVWAPChange(level, tradedSide, tradedVolume) * std::sqrt(tradedVolume.mValue / 10.0);
}

void Autotrader::OnMulticastMessage(Address, std::string message)
{
	std::cout << "received message: " << message << std::endl;

	// TODO

	PrintPnl();
}

void Autotrader::OnPriceFeed(std::string feedcode, Price bidPrice, Volume bidVolume, Price askPrice, Volume askVolume)
{
	const TopLevel newBook{bidPrice, bidVolume, askPrice, askVolume};
	mLastBook[feedcode] = newBook;
}

void Autotrader::OnTrade(std::string feedcode, std::string side, Volume tradedVolume)
{
	auto it = mLastBook.find("ESX-FUTURE");
	if (it == mLastBook.cend())
		return;

	TopLevel& tradedBook = it->second;
	Side tradedSide = side == "BID" ? Side::Bid : Side::Ask;

	std::string targetFeedcode = feedcode == "ESX-FUTURE" ? "SP-FUTURE" : "ESX-FUTURE";
	it = mLastBook.find(targetFeedcode);
	if (it == mLastBook.cend())
		return;

	TopLevel& targetBook = it->second;

	Price targetVWAP = CalcVWAP(targetBook);
	double vwapChange = CalcVWAPPrediction(tradedBook, tradedSide, tradedVolume);
	Price targetPrice = tradedSide == Side::Bid ? targetBook.mBidPrice : targetBook.mAskPrice;

	if ((tradedSide == Side::Bid && targetPrice.mValue > targetVWAP.mValue + vwapChange)
		|| (tradedSide == Side::Ask && targetPrice.mValue < targetVWAP.mValue + vwapChange))
	{
		// TODO send order
	}
}

void Autotrader::OnOrderAck(std::string feedcode, Price tradedPrice, Volume tradedVolume)
{
	if (feedcode == "ESX-FUTURE")
	{
		mESXTrades.emplace_back(tradedPrice, tradedVolume);
	}
	else if (feedcode == "SP-FUTURE")
	{
		mSPTrades.emplace_back(tradedPrice, tradedVolume);
	}
}

void Autotrader::PrintPnl()
{
	auto it = mLastBook.find("ESX-FUTURE");
	if (it != mLastBook.cend())
	{
		const TopLevel& book = it->second;
		std::cout << "pnl_esx=" << CalculatePnL(mESXTrades, book) << std::endl;
	}

	it = mLastBook.find("SP-FUTURE");
	if (it != mLastBook.cend())
	{
		const TopLevel& book = it->second;
		std::cout << "pnl_sp=" << CalculatePnL(mSPTrades, book) << std::endl;
	}
}

