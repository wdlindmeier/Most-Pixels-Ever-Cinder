//
//  MPEAsyncClient.cpp
//  MPEClient
//
//  Created by William Lindmeier on 7/7/13.
//
//

#include "MPEAsyncClient.h"
#include "TCPAsyncClient.h"

using ci::app::console;
using namespace mpe;

#pragma mark - Connection

void MPEAsyncClient::start()
{
    if (mIsStarted)
    {
        stop();
    }
    
    mIsStarted = true;
    mLastFrameConfirmed = -1;
    mTCPClient = new TCPAsyncClient();
    
    TCPAsyncClient *client = static_cast<TCPAsyncClient *>(mTCPClient);
    
    // Set the message callback
    client->setIncomingMessageHandler(boost::bind(&MPEAsyncClient::serverMessageReceived, this, _1));
    
    // Open the client
    client->open(mHostname, mPort, boost::bind(&MPEAsyncClient::tcpConnected, this, _1, _2));
}

void MPEAsyncClient::tcpConnected(bool didConnect, const boost::system::error_code& error)
{
    if (didConnect)
    {
        sendClientID();
    }
    else
    {
        stop();
    }
}

#pragma mark - Receiving Messages

void MPEAsyncClient::serverMessageReceived(const std::string & message)
{
    mFrameIsReady = false;
    // This will set mFrameIsReady
    mProtocol.parse(message, this);
    //mShouldUpdate = mShouldUpdate || mFrameIsReady;
    if (mFrameIsReady)
    {
        if (mUpdateCallback)
        {
            std::lock_guard<std::mutex> lock(mClientDataMutex);
            mUpdateCallback();
        }
    }
}

void MPEAsyncClient::receivedStringMessage(const std::string & dataMessage)
{
    std::lock_guard<std::mutex> lock(mClientDataMutex);
    MPEClient::receivedStringMessage(dataMessage);
}

void MPEAsyncClient::readIncomingIntegers()
{
    std::lock_guard<std::mutex> lock(mClientDataMutex);
    MPEClient::readIncomingIntegers();
}

void MPEAsyncClient::readIncomingBytes()
{
    std::lock_guard<std::mutex> lock(mClientDataMutex);
    MPEClient::readIncomingBytes();
}

#pragma mark - Update
/*
bool MPEAsyncClient::shouldUpdate()
{
    bool shouldUpdate = mShouldUpdate;
    mShouldUpdate = false;
    return shouldUpdate;
}
*/
#pragma mark - Drawing

void MPEAsyncClient::draw(const FrameRenderCallback & renderFrameHandler)
{
    std::lock_guard<std::mutex> lock(mClientDataMutex);
    MPEClient::draw(renderFrameHandler);
}

void MPEAsyncClient::doneRendering()
{
    // Only confirm this once since we may have
    // multiple draws for each update
    if (mLastFrameConfirmed < mCurrentRenderFrame)
    {
        mTCPClient->write(mProtocol.renderIsComplete(mClientID, mCurrentRenderFrame));
        mCurrentRenderFrame = mLastFrameConfirmed;
    }
}