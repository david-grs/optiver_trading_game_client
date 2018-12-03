#include "autotrader.h"
#include "pnl.h"
#include "vwap.h"
#include "types.h"

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>

void Autotrader::Run()
{
	while (true)
	{
		while (mInfoReceiver.Poll());
		while (mExecReceiver.Poll());
	}
}

void Autotrader::OnMulticastMessage(Address, std::string message)
{
	std::cout << "received message: " << message << std::endl;

	std::vector<std::string> fieldsStr;
	boost::split(fieldsStr, message, boost::is_any_of("|"));

	std::map<std::string, std::string> fields;
	for (std::string fieldStr : fieldsStr)
	{
		std::vector<std::string> keyValue;
		boost::split(keyValue, fieldStr, boost::is_any_of("="));

		if (keyValue.size() == 2)
		{
			fields[keyValue[0]] = keyValue[1];
		}
	}

	if (fields["TYPE"] == "PRICE")
	{
		std::string feedcode = fields["FEEDCODE"];
		Price bidPrice{boost::lexical_cast<double>(fields["BID_PRICE"])};
		Volume bidVolume{boost::lexical_cast<int>(fields["BID_VOLUME"])};
		Price askPrice{boost::lexical_cast<double>(fields["ASK_PRICE"])};
		Volume askVolume{boost::lexical_cast<int>(fields["ASK_VOLUME"])};

		OnPriceFeed(feedcode, bidPrice, bidVolume, askPrice, askVolume);
	}
	// TODO TYPE TRADE
	// TODO TYPE ORDER_ACK

	PrintPnl();
}

void Autotrader::OnPriceFeed(std::string feedcode, Price bidPrice, Volume bidVolume, Price askPrice, Volume askVolume)
{
	std::cout << feedcode << ": "
			  << bidVolume.mValue << "@" << bidPrice.mValue << "x"
			  << askVolume.mValue << "@" << askPrice.mValue << std::endl;

	const TopLevel newBook{bidPrice, bidVolume, askPrice, askVolume};
	mLastBook[feedcode] = newBook;
}

void Autotrader::OnTrade(std::string feedcode, std::string side, Volume tradedVolume)
{
	auto it = mLastBook.find(feedcode);
	if (it == mLastBook.cend())
		return;

	TopLevel& tradedBook = it->second;
	Side tradedSide = side == "BID" ? Side::Bid : Side::Ask;

	std::string action = tradedSide == Side::Bid ? "SELL" : "BUY";
	std::string targetFeedcode = feedcode == "ESX-FUTURE" ? "SP-FUTURE" : "ESX-FUTURE";

	it = mLastBook.find(targetFeedcode);
	if (it == mLastBook.cend())
		return;

	TopLevel& targetBook = it->second;
	Price targetVWAP = CalcVWAP(targetBook);

	double vwapChange = CalcVWAPPrediction(tradedBook, tradedSide, tradedVolume);
	Price targetPrice = action == "BUY" ? targetBook.mAskPrice : targetBook.mBidPrice;

	if ((action == "SELL" && targetPrice.mValue > targetVWAP.mValue + vwapChange)
		|| (action == "BUY" && targetPrice.mValue < targetVWAP.mValue + vwapChange))
	{
		OrderMessage order{targetFeedcode, action, targetPrice, Volume{10}};
		mExecutionClient.Send(order);
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
	boost::optional<double> pnlESX, pnlSP;

	auto it = mLastBook.find("ESX-FUTURE");
	if (it != mLastBook.cend())
	{
		pnlESX = CalculatePnL(mESXTrades, it->second);
	}

	it = mLastBook.find("SP-FUTURE");
	if (it != mLastBook.cend())
	{
		pnlSP = CalculatePnL(mSPTrades, it->second);
	}

	std::cout << "pnl_esx=" << pnlESX << ", pnl_sp=" << pnlSP << std::endl;
}

double Autotrader::CalcVWAPChange(TopLevel level, Side tradedSide, Volume tradedVolume)
{
	const Price vwap = CalcVWAP(level);

	int& volume = tradedSide == Side::Bid ? level.mBidVolume.mValue : level.mAskVolume.mValue;
	volume = std::max(1, volume - tradedVolume.mValue);

	const Price newVWAP = CalcVWAP(level);
	return newVWAP.mValue - vwap.mValue;
}

double Autotrader::CalcVWAPPrediction(TopLevel level, Side tradedSide, Volume tradedVolume)
{
	return CalcVWAPChange(level, tradedSide, tradedVolume) * std::sqrt(tradedVolume.mValue / 10.0);
}
