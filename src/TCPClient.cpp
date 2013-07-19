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

// TODO: Test that this works
vector<int> TCPClient::readIntegers()
{
    // Read the first 4 bytes to see how long the integer array is.
    // Then read the rest.
    int intLength;
    boost::asio::read(mSocket, boost::asio::buffer(reinterpret_cast<char*>(&intLength),
                                                   sizeof(int)));
    // console() << "Reading Integer Array of length: %i" << intLength << std::endl;
    int ints[intLength];
    boost::asio::read(mSocket, boost::asio::buffer(reinterpret_cast<char*>(&ints),
                                                   sizeof(int) * intLength));
    vector<int> vecInt;
    vecInt.assign(ints, ints+intLength);
    return vecInt;
}

// TODO: Test that this works
vector<char> TCPClient::readBytes()
{
    // Read the first 4 bytes to see how long the integer array is.
    // Then read the rest.
    int byteLength;
    boost::asio::read(mSocket, boost::asio::buffer(reinterpret_cast<char*>(&byteLength),
                                                   sizeof(int)));
    // console() << "Reading Byte Array of length: %i" << byteLength << std::endl;
    char bytes[byteLength];
    boost::asio::read(mSocket, boost::asio::buffer(reinterpret_cast<char*>(&bytes),
                                                   sizeof(char) * byteLength));

    vector<char> vecBytes;
    vecBytes.assign(bytes, bytes+byteLength);
    return vecBytes;
}

void TCPClient::writeBuffer(const boost::asio::const_buffers_1 & buffer)
{
#if USE_STRING_QUEUE
    console() << "ALERT: Not using write buffer. Message Ignored." << std::endl;
#else
    boost::system::error_code error;
    // TODO: Guarantee that all of the data has been sent.
    // write_some doesn't make that promise.
    size_t len = mSocket.write_some(buffer, error);
    if (error)
    {
        console() << "ERROR: Couldn't write. " << error.message() << std::endl;
        return;
    }
    else if (len == 0)
    {
        console() << "ALERT: Wrote 0 bytes." << std::endl;
        return;
    }
#endif
}

void TCPClient::write(const string & msg)
{
#if USE_STRING_QUEUE
    boost::system::error_code error;
    // TODO: Guarantee that all of the data has been sent.
    // write_some doesn't make that promise.
    size_t len = mSocket.write_some(boost::asio::buffer(msg), error);
    if (error)
    {
        console() << "ERROR: Couldn't write. " << error.message() << "\n";
    }
    else if (len == 0)
    {
        console() << "ALERT: Wrote 0 bytes.\n";
    }
#else
    writeBuffer(boost::asio::buffer(msg));
#endif
}

void TCPClient::close()
{
    mIsConnected = false;
    if (mSocket.is_open())
    {
        mSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        mSocket.close();
    }
    mIOService.stop();
}