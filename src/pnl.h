#pragma once

#include "types.h"

#include <numeric>
#include <vector>
#include <algorithm>

inline double CalculatePnL(const std::vector<std::pair<Price, Volume>>& trades, const TopLevel& level)
{
	int buyVolume = std::accumulate(trades.cbegin(), trades.cend(), 0, [&](double x, const auto& p) { return x + std::max(0, p.second.mValue); });
	int sellVolume = std::accumulate(trades.cbegin(), trades.cend(), 0, [&](double x, const auto& p) { return x - std::min(0, p.second.mValue); });

	double buyAvg = buyVolume == 0 ? 0 : std::accumulate(trades.cbegin(), trades.cend(), .0, [&](double x, const auto& p) { return x + p.first.mValue * std::max(0, p.second.mValue); }) / buyVolume;
	double sellAvg = sellVolume == 0 ? 0 : std::accumulate(trades.cbegin(), trades.cend(), .0, [&](double x, const auto& p) { return x - p.first.mValue * std::min(0, p.second.mValue); }) / sellVolume;

	int closedVolume  = std::min(buyVolume, sellVolume);
	double closedPnl = (sellAvg - buyAvg) * closedVolume;

	bool openBuy = buyVolume != closedVolume;
	int openVolume = openBuy ? buyVolume - closedVolume : sellVolume - closedVolume;
	double openPnl = (openBuy ? level.mBidPrice.mValue - buyAvg : sellAvg - level.mAskPrice.mValue) * openVolume;

	return closedPnl + openPnl;
}

