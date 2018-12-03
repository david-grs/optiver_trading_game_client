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
	void OnMulticastMessage(Address, std::string) override;

	void OnOrderAck(std::string, Price, Volume);
	void OnPriceFeed(std::string, Price, Volume, Price, Volume);
	void OnTrade(std::string, std::string, Volume);
	void PrintPnl();

	UDPClient mInfoReceiver{"LOCAL_ADDRESS", 7000, *this};
	UDPClient mExecReceiver{"LOCAL_ADDRESS", 8000, *this};

	ExecutionClient mExecutionClient{"REMOTE_ADDRESS", 8001};

	std::map<std::string /*instrument feedcode*/, TopLevel> mLastBook;

	std::vector<std::pair<Price, Volume>> mESXTrades;
	std::vector<std::pair<Price, Volume>> mSPTrades;
};
