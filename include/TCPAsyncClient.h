//
//  TCPAsyncClient.h
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>

#include "TCPClient.h"

/*
 
 TCPClient:
 An asynchronous socket layer that communicates between the client and the server.
 This class is used by the MPEAsyncClient, not your App.
 
 */

namespace mpe
{
    class TCPAsyncClient : public TCPClient
    {

    typedef std::deque<const std::string> MessageQueue;
    typedef boost::function<void (bool didConnect, const boost::system::error_code & error)> OpenedCallback;
    typedef boost::function<void (const std::string & message)> ServerMessageCallback;

    public:

        TCPAsyncClient(const std::string & messageDelimeter);
        ~TCPAsyncClient();

        void                            open(const std::string & hostname,
                                             const int port,
                                             const OpenedCallback &callback);
        void                            close();
        void                            write(const std::string & msg);
        void                            setIncomingMessageCallback(ServerMessageCallback callback);

    private:

        void                            handleConnect(const boost::system::error_code & error);
        void                            handleRead(const boost::system::error_code & error);
        void                            handleWrite(const boost::system::error_code & error);

        // Internal versions of the public interface.
        // These happen on a different thread.
        void                            _write(const std::string & msg);
        void                            _close();

        MessageQueue                    mWriteMessages;
        boost::asio::streambuf          mBuffer;
        std::thread                     mClientThread;
        OpenedCallback                  mOpenedCallback;
        ServerMessageCallback           mReadCallback;

    };
}
