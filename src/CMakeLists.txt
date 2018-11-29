set(sources
	autotrader.h
	autotrader.cc
	autotrader_main.cc
	execution_client.h
	execution_client.cc
	order_message.h
	pnl.h
	tsc_clock.h
	types.h
	udp_client.h
	udp_client.cc
	vwap.h
)

add_executable(autotrader ${sources})

find_package(Boost 1.58 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS}) 
target_link_libraries(autotrader ${Boost_LIBRARIES})

