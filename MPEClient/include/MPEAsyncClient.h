//
//  MPEAsyncClient.h
//  MPEClient
//
//  Created by William Lindmeier on 7/7/13.
//
//

#pragma once

#include "MPEClient.h"
#include "TCPAsyncClient.h"

namespace mpe
{
    
class MPEAsyncClient : public MPEClient
{
    
public:
    
    typedef boost::function<void()> FrameUpdateCallback;
    
    MPEAsyncClient() : MPEClient()
    {};
    
    MPEAsyncClient(const std::string & settingsFilename, bool shouldResize = true) :
    MPEClient(settingsFilename, shouldResize),
    mLastFrameConfirmed(0)
    {};
    
    ~MPEAsyncClient()
    {};
    
    // Connection
    void                    start();

    // Drawing
    void                    draw(const FrameRenderCallback & renderFrameHandler);
    void                    setFrameUpdateHandler( const FrameUpdateCallback & updateCallback)
                            {
                                mUpdateCallback = updateCallback;
                            };

    // Recieving Messages
    void                    serverMessageReceived(const std::string & message);
    void                    receivedBroadcast(const std::string & dataMessage);
    void                    readIncomingIntegers();
    void                    readIncomingBytes();

protected:
    
    void                    doneRendering();
    
private:

    void                    tcpConnected(bool didConnect, const boost::system::error_code& error);
    
    long                    mLastFrameConfirmed;
    FrameUpdateCallback     mUpdateCallback;
    // This lock is to protect the client data that's being updated
    // on one thread (that's communicating with the server)
    // and accessed for drawing on another thread.
    std::mutex              mClientDataMutex;

};

}