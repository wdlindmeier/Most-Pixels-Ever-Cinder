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
using std::string;

TCPClient::TCPClient() :
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
    
    ci::app::console() << "Open? " << mSocket.is_open() << "\n";
    
    mIsConnected = true;
    return mIsConnected;
}

string TCPClient::read()
{
    boost::system::error_code error;
    size_t len = mSocket.read_some(boost::asio::buffer(mReadBuffer), error);
    
    // When the server closes the connection, the ip::tcp::socket::read_some()
    // function will exit with the boost::asio::error::eof error,
    // which is how we know to exit the loop.
    
    if (error == boost::asio::error::eof)
    {
        close();
        return "\n";
    }
    else if (error)
    {
        ci::app::console() << "ERROR: " << error.message() << "\n";
        throw boost::system::system_error(error); // Some other error.
    }
    
    // Do something with this data.
    string message(mReadBuffer);

    int breakPos = message.find("\n");
    if (breakPos == string::npos)
    {
        breakPos = PACKET_SIZE;
    }
    else
    {
        // We want to pass along the newline
        breakPos += 1;
    }
    message.resize( breakPos );
    
    return message;
}

void TCPClient::write(string msg)
{
    boost::system::error_code error;
//    ci::app::console() << "Write " << msg << "\n";
    size_t len = mSocket.write_some(boost::asio::buffer(msg), error);
    if (error)
    {
        ci::app::console() << "--ERROR: Couldn't write. " << error.message() << "\n";
    }
    else if (len == 0)
    {
        ci::app::console() << "ALERT: Wrote 0 bytes.\n";
    }
}

void TCPClient::close()
{
    mIsConnected = false;
    ci::app::console() << "Closing socket\n";
    mSocket.close();
    mIOService.stop();
}