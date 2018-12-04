# Optiver Master Class Game 

During this game, you will be developing an automated trading system!

The exchange is broadcasting prices and trades on two famous indices: S&P500 (SP) and Eurostoxx (ESX), and your goal is to seek for arbitrage opportunities between the two.

The system is composed by:

  * The exchange, broadcasting through UDP Multicast the public information messages
  * The exchange execution gateway, with which you will communicate in order to send orders &mdash; and hopefully trade!
  * Your automated trading client, that contains a prediction model between the two instruments


To help you a bit, we provide you the skeleton of an autotrader, with a prediction model between the S&P and ESX instruments, and a partial implementation
of the information protocol.

**All of you are trading on the same exchange, which means that you will compete for the same opportunities... In order to deal with competitive orders, the exchange is using a first come first serve policy!**

**Last thing: every trade will impact your position &mdash; keep an eye on it to manage your risk exposure!**

Good luck!





## Protocol specification
The communication from and towards the exchange is done via a text-based, ASCII, protocol.

Every message is specific to an *instrument*, identified by a *feedcode*. Example: `SP-FUTURE`.
Every message is composed by several fields separated by `|`.

### Info protocol specifications
The exchange broadcasts two messages (multicast UDP):

  * Price message: this tells you what is the current price and volume for an *instrument*
  * Trade message: this tells you what trades on the exchange, at which price and for how much volume


#### Price message fields

| Field | Description | Value
|------|-------------|-----------------
| TYPE | The type of message | Fixed value: `PRICE` |
| FEEDCODE | The instrument's feedcode | String | 
| BID_PRICE | The price at which you can sell | Float |
| BID_VOLUME | The volume available on the bid | Int |
| ASK_PRICE | The price at which you can buy |Float  |
| ASK_VOLUME | The volume available on the offer | Int |

Example: ```TYPE=PRICE|FEEDCODE=FOOBAR|BID_PRICE=10.0|BID_VOLUME=100|ASK_PRICE=11.0|ASK_VOLUME=20```


#### Trade message fields

| Field | Description | Value
--------|------------|----------------
| TYPE | The type of message | Fixed value: `TRADE` |
| FEEDCODE | The instrument's feedcode | String | 
| SIDE | The side of the trade | String, `BID` or `ASK` | 
| PRICE | The traded price | Float |
| VOLUME | The traded volume| Int |

Example: ```TYPE=TRADE|FEEDCODE=FOOBAR|SIDE=BID|PRICE=22.0|VOLUME=100```



### Execution protocol specification
In order to trade, you need to send an order to the exchange (via UDP). Every order is acknowledged by the exchange

#### Order message fields

| Field | Description | Value
-------|--------------|---------------
| TYPE | The type of message | Fixed value: `ORDER` |
| USERNAME | The username associated with the order | String |
| FEEDCODE | The instrument's feedcode | String |
| ACTION | The order action | String, `BUY` or `SELL` | 
| PRICE | The order price | Float |
| VOLUME | The order volume | Int |
| LATENCY | The latency of the order, in nanoseconds | Int | 

#### Order ack message fields

| Field | Description | Value
-------|-------------|----------------
| TYPE | The type of message | Fixed value: `ORDER_ACK` |
| FEEDCODE | The instrument's feedcode | String |
| PRICE | The traded price | Float |
| TRADED_VOLUME | The traded volume, positive if you bought, negative if you sold | Int, optional, only set if the order has been successfully processed |
| ERROR | The errord description | If something went wrong, this field is set, otherwise it is empty | String, optional, only set on error |

