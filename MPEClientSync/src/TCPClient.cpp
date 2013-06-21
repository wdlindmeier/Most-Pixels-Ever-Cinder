//
//  TCPClient.cpp
//  MPEClient
//
//  Created by William Lindmeier on 6/12/13.
//
//

#include "TCPClient.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace mpe;

TCPClient::TCPClient() :
mIOService(),
mSocket(mIOService),
mIsConnected(false)
{
}

void TCPClient::open(const std::string & hostname,
                     const int port,
                     const OpenedCallback &callback)                        
{
    tcp::resolver resolver(mIOService);
    tcp::resolver::query query(hostname, std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);
    
    //mOpenedCallback = callback;
    mIsConnected = false;
    
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end)
    {
        mSocket.close();
        mSocket.connect(*endpoint_iterator++, error);
    }
    if (error)
    {
        close();
        mOpenedCallback(mIsConnected, error);
        return;
        // throw boost::system::system_error(error);
    }
    
    mIsConnected = true;
    mOpenedCallback(mIsConnected, boost::asio::placeholders::error);

    readLoop();
    
}

void TCPClient::readLoop()
{
    for (;;)
    {
        boost::system::error_code error;
        size_t len = socket.read_some(boost::asio::buffer(mReadBuffer), error);
        
        // When the server closes the connection, the ip::tcp::socket::read_some()
        // function will exit with the boost::asio::error::eof error,
        // which is how we know to exit the loop.
        
        if (error == boost::asio::error::eof)
        {
            break; // Connection closed cleanly by peer.
        }
        else if (error)
        {
            throw boost::system::system_error(error); // Some other error.
        }
        
        // Do something with this data.
        string dat = buf.data();
        // std::cout.write(buf.data(), len);
    }
}

void TCPClient::write(string msg)
{
    //ci::app::console() << "Write: " << msg << "\n";
    mIOService.post(boost::bind(&TCPClient::doWrite, this, msg));
}

void TCPClient::close()
{
    mIsConnected = false;
    ci::app::console() << "Closing socket\n";
    mSocket.close();
    mIOService.stop();
}

/*
void TCPClient::handleConnect(const boost::system::error_code& error)
{
    if (!error)
    {
        ci::app::console() << "Connected\n";
        mIsConnected = true;
        boost::asio::async_read(mSocket,
                                boost::asio::buffer(mReadBuffer, PACKET_SIZE),
                                boost::bind(&TCPClient::handleRead, this,
                                            boost::asio::placeholders::error));
    }
    else
    {
        mIsConnected = false;
        ci::app::console() << "Connection error: " << error.message() << "\n";
    }
    
    mOpenedCallback(mIsConnected, error);
}
    
void TCPClient::handleRead(const boost::system::error_code& error)
{
    if (!error)
    {
        string message(mReadBuffer);
        //ci::app::console() << "TCP message: " << message << "\n";
        
        int breakPos = message.find( "\n" );
        if( breakPos < 0 ) breakPos = PACKET_SIZE;
        message.resize( breakPos );
        
        if( mReadCallback ){
            mReadCallback(message);
        }
        
        // Keep reading brah.
        boost::asio::async_read(mSocket,
                                boost::asio::buffer(mReadBuffer, PACKET_SIZE),
                                boost::bind(&TCPClient::handleRead, this,
                                            boost::asio::placeholders::error));
    }
    else
    {
        ci::app::console() << "ERROR: " << error.message() << "\n";
        doClose();
    }
}
*/

void TCPClient::doWrite(string msg)
{
    bool write_in_progress = !mWriteMsgs.empty();
    mWriteMsgs.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(mSocket,
                                 boost::asio::buffer(mWriteMsgs.front(),
                                                     mWriteMsgs.front().length()),
                                 boost::bind(&TCPClient::handleWrite, this,
                                             boost::asio::placeholders::error));
    }
}

void TCPClient::handleWrite(const boost::system::error_code& error)
{
    if (!error)
    {
        mWriteMsgs.pop_front();
        if (!mWriteMsgs.empty())
        {
            boost::asio::async_write(mSocket,
                                     boost::asio::buffer(mWriteMsgs.front(),
                                                         mWriteMsgs.front().length()),
                                     boost::bind(&TCPClient::handleWrite, this,
                                                 boost::asio::placeholders::error));
        }
    }
    else
    {
        ci::app::console() << "ERROR: " << error.message() << "\n";
        close();
    }
}
/*
void TCPClient::doClose()
{
    mIsConnected = false;
    ci::app::console() << "Closing socket\n";
    mSocket.close();
    mIOService.stop();
}
*/