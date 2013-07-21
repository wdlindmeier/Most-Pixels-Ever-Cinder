//
//  TCPClient.cpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#include "cinder/CinderMath.h"
#include "TCPClient.h"

using namespace ci;
using namespace mpe;
using std::string;
using std::vector;
using namespace boost::asio::ip;
using cinder::app::console;

TCPClient::TCPClient(const std::string & messageDelimeter) :
mMessageDelimiter(messageDelimeter),
mIOService(),
mSocket(mIOService),
mIsConnected(false)
{
}

bool TCPClient::open(const std::string & hostname,
                     const int port)
{
    tcp::resolver resolver(mIOService);
    tcp::resolver::query query(hostname, std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    mIsConnected = false;

    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && iterator != end)
    {
        mSocket.close();
        mSocket.connect(*iterator++, error);
    }
    if (error)
    {
        close();
        return false;
    }

    mIsConnected = true;
    return mIsConnected;
}

string TCPClient::read(bool & isDataAvailable)
{
    std::string message;

    // First check if there's any data
    boost::asio::socket_base::bytes_readable command(true);
    mSocket.io_control(command);
    std::size_t bytes_readable = command.get();
    isDataAvailable = bytes_readable > 0;

    if (isDataAvailable)
    {

        boost::system::error_code error;
        boost::asio::streambuf buffer;
        boost::asio::read_until(mSocket, buffer, mMessageDelimiter, error);

        // When the server closes the connection, the ip::tcp::socket::read_some()
        // function will exit with the boost::asio::error::eof error,
        // which is how we know to exit the loop.
        if (error == boost::asio::error::eof)
        {
            close();
            return "";
        }
        else if (error)
        {
            console() << "ERROR: " << error.message() << std::endl;
            throw boost::system::system_error(error); // Some other error.
        }

        std::istream str(&buffer);
        std::getline(str, message);
    }

    return message;
}

void TCPClient::write(const string & msg)
{
    boost::system::error_code error;
    size_t len = mSocket.write_some(boost::asio::buffer(msg), error);
    if (error)
    {
        console() << "ERROR: Couldn't write. " << error.message() << "\n";
    }
    else if (len == 0)
    {
        console() << "ALERT: Wrote 0 bytes.\n";
    }
    else if (len < msg.length())
    {
        console() << "ALERT: Only sent " << len << " of " << msg.length() << " message bytes." << std::endl;
    }
}

void TCPClient::close()
{
    mIOService.stop();
    mIsConnected = false;
}
