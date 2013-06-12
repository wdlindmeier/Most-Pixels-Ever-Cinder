//
//  TCPClient.h
//  MPEClient
//
//  Created by William Lindmeier on 6/12/13.
//
//

#pragma once

#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

using namespace std;
using boost::asio::ip::tcp;


namespace mpe {

    const static int PACKET_SIZE = 10;
    typedef std::deque<string> message_queue;
    
    class TCPClient
    {
    public:
        
        TCPClient(boost::asio::io_service& io_service,
                  tcp::resolver::iterator endpoint_iterator);
        void write(string msg);
        void close();
        bool isConnected(){ return mIsConnected; }
        
    private:
        
        void handle_connect(const boost::system::error_code& error);
        void handle_read(const boost::system::error_code& error);
        void do_write(string msg);
        void handle_write(const boost::system::error_code& error);
        void do_close();
        
    private:
        
        boost::asio::io_service& mIOService;
        tcp::socket mSocket;
        char mReadBuffer[PACKET_SIZE];
        message_queue mWriteMsgs;
        bool mIsConnected;
        
    };
        
}
