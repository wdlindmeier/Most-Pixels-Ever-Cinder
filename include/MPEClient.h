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
#include "MPEProtocol2.hpp"
#include "TCPClient.h"
#include "MPEApp.hpp"

/*

 MPEClient:
 This class is the interface through which your App communicates with an MPE server.

 Once you've subclassed your Cinder App from MPEApp, you construct a client by passing
 it a pointer to your app. It uses that pointer to call update/reset/etc.
 
 The client keeps track of the current frame that should be rendered (see
 MPEMessageHandler::getCurrentRenderFrame) and informs the server when it's complete. Once
 all of the clients have rendered the frame the server will send out the next frame number.

*/

namespace mpe
{
    class MPEClient : public MPEMessageHandler
    {

    public:

        // Constructors
        MPEClient(){};
        MPEClient(MPEApp *cinderApp);

        // Misc Accessors
        int                 getClientID();
        
        // Screen Dimensions
        ci::Rectf           getVisibleRect();
        void                setVisibleRect(const ci::Rectf & rect);
        ci::Vec2i           getMasterSize();

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
        // The sending App will receive its own data and should act on it when it's received,
        // rather than before it's sent, so all of the clients are in sync.
        void                sendMessage(const std::string & message); // née broadcast
        // Send data to specific client IDs
        void                sendMessage(const std::string & message,
                                        const std::vector<int> & clientIds);

    protected:
        
        virtual void        receivedStringMessage(const std::string & dataMessage,
                                                  const int fromClientID = -1);
        virtual void        receivedResetCommand();
        void                setCurrentRenderFrame(long frameNum);
        void                doneRendering();
        void                positionViewport();
        void                positionViewport3D();
        void                positionViewport2D();
        void                sendClientID();

        // A pointer to your Cinder app
        MPEApp              *mApp;
        std::shared_ptr<MPEProtocol> mProtocol;

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
        void                loadSettings(std::string settingsFilename);

    };
}
