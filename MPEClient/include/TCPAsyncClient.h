//
//  TCPAsyncClient.h
//  MPEClient
//
//  Created by William Lindmeier on 7/7/13.
//
//

#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>
#include "TCPClient.h"

namespace mpe
{
    class TCPAsyncClient : public TCPClient
    {
        
    typedef std::deque<const std::string> MessageQueue;
    typedef boost::function<void (bool didConnect, const boost::system::error_code& error)> OpenedCallback;
    typedef boost::function<void (const std::string & message)> ServerMessageCallback;
        
    public:
        
        TCPAsyncClient() : TCPClient() {};
        ~TCPAsyncClient() {};
        
        void                            open(const std::string & hostname,
                                             const int port,
                                             const OpenedCallback &callback);
        void                            close();
        void                            write(const std::string & msg);
        void                            setIncomingMessageHandler( ServerMessageCallback callback )
                                        {
                                            mReadCallback = callback;
                                        };
        
    private:
        
        void                            handleConnect(const boost::system::error_code& error);
        void                            handleRead(const boost::system::error_code& error);
        void                            handleWrite(const boost::system::error_code& error);
        void                            doWrite(const std::string & msg);
        void                            doClose();
        
//        boost::asio::io_service         mIOService;
//        boost::asio::ip::tcp::socket    mSocket;
//        bool                            mIsConnected;
        MessageQueue                    mWriteMessages;
        //std::string                     mReadMessage;
        boost::asio::streambuf          mBuffer;
        std::thread                     mClientThread;
        OpenedCallback                  mOpenedCallback;
        ServerMessageCallback           mReadCallback;
        
    };
}