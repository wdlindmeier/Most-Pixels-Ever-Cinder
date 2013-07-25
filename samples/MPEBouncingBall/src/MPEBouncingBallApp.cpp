//
//  MPEBouncingBallApp.cpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#include <boost/foreach.hpp>

#include "Ball.hpp"
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "MPEApp.hpp"

// Choose the threading mode.
// Generally Threaded is the way to go, but if find your app crashing
// because you're making GL calls on a different thread, use the non-threaded client.
// Threaded is used by default. This switch is for demo purposes.
#define USE_THREADED   1

// Choose the protocol version. MPE 2.0 is the latest and greatest.
// 2.0 is used by default. This switch is for demo purposes.
#define USE_VERSION_2   1

#include "MPEClient.h"

using namespace ci;
using namespace ci::app;
using std::string;
using std::vector;
using namespace mpe;

const static string kCommandNewBall = "BALL++";

/*

 MPEBouncingBallApp:
 A sample Cinder App that uses the Most Pixels Ever client.

 Usage:

    1) Start the server (one is located in server/ of the MPE Cinder block) by running:

            $ python simple_server.py

       Passing in `--num-clients N` will wait to start the loop until N clients have connected.
       Otherwise, the server will reset the loop whenever a new client joins.

    2) Build and run both targets from XCode (or launch them from the build folder).

 Behavior:

    • Dragging the app windows around the screen will update their positions relative to the master
      dimensions to simulate physical screens.

    • Clicking the mouse adds balls to the scene.

    • Dragging the mouse will send messages to client 1, but not client 0. Messages are just logged.

    • Pressing 'p' will pause the server.

*/

class MPEBouncingBallApp : public AppNative, public MPEApp
{
public:

    // Setup
    void        prepareSettings(Settings *settings);
    void        setup();

    // Balls
    void        addBallAtPosition(const Vec2f & posBall);

    // Update
    void        update();
    void        mpeFrameUpdate(long serverFrameNumber);
    void        updateLocalViewportFromScreenPosition();

    // Draw
    void        draw();
    void        mpeFrameRender(bool isNewFrame);

    // Input Events
    void        mouseDown(MouseEvent event);
    void        mouseDrag(MouseEvent event);
    void        keyDown(KeyEvent event);

    // MPE App
    void        mpeMessageReceived(const std::string & message, const int fromClientID);
    void        mpeReset();
    std::string mpeSettingsFilename();
#if !USE_VERSION_2
    boost::shared_ptr<MPEProtocol> mpeProtocol();
#endif

private:

    MPEClient::Ptr  mClient;

    bool            mDidMoveFrame;
    long            mServerFramesProcessed;
    Rand            mRand;
    vector<Ball>    mBalls;
    Vec2i           mScreenSize;
    Vec2i           mScreenPos;
};

#pragma mark - Setup

void MPEBouncingBallApp::prepareSettings(Settings *settings)
{
    // NOTE: Initially making the window small to prove that
    // the settings.xml forces a resize.
    settings->setWindowSize(100, 100);
}

void MPEBouncingBallApp::setup()
{
    mClient = MPEClient::New(this);

    mDidMoveFrame = false;

#if !USE_VERSION_2
    // MPE 1.0 doesn't send out a reset command when a connection is made.
    // Doing it manually.
    mpeReset();
#endif
}

#pragma mark - MPE App

void MPEBouncingBallApp::mpeReset()
{
    console() << "RESETTING\n";

    // Set the random seed to a known value so all of the clients are using the same rand values.
    mRand.seed(1);

    // Clear out the previous state
    mServerFramesProcessed = 0;
    mBalls.clear();

    // Add the first ball
    Vec2i sizeMaster = mClient->getMasterSize();
    addBallAtPosition(Vec2f(mRand.nextFloat(sizeMaster.x), mRand.nextFloat(sizeMaster.y)));
}

std::string MPEBouncingBallApp::mpeSettingsFilename()
{
    // CLIENT_ID is a preprocessor macro defined in the Target build settings
    return "settings." + std::to_string(CLIENT_ID) + ".xml";
}

void MPEBouncingBallApp::mpeMessageReceived(const std::string & message, const int fromClientID)
{
    vector<string> tokens = split(message, ",");

    // Check if it's a "new ball" command
    if (tokens.size() > 0 && tokens[0] == kCommandNewBall)
    {
        Vec2f posNewBall = Vec2f(stoi(tokens[1]),stoi(tokens[2]));
        addBallAtPosition(posNewBall);
        console() << "Adding a ball to " << posNewBall << ". Is on screen? "
                  << mClient->isOnScreen(posNewBall) << std::endl;
    }

#if USE_VERSION_2

    console() << mClient->getClientID() << ") Message from client #"
              << fromClientID << ": " << message << std::endl;

#else

    // MPE v 1 didn't support broadcast client IDs
    console() << "Received message : " << message << std::endl;

#endif

}

#if !USE_VERSION_2

// If we want to use a non-default protocol, we override this function
boost::shared_ptr<MPEProtocol> MPEBouncingBallApp::mpeProtocol()
{
    return boost::shared_ptr<MPEProtocol>(new MPEProtocol());
}

#endif

#pragma mark - Balls

void MPEBouncingBallApp::addBallAtPosition(const Vec2f & posBall)
{
    Vec2i sizeMaster = mClient->getMasterSize();
    Vec2f velBall = Vec2f(mRand.nextFloat(-5,5), mRand.nextFloat(-5,5));
    mBalls.push_back(Ball(posBall, velBall, sizeMaster));
}

