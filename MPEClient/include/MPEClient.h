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
    typedef boost::function<void(bool isNewFrame)> FrameRenderCallback;
    typedef boost::function<void( const std::string & message )> StringDataCallback;
    typedef boost::function<void( const std::vector<int> & integers )> IntegerDataCallback;
    typedef boost::function<void( const std::vector<char> & bytes )> BytesDataCallback;

    class MPEClient : public MPEMessageHandler
    {

    public:

        MPEClient(){};
        MPEClient(const std::string & settingsFilename, bool shouldResize = true );
        ~MPEClient();

        // Accessors
        ci::Rectf           getVisibleRect(){ return mLocalViewportRect; };
        void                setVisibleRect(const ci::Rectf & rect){ mLocalViewportRect = rect; }
        ci::Vec2i           getMasterSize(){ return mMasterSize; };
        bool                getIsRendering3D(){ return mIsRendering3D; };
        void                setIsRendering3D(bool is3D){ mIsRendering3D = is3D; };
        
        // Handle Connection
        void                start();
        void                stop();
        bool                isConnected(){ return mTCPClient && mTCPClient->isConnected(); };

        // Loop
        virtual bool        shouldUpdate();
        virtual void        draw(const FrameRenderCallback & renderFrameHandler);

        // Sending Messages To Server
        void                sendClientID();
        // Send Data functions are called by the App and the values are received by every client.
        // The sending App will receive its own data, and should act on that data
        // only once it's received (always as if it came from another client) so the
        // apps are in sync.
        void                sendStringData(const std::string & message); // née broadcast
        void                sendIntegerData(const std::vector<int> & integers);
        void                sendBytesData(const std::vector<char> & bytes);
        
        // Receiving Messages From Server
        // These are called by the MPE Message Handler and should not be called by the App.
        virtual void        receivedStringMessage(const std::string & dataMessage);
        virtual void        readIncomingIntegers();
        virtual void        readIncomingBytes();
        
        void                setStringDataCallback(const StringDataCallback & callback)
        {
            // Send the app an incoming string message.
            // This is a broadcast message.
            mStringDataCallback = callback;
        }
        
        void                setIntegerDataCallback(const IntegerDataCallback & callback)
        {
            // Send the app an incoming integer vector.
            mIntegerDataCallback = callback;
        }
        
        void                setBytesDataCallback(const BytesDataCallback & callback)
        {
            // Send the app an incoming char vector.
            mBytesDataCallback = callback;
        }
        
    protected:

        virtual void        doneRendering();

        void                positionViewport();
        void                positionViewport3D();
        void                positionViewport2D();

        // A protocol to convert a given command into a transport string.
        MPEProtocol         mProtocol;
        
        // Callbacks
        StringDataCallback  mStringDataCallback;
        IntegerDataCallback mIntegerDataCallback;
        BytesDataCallback   mBytesDataCallback;
        
        bool                mIsRendering3D;
        
        // Settings loaded from settings.xml
        int                 mPort;
        std::string         mHostname;
        bool                mIsStarted;
        ci::Rectf           mLocalViewportRect;
        ci::Vec2i           mMasterSize;
        int                 mClientID;
        bool                mIsDebug;
        
        // A connection to the server.
        TCPClient           *mTCPClient;

    private:

        void                tcpConnected();
        void                loadSettings(std::string settingsFilename, bool shouldResize);
        
    };
}
