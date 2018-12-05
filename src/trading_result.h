#pragma once

#include "types.h"

#include <numeric>
#include <vector>
#include <cmath>
#include <ostream>

struct TradingResult
{
	double mPnL;
	double mLockedPnL;
	int mPosition;
	int mTradedVolume;
};

inline std::ostream& operator<<(std::ostream& stream, const TradingResult& res)
{
	return stream << "pnl=" << res.mPnL << " locked_pnl=" << res.mLockedPnL << " position=" << res.mPosition << " traded_volume=" << res.mTradedVolume;
}

inline double CalculatePnL(const std::vector<std::pair<Price, Volume>>& trades, Price valuation)
{
	return std::accumulate(trades.cbegin(), trades.cend(), .0, [valuation](double pnl, const auto& p)
	{
		const int volume = p.second.mValue;
		const double price = p.first.mValue;
		return pnl + volume * (valuation.mValue - price);
	});
}

inline double CalculateLockedPnL(const std::vector<std::pair<Price, Volume>>& trades)
{
	const int buyVolume = std::accumulate(trades.cbegin(), trades.cend(), 0, [&](double x, const auto& p) { return x + std::max(0, (int)p.second.mValue); });
	const int sellVolume = std::accumulate(trades.cbegin(), trades.cend(), 0, [&](double x, const auto& p) { return x - std::min(0, (int)p.second.mValue); });

	const double buyAvg = buyVolume == 0 ? 0 : std::accumulate(trades.cbegin(), trades.cend(), .0, [&](double x, const auto& p) { return x + p.first.mValue * std::max(0, (int)p.second.mValue); }) / buyVolume;
	const double sellAvg = sellVolume == 0 ? 0 : std::accumulate(trades.cbegin(), trades.cend(), .0, [&](double x, const auto& p) { return x - p.first.mValue * std::min(0, (int)p.second.mValue); }) / sellVolume;

	const int closedVolume  = std::min(buyVolume, sellVolume);
	return (sellAvg - buyAvg) * closedVolume;
}

inline int CalculatePosition(const std::vector<std::pair<Price, Volume>>& trades)
{
	return std::accumulate(trades.cbegin(), trades.cend(), 0, [](int position, const auto& p)
	{
		const int volume = p.second.mValue;
		return position + volume;
	});
}

inline int CalculateTradedVolume(const std::vector<std::pair<Price, Volume>>& trades)
{
	return std::accumulate(trades.cbegin(), trades.cend(), 0, [](int position, const auto& p)
	{
		const int absVolume = std::abs(p.second.mValue);
		return position + absVolume;
	});
}

inline TradingResult CalculateTradingResult(const std::vector<std::pair<Price, Volume>>& trades, Price valuation)
{
	return TradingResult
	{
		CalculatePnL(trades, valuation),
		CalculateLockedPnL(trades),
		CalculatePosition(trades),
		CalculateTradedVolume(trades)
	};
}

