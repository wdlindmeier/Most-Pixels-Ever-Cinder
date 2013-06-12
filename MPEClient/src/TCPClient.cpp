//
//  TCPClient.cpp
//  MPEClient
//
//  Created by William Lindmeier on 6/12/13.
//
//

#include "TCPClient.h"

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
    
    /*
    void TCPClient::writeString(const string& string)
    {
        assert(string.length() <= TCPMessage::max_body_length);
        const char *line = string.c_str(); //[TCPMessage::max_body_length + 1];
        TCPMessage msg;
        msg.body_length(strlen(line));
        memcpy(msg.body(), line, msg.body_length());
        msg.encode_header();
        write(msg);
    }

    void TCPClient::write(const TCPMessage& msg)
    {
        mIOService.post(boost::bind(&TCPClient::do_write, this, msg));
    }
    */
    
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
                                    boost::asio::buffer(mReadMsg.data(), TCPMessage::header_length),
                                    boost::bind(&TCPClient::handle_read_header, this,
                                                boost::asio::placeholders::error));
        }else{
            mIsConnected = false;
            ci::app::console() << "Connection error: " << error.message() << "\n";
        }
    }

    void TCPClient::handle_read_header(const boost::system::error_code& error)
    {
        if (!error && mReadMsg.decode_header())
        {
            boost::asio::async_read(mSocket,
                                    boost::asio::buffer(mReadMsg.body(), mReadMsg.body_length()),
                                    boost::bind(&TCPClient::handle_read_body, this,
                                                boost::asio::placeholders::error));
        }
        else
        {
            do_close();
        }
    }

    void TCPClient::handle_read_body(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout.write(mReadMsg.body(), mReadMsg.body_length());
            std::cout << "\n";
            boost::asio::async_read(mSocket,
                                    boost::asio::buffer(mReadMsg.data(), TCPMessage::header_length),
                                    boost::bind(&TCPClient::handle_read_header, this,
                                                boost::asio::placeholders::error));
        }
        else
        {
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