//
//  TCPAsyncClient.cpp
//  MPEClient
//
//  Created by William Lindmeier on 7/7/13.
//
//

#include "TCPAsyncClient.h"

using namespace mpe;
using std::string;
using ci::app::console;
using namespace boost::asio::ip;

void TCPAsyncClient::open(const std::string & hostname,
                          const int port,
                          const OpenedCallback &callback)
{
    tcp::resolver resolver(mIOService);
    tcp::resolver::query query(hostname, std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);
    
    mOpenedCallback = callback;
    
    printf("Opening connection to %s:%i\n", hostname.c_str(), port);
    
    boost::asio::async_connect(mSocket, iterator,
                               boost::bind(&TCPAsyncClient::handleConnect, this,
                                            boost::asio::placeholders::error));
    
    mClientThread = std::thread(boost::bind(&boost::asio::io_service::run,
                                            &mIOService));
}

void TCPAsyncClient::close()
{
    mIOService.post(boost::bind(&TCPAsyncClient::doClose, this));
}

void TCPAsyncClient::handleConnect(const boost::system::error_code& error)
{
    if (!error)
    {
        console() << "Connected Async\n";
        mIsConnected = true;
        boost::asio::async_read_until(mSocket, mBuffer, "\n",
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
    console() << "Writing async: " << msg << "\n";
    mIOService.post(boost::bind(&TCPAsyncClient::doWrite, this, msg));
}

void TCPAsyncClient::writeBuffer(const boost::asio::const_buffers_1 & buffer)
{
#if USE_STRING_QUEUE
    console() << "ALERT: Not using writeBuffer. Ignoring message.\n";
#else
    mIOService.post(boost::bind(&TCPAsyncClient::doWriteBuffer, this, buffer));
#endif
}

void TCPAsyncClient::handleRead(const boost::system::error_code& error)
{
    // console() << "handle read\n";
    if (!error)
    {
        if (mReadCallback)
        {
            std::ostringstream ss;
            ss << &mBuffer;
            std::string message = ss.str();
            mReadCallback(message);
        }
        boost::asio::async_read_until(mSocket, mBuffer, "\n",
                                      boost::bind(&TCPAsyncClient::handleRead, this,
                                                  boost::asio::placeholders::error));
    }
    else
    {
        printf("ERROR reading from host: %s\n", error.message().c_str());
        doClose();
    }
}

void TCPAsyncClient::doWriteBuffer(const boost::asio::const_buffers_1 & buffer)
{
#if USE_STRING_QUEUE
    printf("ERROR not using doWriteBuffer. Ignoring message.\n");
#else
    bool write_in_progress = !mWriteMessages.empty();
    mWriteMessages.push_back(buffer);
    if (!write_in_progress)
    {
        boost::asio::async_write(mSocket, mWriteMessages.front(),
                                 boost::bind(&TCPAsyncClient::handleWrite, this,
                                             boost::asio::placeholders::error));
    }
#endif
}

void TCPAsyncClient::doWrite(const string & msg)
{
#if USE_STRING_QUEUE
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
#else
    doWriteBuffer(boost::asio::buffer(msg, msg.length()));
#endif
}

void TCPAsyncClient::handleWrite(const boost::system::error_code& error)
{
    if (!error)
    {
        mWriteMessages.pop_front();
        if (!mWriteMessages.empty())
        {
            //Send another
#if USE_STRING_QUEUE
            boost::asio::async_write(mSocket,
                                     boost::asio::buffer(mWriteMessages.front(),
                                                         mWriteMessages.front().length()),
                                     boost::bind(&TCPAsyncClient::handleWrite, this,
                                                 boost::asio::placeholders::error));
#else
            boost::asio::async_write(mSocket, mWriteMessages.front(),
                                     boost::bind(&TCPAsyncClient::handleWrite, this,
                                                 boost::asio::placeholders::error));
#endif
        }
    }
    else
    {
        printf("ERROR writing: %s\n", error.message().c_str());
        doClose();
    }
}

void TCPAsyncClient::doClose()
{
    if (mSocket.is_open())
    {
        mSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        mSocket.close();
    }
    mIsConnected = false;
}
