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

 MPEClient:
 This class is the interface through which your App communicates with an MPE server.
 
 Create a pointer to an instance of MPEClient in your Cinder app by passing in the
 settings file. See settings.0.xml for an example.
 
 The client keeps track of the current frame that should be rendered (see 
 MPEMessageHandler::getCurrentRenderFrame) and informs the server when it's complete. Once
 all of the clients have rendered the frame the server will send out the next frame number.
 
 MPEClient uses callbacks for updating, drawing, and sending data to your App.
 
    • FrameUpdateCallback: This is the update callback, called whenever the server sends a new frame.
        App state changes should only happen in this callback, rather than in App::update() so that
        all of the clients stay in sync. This must be set.
 
    • FrameRenderCallback: This is the draw callback. The client will position the viewport before
        calling the callback and tells the server that the frame has been rendered after the callback.
 
    • StringDataCallback: This will be called when string data is received from any of the connected
        clients (including yourself).
 
    • IntegerDataCallback & BytesDataCallback: Similar to the above. These formats are not yet 
        supported.
  
*/

namespace mpe
{
    typedef boost::function<void(bool isNewFrame)> FrameRenderCallback;
    typedef boost::function<void(long serverFrameNumber)> FrameUpdateCallback;
    typedef boost::function<void( const std::string & message )> StringDataCallback;
    typedef boost::function<void( const std::vector<int> & integers )> IntegerDataCallback;
    typedef boost::function<void( const std::vector<char> & bytes )> BytesDataCallback;
    
    class MPEClient : public MPEMessageHandler
    {

    public:

        // Constructors
        MPEClient(){};
        MPEClient(const std::string & settingsFilename, bool shouldResize = true);
        MPEClient(const std::string & settingsFilename, MPEProtocol protocol, bool shouldResize = true);
        ~MPEClient(){};

        // Screen Dimensions
        ci::Rectf           getVisibleRect();
        void                setVisibleRect(const ci::Rectf & rect);
        ci::Vec2i           getMasterSize();
        
        // Callbacks
        void                setFrameUpdateCallback( const FrameUpdateCallback & callback);
        void                setDrawCallback( const FrameRenderCallback & callback);
        void                setStringDataCallback(const StringDataCallback & callback);
        void                setIntegerDataCallback(const IntegerDataCallback & callback);
        void                setBytesDataCallback(const BytesDataCallback & callback);
        
        // 3D Rendering
        bool                getIsRendering3D();
        void                setIsRendering3D(bool is3D);
        void                set3DFieldOfView(float fov);
        float               get3DFieldOfView();
        void                restore3DCamera();

        // Connection
        void                start();
        void                stop();
        bool                isConnected();

        // Loop
        virtual void        update();
        virtual void        draw();

        // Sending Data
        // Data sent to the server is broadcast to every client.
        // The sending App will receive its own data, and should act on it
        // then (rather than before it's sent) so all of the clients are in sync.
        void                sendStringData(const std::string & message); // née broadcast
        void                sendIntegerData(const std::vector<int> & integers);
        void                sendBytesData(const std::vector<char> & bytes);

        // Receiving Messages From Server
        // These are called by the MPE Message Callback and should not be called by the App.
        virtual void        receivedStringMessage(const std::string & dataMessage);
        virtual void        readIncomingIntegers();
        virtual void        readIncomingBytes();
        
    protected:

        void                setCurrentRenderFrame(long frameNum);
        void                doneRendering();
        void                positionViewport();
        void                positionViewport3D();
        void                positionViewport2D();
        void                sendClientID();

        // A protocol to convert a given command into a transport string.
        MPEProtocol         mProtocol;

        // Callbacks
        StringDataCallback  mStringDataCallback;
        IntegerDataCallback mIntegerDataCallback;
        BytesDataCallback   mBytesDataCallback;
        FrameUpdateCallback mUpdateCallback;
        FrameRenderCallback mRenderCallback;

        bool                mIsRendering3D;
        long                mLastFrameConfirmed;
        
        // 3D Positioning
        float               mFieldOfView;
        float               mCameraZ;

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

        void                tcpDidConnect();
        void                loadSettings(std::string settingsFilename, bool shouldResize);

    };
}
