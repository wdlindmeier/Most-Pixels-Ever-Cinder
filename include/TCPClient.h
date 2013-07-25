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
        std::string                     read(bool & isDataAvailable);
        std::vector<int>                readIntegers();
        std::vector<char>               readBytes();

        // NOTE: There's no real support for asking a TCP socket "Are you connected"?
        // http://stackoverflow.com/questions/1511129/boostasioiptcpsocket-is-connected
        // So we'll just disconnect after N frames of no data.
        int                             mNumDatalessFramesTimeout;

    protected:

        std::string                     mMessageDelimiter;
        boost::asio::io_service         mIOService;
        boost::asio::ip::tcp::socket    mSocket;
        bool                            mIsConnected;
        int                             mNumDatalessFrames;

    };
}
