//
//  TCPClient.h
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>

/*

 TODO: Describe TCPClient class.

*/

namespace mpe
{
    class TCPClient
    {
    public:

        TCPClient();
        bool                        open(const std::string & hostname, const int port);
        void                        close();
        bool                        isConnected(){ return mIsConnected; }
        void                        write(string msg);
        std::string                 read();

    private:

        boost::asio::io_service     mIOService;
        boost::tcp::socket          mSocket;
        bool                        mIsConnected;

    };
}
