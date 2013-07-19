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

// TEST
// Use String Queue rather than buffer queue.
// Having some issues sending a bunch of data (as a buffer) to the server,
// but not sure what it is yet.
// I was using a buffer queue rather than string to facilitate sending
// int and byte arrays, but it looks like that's not supported anyway.
#define USE_STRING_QUEUE    1

/*

 TCPClient:
 A synchronous socket layer that communicates between the client and the server.
 This class is used by the MPEClient, not your App.

*/

namespace mpe
{
    class TCPClient
    {

    public:

        TCPClient(const std::string & messageDelimeter);
        ~TCPClient() {};
        virtual bool                    open(const std::string & hostname, const int port);
        virtual void                    close();
        virtual bool                    isConnected(){ return mIsConnected; }
        virtual void                    write(const std::string & msg);
        virtual void                    writeBuffer(const boost::asio::const_buffers_1 & buffer);
        std::string                     read(bool & isDataAvailable);
        std::vector<int>                readIntegers();
        std::vector<char>               readBytes();

    protected:

        std::string                     mMessageDelimiter;
        boost::asio::io_service         mIOService;
        boost::asio::ip::tcp::socket    mSocket;
        bool                            mIsConnected;

    };
}
