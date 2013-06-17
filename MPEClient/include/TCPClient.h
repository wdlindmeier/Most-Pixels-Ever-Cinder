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
    typedef boost::function<void (bool didConnect, const boost::system::error_code& error)> OpenedCallback;
    
    class TCPClient
    {
    public:
        
        TCPClient();
        void open(const std::string & hostname,
                  const int port,
                  const OpenedCallback &callback);
        void close();
        bool isConnected(){ return mIsConnected; }
        void write(string msg);

    protected:
        
        boost::asio::io_service mIOService;
        
    private:
        
        void handleConnect(const boost::system::error_code& error);
        void handleRead(const boost::system::error_code& error);
        void doWrite(string msg);
        void handleWrite(const boost::system::error_code& error);
        void doClose();

        tcp::socket mSocket;
        char mReadBuffer[PACKET_SIZE];
        message_queue mWriteMsgs;
        bool mIsConnected;
        OpenedCallback mOpenedCallback;    
        std::thread mClientThread;

    };
        
}
