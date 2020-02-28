#include "autotrader.h"
#include "trading_result.h"
#include "vwap.h"
#include "types.h"

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>

void Autotrader::Run()
{
	mInformationClient.Subscribe();

	while (true)
	{
		while (mInformationClient.Poll());
		while (mExecutionClient.Poll());
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

	auto& history = mLastBookHistory[feedcode];
	history[0] = history[1];
	history[1] = newBook;
	++mPriceCount[feedcode];

	RunStrategy(feedcode, history);
}

void Autotrader::OnTrade(const std::string feedcode, std::string side, Volume tradedVolume)
{
/*
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
		mExecutionClient.SendOrder(order);
	}
*/
}

void Autotrader::RunStrategy(const std::string& feedcode, const std::array<TopLevel, 2>& history)
{
	if (mPriceCount[feedcode] < 2)
	{
		// Need price history.
		return;
	}

	if (feedcode == "SP-FUTURE")
	{
		return;
	}

	auto& price1 = history[0];
	auto& price2 = history[1];

	if (mNextOrder)
	{
		if (mNextOrder->mAction == "SELL")
		{
			if (price2.mBidPrice.mValue >= price1.mBidPrice.mValue)
			{
				// Keep riding the wave.
				std::cout << "***** ride buy" << std::endl;
				mNextOrder->mPrice = price2.mBidPrice;
				return;
			}
			else
			{
				// Profit.
				std::cout << "***** close buy; sell @ " << mNextOrder->mPrice.mValue << std::endl;
				mExecutionClient.SendOrder(*mNextOrder);
				mNextOrder = boost::none;
			}
		}
		else
		{
			if (price2.mAskPrice.mValue <= price1.mAskPrice.mValue)
			{
				// Keep riding the wave.
				std::cout << "***** ride sell" << std::endl;
				mNextOrder->mPrice = price2.mAskPrice;
				return;
			}
			else
			{
				// Profit.
				std::cout << "***** close sell; buy @ " << mNextOrder->mPrice.mValue << std::endl;
				mExecutionClient.SendOrder(*mNextOrder);
				mNextOrder = boost::none;
			}
		}
	}

	if (price2.mBidPrice.mValue > price1.mAskPrice.mValue)
	{
		std::cout << "***** enter buy @ " << price1.mAskPrice.mValue << std::endl;
		mExecutionClient.SendOrder(OrderMessage{ feedcode, "BUY", price1.mAskPrice, Volume{1} });
		mNextOrder = OrderMessage{ feedcode, "SELL", price2.mBidPrice, Volume{1} };
	}
	else if (price1.mBidPrice.mValue > price2.mAskPrice.mValue)
	{
		std::cout << "***** enter sell @ " << price1.mBidPrice.mValue << std::endl;
		mExecutionClient.SendOrder(OrderMessage{ feedcode, "SELL", price1.mBidPrice, Volume{1} });
		mNextOrder = OrderMessage{ feedcode, "BUY", price2.mAskPrice, Volume{1} };
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
	boost::optional<TradingResult> esxResult, spResult;

	auto it = mLastBook.find("ESX-FUTURE");
	if (it != mLastBook.cend())
	{
		const Price esxValuation = CalcVWAP(it->second);
		esxResult = CalculateTradingResult(mESXTrades, esxValuation);
	}

	it = mLastBook.find("SP-FUTURE");
	if (it != mLastBook.cend())
	{
		const Price spValuation = CalcVWAP(it->second);
		spResult = CalculateTradingResult(mSPTrades, spValuation);
	}

	std::cout << "--------------------------\n"
			  << "ESX RESULT: " << esxResult << "\n"
			  << "SP RESULT: " << spResult << "\n"
			  << "--------------------------"
			  << std::endl;
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
