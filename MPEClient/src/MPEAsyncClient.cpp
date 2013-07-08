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

void MPEAsyncClient::serverMessageReceived(const std::string & message)
{
    // console() << "Received server message:\n" << message << "\n";
    mProtocol.parse(message, this);
    if (mFrameIsReady)
    {
        // Call the app update.
        
        if (mUpdateCallback)
        {
            // TODO: Wrap this in a mutex lock
            mUpdateCallback();
            // Unwrap lock
        }        
    }
}

void MPEAsyncClient::draw(const FrameRenderCallback & renderFrameHandler)
{
    // TODO: Wrap in a mutex lock
    MPEClient::draw(renderFrameHandler);
    // TODO: Unwrap lock
}

void MPEAsyncClient::doneRendering()
{
    // Only confirm this once since we may have
    // multiple draws for each update
    if (mLastFrameConfirmed < mRenderFrameNum)
    {
        mTCPClient->write(mProtocol.renderIsComplete(mClientID, mRenderFrameNum));
        mRenderFrameNum = mLastFrameConfirmed;
    }
}