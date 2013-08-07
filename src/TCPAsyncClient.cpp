//
//  TCPAsyncClient.cpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#include "cinder/Utilities.h"
#include "TCPAsyncClient.h"

using namespace mpe;
using std::string;
using ci::app::console;
using namespace boost::asio::ip;

TCPAsyncClient::TCPAsyncClient(const std::string & messageDelimeter) :
TCPClient(messageDelimeter)
{
};

TCPAsyncClient::~TCPAsyncClient()
{
    mSocket.close();
    mIOService.stop();
    mClientThread.detach();
};

void TCPAsyncClient::open(const string & hostname,
                          const int port,
                          const OpenedCallback &callback)
{
    if (mIsConnected)
    {
        close();
    }

    tcp::resolver resolver(mIOService);
    tcp::resolver::query query(hostname, std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);

    mOpenedCallback = callback;

    boost::asio::async_connect(mSocket, iterator,
                               boost::bind(&TCPAsyncClient::handleConnect, this,
                                            boost::asio::placeholders::error));
    mClientThread = std::thread(boost::bind(&boost::asio::io_service::run,
                                            &mIOService));
}

void TCPAsyncClient::close()
{
    mIOService.post(boost::bind(&TCPAsyncClient::_close, this));
}

void TCPAsyncClient::handleConnect(const boost::system::error_code & error)
{
    if (!error)
    {
        mIsConnected = true;
        boost::asio::async_read_until(mSocket, mBuffer, mMessageDelimiter,
                                      boost::bind(&TCPAsyncClient::handleRead, this,
                                                  boost::asio::placeholders::error));
    }
    else
    {
        printf("ERROR connecting to host: %s\n", error.message().c_str());
    }

    if (mOpenedCallback)
    {
        mOpenedCallback(mIsConnected, error);
    }
}

void TCPAsyncClient::write(const string & msg)
{
    mIOService.post(boost::bind(&TCPAsyncClient::_write, this, msg));
}

void TCPAsyncClient::setIncomingMessageCallback(ServerMessageCallback callback)
{
    mReadCallback = callback;
};

void TCPAsyncClient::handleRead(const boost::system::error_code & error)
{
    if (!error)
    {
        if (mReadCallback)
        {
            std::ostringstream ss;
            ss << &mBuffer;
            string messageData = ss.str();
            // There may be more than 1 message in the read.
            std::vector<string> messages = ci::split(messageData, mMessageDelimiter);
            for (int i = 0; i < messages.size(); ++i)
            {
                std::string message = messages[i];
                if (message.length() > 0)
                {
                    mReadCallback(message);
                }
            }
        }
        boost::asio::async_read_until(mSocket, mBuffer, mMessageDelimiter,
                                      boost::bind(&TCPAsyncClient::handleRead, this,
                                                  boost::asio::placeholders::error));
    }
    else
    {
        printf("ERROR reading from host: %s\n", error.message().c_str());
        _close();
    }
}

void TCPAsyncClient::_write(const string & msg)
{
    bool write_in_progress = !mWriteMessages.empty();
    mWriteMessages.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(mSocket,
                                 boost::asio::buffer(mWriteMessages.front(),
                                                     mWriteMessages.front().length()),
                                 boost::bind(&TCPAsyncClient::handleWrite, this,
                                             boost::asio::placeholders::error));
    }
}

void TCPAsyncClient::handleWrite(const boost::system::error_code & error)
{
    if (!error)
    {
        mWriteMessages.pop_front();
        if (!mWriteMessages.empty())
        {
            //Send another
            boost::asio::async_write(mSocket,
                                     boost::asio::buffer(mWriteMessages.front(),
                                                         mWriteMessages.front().length()),
                                     boost::bind(&TCPAsyncClient::handleWrite, this,
                                                 boost::asio::placeholders::error));
        }
    }
    else
    {
        printf("ERROR writing: %s\n", error.message().c_str());
        _close();
    }
}

void TCPAsyncClient::_close()
{
    TCPClient::close();
}
