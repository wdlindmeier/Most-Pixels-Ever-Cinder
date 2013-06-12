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

namespace mpe {
    
    TCPClient::TCPClient(boost::asio::io_service& io_service,
              tcp::resolver::iterator endpoint_iterator) :
    mIOService(io_service),
    mSocket(io_service),
    mIsConnected(false)
    {
        boost::asio::async_connect(mSocket, endpoint_iterator,
                                   boost::bind(&TCPClient::handle_connect, this,
                                               boost::asio::placeholders::error));
    }
        
    void TCPClient::write(string msg)
    {
        mIOService.post(boost::bind(&TCPClient::do_write, this, msg));
    }
    
    void TCPClient::close()
    {
        mIOService.post(boost::bind(&TCPClient::do_close, this));
    }

    void TCPClient::handle_connect(const boost::system::error_code& error)
    {
        if (!error){
            ci::app::console() << "Connected\n";
            mIsConnected = true;
            boost::asio::async_read(mSocket,
                                    boost::asio::buffer(mReadBuffer, PACKET_SIZE),
                                    boost::bind(&TCPClient::handle_read, this,
                                                boost::asio::placeholders::error));
        }else{
            mIsConnected = false;
            ci::app::console() << "Connection error: " << error.message() << "\n";
        }
    }
        
    void TCPClient::handle_read(const boost::system::error_code& error)
    {
        if (!error)
        {
            string message(mReadBuffer);
            int breakPos = message.find("\n");
            if(breakPos < 0) breakPos = PACKET_SIZE;
            message.resize(breakPos);
            ci::app::console() << message << "\n";
            
            // Keep reading brah.
            boost::asio::async_read(mSocket,
                                    boost::asio::buffer(mReadBuffer, PACKET_SIZE),
                                    boost::bind(&TCPClient::handle_read, this,
                                                boost::asio::placeholders::error));

        }
        else
        {
            ci::app::console() << "ERROR: " << error.message() << "\n";
            do_close();
        }
    }

    void TCPClient::do_write(string msg)
    {
        bool write_in_progress = !mWriteMsgs.empty();
        mWriteMsgs.push_back(msg);
        if (!write_in_progress)
        {
            boost::asio::async_write(mSocket,
                                     boost::asio::buffer(mWriteMsgs.front(),
                                                         mWriteMsgs.front().length()),
                                     boost::bind(&TCPClient::handle_write, this,
                                                 boost::asio::placeholders::error));
        }
    }

    void TCPClient::handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            mWriteMsgs.pop_front();
            if (!mWriteMsgs.empty())
            {
                boost::asio::async_write(mSocket,
                                         boost::asio::buffer(mWriteMsgs.front(),
                                                             mWriteMsgs.front().length()),
                                         boost::bind(&TCPClient::handle_write, this,
                                                     boost::asio::placeholders::error));
            }
        }
        else
        {
            ci::app::console() << "ERROR: " << error.message() << "\n";
            do_close();
        }
    }

    void TCPClient::do_close()
    {
        mIsConnected = false;
        ci::app::console() << "Closing socket\n";
        mSocket.close();
    }
    
}