#pragma mark - Update

void MPEBouncingBallApp::update()
{
    updateLocalViewportFromScreenPosition();

    if (mClient->isConnected())
    {
        if (!mClient->isThreaded())
        {
            // NOTE:
            // MPEClient::update has no effect in threaded mode since it's event based (not polling)
            // If you're using the threaded client, this can be removed.
            mClient->update();
        }
    }
    else if (getElapsedFrames() % 60 == 0)
    {
        // Attempt to reconnect.
        mClient->start();
    }
}

// mpeFrameUpdate is where any state/data changes should be happen (rather than update).
// It's only called when the server has a new frame, which may be less often
// than update() is called by the App loop.

void MPEBouncingBallApp::mpeFrameUpdate(long serverFrameNumber)
{
    if (mBalls.size() > 0)
    {
        mBalls[0].manipulateInternalData();
    }

    // This loop forces the app to get up-to-speed if it disconnects and then re-connects.
    while (mServerFramesProcessed < serverFrameNumber)
    {
        BOOST_FOREACH(Ball & ball, mBalls)
        {
            ball.calc();
        }
        mServerFramesProcessed++;
    }
}

// Using the window's position in the screen as the local
// viewport to simulate multiple screens in space.

void MPEBouncingBallApp::updateLocalViewportFromScreenPosition()
{
    // Moving the window simulates repositioning the screen.
    Vec2i size = getWindowSize();
    Vec2i pos = getWindowPos() - Vec2f(100, 100); // Add a margin to the master

    if (mScreenSize != size || mScreenPos != pos)
    {
        if (mScreenSize != Vec2f::zero())
        {
            mDidMoveFrame = true;
        }

        // The position has changed.
        // Update the renderable area.
        mClient->setVisibleRect(ci::Rectf(pos.x, pos.y, pos.x + size.x, pos.y + size.y));
        console() << "Visible Rect: " << mClient->getVisibleRect() << std::endl;
        mScreenSize = size;
        mScreenPos = pos;
    }
}

#pragma mark - Draw

void MPEBouncingBallApp::draw()
{
    mClient->draw();

    // The MPEClient will reposition the viewport in a default way, but if you want to
    // handle this yourself don't override mpeFrameRender().
    // However, MPEClient::draw() must always be called after you render.
    //
    // E.g.
    //      // APP DRAWING CODE
    //      // ...
    //      mClient->draw();
}

void MPEBouncingBallApp::mpeFrameRender(bool isNewFrame)
{
    // Just for the hell of it to see if we can crash by accessing the same data on different threads.
    if (mBalls.size() > 0)
    {
        mBalls[0].manipulateInternalData();
    }

    gl::clear(Color(0,0,0));

    // Paint the Master drawable region red if we're in step with the server, or magenta if our
    // draw loop is ahead of the server.
    // The App's loop will continue at the normal speed even if we're waiting for data from the
    // server, we just don't update the state.
    // A flickering background means the FPS is faster than the server data-rate.
    if (isNewFrame)
    {
        gl::color(Color(1, 0, 0));
    }
    else
    {
        gl::color(Color(1, 0, 1));
    }

    Vec2i masterSize = mClient->getMasterSize();
    Rectf masterFrame = Rectf(0,0,masterSize.x,masterSize.y);
    gl::drawSolidRect(masterFrame);

    gl::color(0,0,0);
    gl::drawString("Frame Num: " + std::to_string(mClient->getCurrentRenderFrame()),
                   Vec2f(100, 100));
    gl::drawString("FPS: " + std::to_string((int)getAverageFps()),
                   Vec2f(100, 130));
    gl::drawString("Updates Per Second: " + std::to_string((int)mClient->getUpdatesPerSecond()),
                   Vec2f(100, 160));

    float myX = mClient->getVisibleRect().x1;
    float myY = mClient->getVisibleRect().y1;
    if (!mDidMoveFrame)
    {
        gl::drawString("Drag the window to reposition the virtual screen.",
                       Vec2f(myX + 20, myY + 20));
    }
    else
    {
        gl::drawString("X: " + std::to_string((int)myX) + ", y: " + std::to_string((int)myY),
                       Vec2f(myX + 20, myY + 20));
    }

    BOOST_FOREACH(Ball & ball, mBalls)
    {
        ball.draw();
    }
}

#pragma mark - Input Events

void MPEBouncingBallApp::mouseDown(MouseEvent event)
{
    // Adding a new ball to the scene by sending out a message.
    if (mClient->isConnected())
    {
        Vec2i pos = event.getPos() + mClient->getVisibleRect().getUpperLeft();
        mClient->sendMessage(kCommandNewBall + "," +
                             std::to_string(pos.x) + "," +
                             std::to_string(pos.y));
    }
}

void MPEBouncingBallApp::mouseDrag(MouseEvent event)
{
    if (mClient->isConnected())
    {
        Vec2i pos = event.getPos() + mClient->getVisibleRect().getUpperLeft();
        // For testing purposes. Only send drag data to client 1.
        vector<int> toClientIDs = {1};
        mClient->sendMessage(std::to_string(pos.x) + "," + std::to_string(pos.y), toClientIDs);
    }
}

void MPEBouncingBallApp::keyDown(KeyEvent event)
{
    switch (event.getChar())
    {
        case 'p':
            mClient->togglePause();
            break;
        case 'f':
            setFullScreen(!isFullScreen());
            break;
    }
}

CINDER_APP_NATIVE( MPEBouncingBallApp, RendererGl )
