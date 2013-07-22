//
//  MPEAsyncClient.cpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#include "MPEAsyncClient.h"
#include "TCPAsyncClient.h"
#include "MPEProtocol2.hpp"

using ci::app::console;
using namespace mpe;

MPEAsyncClient::MPEAsyncClient(MPEApp *cinderApp) :
MPEClient(cinderApp)
{
};

#pragma mark - Connection

void MPEAsyncClient::start()
{
    if (mIsStarted)
    {
        stop();
    }

    mIsStarted = true;
    mLastFrameConfirmed = -1;
    mTCPClient = new TCPAsyncClient(mProtocol->incomingMessageDelimiter());

    TCPAsyncClient *client = static_cast<TCPAsyncClient *>(mTCPClient);
    client->setIncomingMessageCallback(boost::bind(&MPEAsyncClient::serverMessageReceived, this, _1));
    client->open(mHostname, mPort, boost::bind(&MPEAsyncClient::tcpDidConnect, this, _1, _2));
}

void MPEAsyncClient::tcpDidConnect(bool didConnect, const boost::system::error_code & error)
{
    if (didConnect)
    {
        if (mIsDebug)
        {
            console() << "Established async connection to server: "
                      << mHostname << ":" << mPort << std::endl;
        }
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
    mProtocol->parse(message, this);
    if (mFrameIsReady)
    {
        std::lock_guard<std::mutex> lock(mClientDataMutex);
        mApp->mpeFrameUpdate(this->getCurrentRenderFrame());
    }
}

void MPEAsyncClient::receivedStringMessage(const std::string & dataMessage, const int fromClientID)
{
    std::lock_guard<std::mutex> lock(mClientDataMutex);
    MPEClient::receivedStringMessage(dataMessage, fromClientID);
}

void MPEAsyncClient::receivedResetCommand()
{
    std::lock_guard<std::mutex> lock(mClientDataMutex);
    MPEClient::receivedResetCommand();
}

#pragma mark - Update

void MPEAsyncClient::update()
{
    if (mIsDebug)
    {
        static bool DidAlertAsyncNoEffect = false;
        if (!DidAlertAsyncNoEffect)
        {
            // Frame events are called as messages are received from the server.
            console() << "**INFO: Calling update() has no effect in the Async client." << std::endl;
            DidAlertAsyncNoEffect = true;
        }
    }
}

#pragma mark - Drawing

void MPEAsyncClient::draw()
{
    std::lock_guard<std::mutex> lock(mClientDataMutex);
    MPEClient::draw();
}
