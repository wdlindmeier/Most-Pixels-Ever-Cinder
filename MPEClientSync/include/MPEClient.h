//
//  MPEClient.h
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#include <boost/asio.hpp>

#include "cinder/Rect.h"
#include "MPEMessageHandler.hpp"
#include "MPEProtocol.hpp"
#include "TCPClient.h"

/*

 TODO: Describe MPEClient class.

*/

namespace mpe
{
    typedef boost::function<void()> FrameEventCallback;
    typedef boost::function<void( const ci::Rectf & renderRect, bool is3D )> RepositionCallback;

    class MPEClient : public MPEMessageHandler
    {

    public:

        MPEClient(){};
        MPEClient( std::string settingsFilename, bool shouldResize = true );
        ~MPEClient();

        // Handle Connection
        void                start();
        void                stop();
        bool                isConnected(){ return mTCPClient && mTCPClient->isConnected(); };

        // Loop
        void                update();
        void                draw(FrameEventCallback renderFrameHandler);

        // Server Com
        void                broadcast(const std::string & message);
        void                sendClientID();

        // Accessors
        ci::Rectf           getVisibleRect(){ return mLocalViewportRect; };
        void                setVisibleRect(const Rectf & rect){ mLocalViewportRect = rect; }
        ci::Vec2i           getMasterSize(){ return mMasterSize; };
        bool                getIsRendering3D(){ return mIsRendering3D; };
        void                setIsRendering3D(bool is3D){ mIsRendering3D = is3D; };

        protected:

        void                positionViewport();
        void                positionViewport3D();
        void                positionViewport2D();
        void                handleServerMessage(const std::string & serverMessage);

        private:

        void                tcpConnected();
        void                doneRendering();
        void                loadSettings(string settingsFilename, bool shouldResize);

        // A connection to the server.
        TCPClient           *mTCPClient;

        // A protocol to convert a given command into a transport string.
        MPEProtocol         mProtocol;

        // A reposition callback to let the App override the repositioning GL calls.
        RepositionCallback  mRepositionCallback;

        bool                mIsRendering3D;

        // Settings loaded from settings.xml
        int                 mPort;
        std::string         mHostname;
        bool                mIsStarted;
        ci::Rectf           mLocalViewportRect;
        ci::Vec2i           mMasterSize;
        int                 mClientID;
    };
}
