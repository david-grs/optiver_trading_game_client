#pragma once

#include "execution_client.h"
#include "udp_client.h"

#include <string>
#include <vector>
#include <utility>
#include <map>

class Autotrader : public IUDPClientHandler
{
public:
	void Run();

private:
	// [IUDPClientHandler]
	void OnMulticastMessage(TSCTimestamp, Address, std::string) override;

	void OnOrderAck(std::string, Price, Volume);
	void OnPriceFeed(std::string, Price, Volume, Price, Volume);
	void OnTrade(std::string, std::string, Volume);
	void PrintPnl();

	UDPClient mInfoReceiver{"224.0.0.1", 9000, *this};
	UDPClient mExecReceiver{"127.0.0.1", 8001, *this};

	ExecutionClient mExecutionClient{"127.0.0.1", 8000};

	std::map<std::string /*instrument feedcode*/, TopLevel> mLastBook;

	std::vector<std::pair<Price, Volume>> mESXTrades;
	std::vector<std::pair<Price, Volume>> mSPTrades;
};
