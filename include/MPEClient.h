//
//  MPEClient.h
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "cinder/Rect.h"
#include "MPEApp.hpp"
#include "MPEMessageHandler.hpp"
#include "MPEProtocol.hpp"
#include "TCPClient.h"

/*

 MPEClient:
 This class is the interface through which your App communicates with an MPE server.

 Once you've subclassed your Cinder App from MPEApp, you construct a client by passing
 it a pointer to your app like so:

 MPEClient::Create(this); // <-- called from your Cinder app setup()

 The client keeps track of the current frame that should be rendered (see
 MPEMessageHandler::getCurrentRenderFrame) and informs the server when it's complete. Once
 all of the clients have rendered the frame the server will send out the next frame number.

*/

namespace mpe
{
    typedef boost::shared_ptr<class MPEClient> MPEClientRef;
    
    class MPEClient : public MPEMessageHandler
    {

    public:

                                    MPEClient() :
                                    MPEMessageHandler(){};
        
        virtual                     ~MPEClient(){};

        static MPEClientRef         Create(MPEApp *app, bool isThreaded = true);

        // Misc Accessors
        virtual int                 getClientID() = 0;
        virtual bool                isThreaded() = 0;
        virtual std::string         getClientName() = 0;

        // Async vs Sync
        // async == doesn't sync frames with other clients
        // sync == renders frames in-step with other clients
        virtual bool                isAsynchronousClient() = 0;

        // Screen Dimensions
        virtual ci::Rectf           getVisibleRect() = 0;
        virtual void                setVisibleRect(const ci::Rectf & rect) = 0;
        virtual ci::Vec2i           getMasterSize() = 0;

        // 3D Rendering
        virtual bool                getIsRendering3D() = 0;
        virtual void                setIsRendering3D(bool is3D) = 0;
        virtual void                set3DFieldOfView(float fov) = 0;
        virtual float               get3DFieldOfView() = 0;
        virtual void                set3DCameraZ(float camZ) = 0;
        virtual float               get3DCameraZ() = 0;
        virtual void                set3DAspectRatio(float aspectRatio) = 0;
        virtual float               get3DAspectRatio() = 0;

        // Hit testing
        virtual bool                isOnScreen(float x, float y) = 0;
        virtual bool                isOnScreen(const ci::Vec2f & pos) = 0;
        virtual bool                isOnScreen(float x, float y, float w, float h) = 0;
        virtual bool                isOnScreen(const ci::Rectf & rect) = 0;

        // Connection
        virtual void                start() = 0;
        virtual void                start(const std::string & hostname, const int port) = 0;
        virtual void                stop() = 0;
        virtual void                togglePause() = 0;
        virtual void                resetAll() = 0;
        virtual bool                isConnected() = 0;

        // Loop
        virtual void                update() = 0;
        virtual void                draw() = 0;

        // Sending Data
        // Data sent to the server is broadcast to every client.
        // The sending App will receive its own data and should act on it when it's received,
        // rather than before it's sent, so all of the clients are in sync.
        virtual void                sendMessage(const std::string & message) = 0; // née broadcast
        // Send data to specific client IDs
        virtual void                sendMessage(const std::string & message,
                                                const std::vector<int> & clientIds) = 0;

    };
}
