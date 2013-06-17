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
#include "MPEProtocol.hpp"

using namespace std;
using namespace ci;
using namespace mpe;

namespace mpe {
    
    typedef boost::function<void()> FrameEventCallback;
    typedef boost::function<void( const Rectf & renderRect, bool is3D )> RepositionCallback;

    class MPEClient {
        
        public:
        
        MPEClient(){};
        MPEClient( string settingsFilename, bool shouldResize = true );
        ~MPEClient();
        
        // Handle Connection
        void                start( FrameEventCallback renderFrameHandler );
        void                stop();
        bool                isConnected(){ return mTCPClient->isConnected(); };
        void                handleTCPConnect( bool didConnect, const boost::system::error_code& error );
        
        // Server Com
        void                broadcast( const std::string & message );
        void                sendPing();
        void                sendClientID();
        
        // Accessors
        ci::Rectf           getVisibleRect(){ return mLocalViewportRect; };
        void                setVisibleRect(const Rectf & rect ){ mLocalViewportRect = rect; }
        ci::Vec2i           getMasterSize(){ return mMasterSize; };        
        bool                getIsRendering3D(){ return mIsRendering3D; };
        void                setIsRendering3D( bool is3D ){ mIsRendering3D = is3D; };
        
        protected:
        
        void                draw();
        void                positionViewport();
        void                positionViewport3D();
        void                positionViewport2D();
        void                handleServerMessage( const std::string & serverMessage );
        
        private:
        
        void                doneRendering();
        void                loadSettings( string settingsFilename, bool shouldResize );
        
        // A connection to the server.
        TCPClient           *mTCPClient;

        // A protocol to convert a given command into a transport string.
        MPEProtocol         mProtocol;
        
        // The frame render callback. This is a member of the Cinder App.
        FrameEventCallback  mRenderFrameCallback;
        
        // A reposition callback to let the App override the repositioning GL calls.
        RepositionCallback  mRepositionCallback;
        
        bool                mIsRendering3D;
        
        // Settings loaded from settings.xml
        int                 mPort;
        string              mHostname;
        bool                mIsStarted;
        ci::Rectf           mLocalViewportRect;
        ci::Vec2i           mMasterSize;
        int                 mClientID;
        
    };

}