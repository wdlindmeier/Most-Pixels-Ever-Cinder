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

namespace mpe
{
    const static int PACKET_SIZE = 10;

    class TCPClient
    {
    public:
        
        TCPClient();
        bool                        open(const std::string & hostname,
                                         const int por);
        void                        close();
        bool                        isConnected(){ return mIsConnected; }
        void                        write(string msg);
        std::string                 read();

    private:
        
        boost::asio::io_service     mIOService;
        tcp::socket                 mSocket;
        //char                        mReadBuffer[PACKET_SIZE];
        bool                        mIsConnected;

    };        
}
