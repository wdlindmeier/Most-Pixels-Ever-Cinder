//
//  MPEClient.h
//  MPEClient
//
//  Created by William Lindmeier on 6/12/13.
//
//

#pragma once

#include "TCPClient.h"
#include <boost/asio.hpp>
#include "cinder/Rect.h"
#include "MPEProtocolClassic.hpp"

using namespace std;
using namespace ci;
using namespace mpe;

class MPEClient {
    
    public:
    
    MPEClient(){};
    MPEClient( string settingsFilename, bool shouldResize = true );
    ~MPEClient();
    
    // Handle Connection
    void                start();
    void                stop();
    bool                isConnected(){ return mTCPClient->isConnected(); };
    void                handleTCPConnect(bool didConnect, const boost::system::error_code& error);
    
    // Server Com
    void                sendLocalScreenRect(ci::Rectf localRect);
    void                sendPing();
    void                sendClientID();
    
    private:
    
    void                loadSettings(string settingsFilename, bool shouldResize);
    
    TCPClient           *mTCPClient;
    MPEProtocolClassic  mProtocol;
    
    // Settings
    int                 mPort;
    string              mHostname;
    bool                mIsStarted;
    cinder::Vec2i       mLocalSize;
    int                 mClientID;
    
};