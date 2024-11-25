#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include "quant.h"

int main()
{
    std::string key, secret;
    key = "";
    secret = "";

    std::vector<std::string> tickers = {"SPY","AAPL","MSFT","TSLA","WMT","AMZN"};

    quant hft(key, secret, tickers);

    std::thread feed(hft.Socket, &hft);

    while(true){
        if(hft.sync() == true){
            for(auto & tk : tickers){
                std::cout << tk << " | " << hft.StockPrice[tk] << std::endl;
            }
        } else {
            std::cout << "Still loading stocks....." << std::endl;
        }
    }

    feed.join();

    return 0;
}