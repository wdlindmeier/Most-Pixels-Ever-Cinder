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

using namespace std;
using namespace mpe;

class MPEClient {
    
    public:
    
    MPEClient(){};
    MPEClient( string settingsFilename, bool shouldResize = true );
    ~MPEClient();
    void start();
    void stop();
    void ping();
    bool isConnected(){ return mTCPClient->isConnected(); };
    
    private:
    
    void loadSettings(string settingsFilename, bool shouldResize);
    
    TCPClient *mTCPClient;
    boost::asio::io_service mIOservice;
    std::thread mClientThread;
    int mPort;
    string mHostname;
    bool mIsStarted;
    cinder::Vec2i mLocalSize;
    
};