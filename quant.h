#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <cpprest/ws_client.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace web;
using namespace web::websockets::client;
using namespace boost::property_tree;

class quant {

    private:
        std::string key_;
        std::string secret_;
        std::vector<std::string> stocks;

        void Cyclone(std::string message){
            std::stringstream ss(message);
            ptree df;
            read_json(ss, df);
            std::string stock_ticker;
            double price;
            double volume;
            bool parse = false;
            for(ptree::const_iterator it = df.begin(); it != df.end(); ++it){
                for(ptree::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt){
                    if(parse == true){
                        if(jt->first == "S"){
                            stock_ticker = jt->second.get_value<std::string>();
                        }
                        if(jt->first == "p"){
                            price = atof(jt->second.get_value<std::string>().c_str());
                        }
                        if(jt->first == "s"){
                            volume = atof(jt->second.get_value<std::string>().c_str());
                        }
                    }
                    
                    if(jt->first == "T"){
                        if(jt->second.get_value<std::string>() == "t"){
                            parse = true;
                        }
                    }
                }
                StockData[stock_ticker]["price"].push_back(price);
                StockData[stock_ticker]["volume"].push_back(volume);
                StockPrice[stock_ticker] = price;
            }
        }

    public:
        quant(std::string key, std::string secret, std::vector<std::string> tickers){
            key_ = key;
            secret_ = secret;
            stocks = tickers;
        }

        std::map<std::string, std::map<std::string, std::vector<double>>> StockData;
        std::map<std::string, double> StockPrice;

        bool sync(){
            if(StockData.size() == stocks.size()){
                return true;
            } else {
                return false;
            }
        }

        static void Socket(quant * qfin){
            std::string url = "wss://stream.data.alpaca.markets/v2/iex";
            std::string auth = "{\"action\": \"auth\", \"key\": \"" + qfin->key_ + "\", \"secret\": \""+ qfin->secret_ + "\"}";
            std::string msg = "{\"action\":\"subscribe\",\"trades\":[";
            for(auto & stock : qfin->stocks){
                msg += "\"" + stock + "\",";
            }
            msg.pop_back();
            msg += "]}";

            websocket_client client;
            client.connect(url).wait();

            websocket_outgoing_message outmsg;
            outmsg.set_utf8_message(auth);
            client.send(outmsg);

            outmsg.set_utf8_message(msg);
            client.send(outmsg);

            while(true){
                client.receive().then([](websocket_incoming_message inmsg){
                    return inmsg.extract_string();
                }).then([&](std::string message){
                    qfin->Cyclone(message);
                }).wait();
            }

            client.close().wait();

        }

};