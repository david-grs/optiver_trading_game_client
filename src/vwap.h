#pragma once

#include "types.h"

#include <stdexcept>
#include <cmath>

inline Price CalcVWAP(const TopLevel& level)
{
	const Price vwap{(level.mBidPrice.mValue * level.mAskVolume.mValue + level.mAskPrice.mValue * level.mBidVolume.mValue)
					/ (level.mBidVolume.mValue + level.mAskVolume.mValue)};

	if (!std::isfinite(vwap.mValue))
	{
		throw std::overflow_error("unexpected non finite number as vwap");
	}

	return vwap;
}
