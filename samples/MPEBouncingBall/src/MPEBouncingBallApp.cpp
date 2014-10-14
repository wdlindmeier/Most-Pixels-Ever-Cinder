//
//  MPEBouncingBallApp.cpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#include "Resources.h"
#include "Ball.hpp"
#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Rand.h"
#include "MPEApp.hpp"
#include "MPEClient.h"
#include "SimpleGUI.h"

// Choose the threading mode.
// Generally Threaded is the way to go, but if find your app crashing
// because you're making GL calls on a different thread, use the non-threaded client.
// Threaded is used by default. This switch is for demo purposes.
#define USE_THREADED   1

using namespace ci;
using namespace ci::app;
using std::string;
using std::vector;
using namespace mpe;
using namespace mowa::sgui;

const static string kCommandNewBall = "BALL++";
const static string kCommand3DSettings = "3D";
const static string kCommandRenderMode = "3DMODE";

/*

 MPEBouncingBallApp:
 A sample Cinder App that uses the Most Pixels Ever client.

 Usage:

    1) Start the server (one is located in server/ of the MPE Cinder block) by running:

            $ python mpe-python-server/mpe_server.py

       Passing in `--num-clients N` will wait to start the loop until N clients have connected.
       Otherwise, the server will reset the loop whenever a new client joins.

    2) Build and run all 4 targets from XCode (or launch them from the build folder).
       There are 3 Synchronous (i.e. frame rendering) clients and 1 Async client
       that controls the 3D camera.

 Behavior:

    • Clicking the mouse in a Sync window adds cubes to the scene.

    • Dragging the mouse will send messages to client 1, and 555 but not client 0 or 2.

    • Adjusting the params in the Async controller will update the 3D camera.

    • Pressing 'p' will pause/unpause the server.

 Notes:

    For this demo all of the clients are built from the same source, but in pratice the Asynchronous
    controller should be it's own app since the behavior is different than the Sync clients.

*/

class MPEBouncingBallApp : public AppNative, public MPEApp
{
public:

    // Setup
    void        prepareSettings(Settings *settings);
    void        setup();

    // Balls
    void        addBallAtPosition(const vec2 & posBall);

    // Update
    void        update();
    void        mpeFrameUpdate(long serverFrameNumber);
    void        update3DSettings();
    void        send3DSettings();

    // Draw
    void        draw();
    void        drawAsyncClient();
    void        mpeFrameRender(bool isNewFrame);

    // Input Events
    void        mouseDown(MouseEvent event);
    void        mouseDrag(MouseEvent event);
    void        mouseUp(MouseEvent event);
    void        keyDown(KeyEvent event);
    bool        buttonResetClicked(MouseEvent event);
    bool        buttonRenderModeClicked(MouseEvent event);

    // MPE App
    void        mpeMessageReceived(const std::string & message, const int fromClientID);
    void        mpeReset();
    DataSourceRef mpeSettingsFile();

private:

    MPEClientRef        mClient;

    long                mServerFramesProcessed;
    Rand                mRand;
    vector<Ball>        mBalls;
    string              mLastMessage;

    SimpleGUI           *mGUI;
    LabelControl        *mLabelFOV;
    LabelControl        *mLabelAspectRatio;
    LabelControl        *mLabelCameraZ;
    ButtonControl       *mButtonRender3D;

    float               mFOV;
    float               mCamZ;
    float               mAspectRatio;

    Font				mFont;
    gl::TextureFontRef	mTextureFont;
};

#pragma mark - Setup

void MPEBouncingBallApp::prepareSettings(Settings *settings)
{
    // NOTE: Initially making the window small to prove that
    // the settings.xml forces a resize.
    settings->setWindowSize(150, 150);

    // NOTE: We're using the same .cpp file for both the Grid and non-Grid demo apps.
    // If you change one of them, both projects will be affected.
    // The Grid demo has a USE_GRID preprocessor macro.
#ifdef USE_GRID
    if (CLIENT_ID < 500)
    {
        settings->setBorderless();
    }
#endif
    
}

void MPEBouncingBallApp::setup()
{
    mClient = MPEClient::Create(this, USE_THREADED);

    // 3D
    mClient->setIsRendering3D(true);
    mCamZ = -900.0f;
    mClient->set3DCameraZ(mCamZ);
    mFOV = mClient->get3DFieldOfView();
    mAspectRatio = mClient->get3DAspectRatio();

    mGUI = new SimpleGUI(this);
    mGUI->addParam("Field Of View", &mFOV, 1.f, 180.f, mFOV);
    mLabelFOV = mGUI->addLabel("--");
    mGUI->addParam("Camera Z", &mCamZ, -1500.f, 0.f, mCamZ);
    mLabelCameraZ = mGUI->addLabel("--");
    mGUI->addParam("Aspect Ratio", &mAspectRatio, 0.f, 2.f, mAspectRatio);
    mLabelAspectRatio = mGUI->addLabel("--");
    ButtonControl *button = mGUI->addButton("Reset");
    button->registerClick(std::bind(&MPEBouncingBallApp::buttonResetClicked,
                                    this,
                                    std::placeholders::_1));
    mButtonRender3D = mGUI->addButton("Render Mode: 3D");
    mButtonRender3D->registerClick(std::bind(&MPEBouncingBallApp::buttonRenderModeClicked,
                                    this,
                                    std::placeholders::_1));
    
    mFont = Font( "Helvetica Bold", 12 );
    mTextureFont = gl::TextureFont::create( mFont );
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
    ivec2 sizeMaster = mClient->getMasterSize();
    addBallAtPosition(vec2(mRand.nextFloat(sizeMaster.x), mRand.nextFloat(sizeMaster.y)));
    
    if (mClient->isAsynchronousClient())
    {
        send3DSettings();
    }
}

DataSourceRef MPEBouncingBallApp::mpeSettingsFile()
{
    // CLIENT_ID is a preprocessor macro defined in the Target build settings
    return loadResource(SETTINGS_RESOURCE(CLIENT_ID));
}

void MPEBouncingBallApp::mpeMessageReceived(const std::string & message, const int fromClientID)
{
    vector<string> tokens = split(message, ",");

    if (tokens.size() > 0)
    {
        string command = tokens[0];
        if (command == kCommandNewBall)
        {
            vec2 posNewBall = vec2(stoi(tokens[1]),stoi(tokens[2]));
            addBallAtPosition(posNewBall);
            console() << "Adding a ball to " << posNewBall << ". Is on screen? "
                      << mClient->isOnScreen(posNewBall) << std::endl;
        }
        else if (command == kCommand3DSettings)
        {
            mFOV = stof(tokens[1]);
            mCamZ = stof(tokens[2]);
            mAspectRatio = stof(tokens[3]);
            mClient->set3DFieldOfView(mFOV);
            mClient->set3DCameraZ(mCamZ);
            mClient->set3DAspectRatio(mAspectRatio);
        }
        else if (command == kCommandRenderMode)
        {
            bool render3D = stoi(tokens[1]);
            console() << "Changing render mode. Is 3D? " << render3D << std::endl;
            mClient->setIsRendering3D(render3D);
            mButtonRender3D->name = render3D ? "Render Mode: 3D" : "Render Mode: 2D";
        }
    }

    mLastMessage = message;

    console() << mClient->getClientID() << ") Message from client #"
              << fromClientID << ": " << message << std::endl;
}

#pragma mark - Balls

void MPEBouncingBallApp::addBallAtPosition(const vec2 & posBall)
{
    ivec2 sizeMaster = mClient->getMasterSize();
    vec2 velBall = vec2(mRand.nextFloat(-5,5), mRand.nextFloat(-5,5));
    mBalls.push_back(Ball(posBall, velBall, sizeMaster));
}

#pragma mark - Update

void MPEBouncingBallApp::update()
{
    if (mClient->isConnected())
    {
        if (mClient->isAsynchronousClient())
        {
            update3DSettings();
        }

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
        for( Ball & ball : mBalls )
        {
            ball.calc();
        }
        mServerFramesProcessed++;
    }
}

void MPEBouncingBallApp::update3DSettings()
{
    /*
    mLabelFOV->setText(std::to_string(mClient->get3DFieldOfView()));
    mLabelCameraZ->setText(std::to_string(mClient->get3DCameraZ()));
    mLabelAspectRatio->setText(std::to_string(mClient->get3DAspectRatio()));
    */
    // If the client values are different than the local values, send an update message
    if (mClient->get3DFieldOfView() != mFOV ||
        mClient->get3DCameraZ() != mCamZ ||
        mClient->get3DAspectRatio() != mAspectRatio)
    {
        send3DSettings();
    }
}

void MPEBouncingBallApp::send3DSettings()
{
    if (mClient->isConnected())
    {
        mClient->sendMessage(kCommand3DSettings + "," +
                             std::to_string(mFOV) + "," +
                             std::to_string(mCamZ) + "," +
                             std::to_string(mAspectRatio));
    }
}

#pragma mark - Draw

void MPEBouncingBallApp::draw()
{
    if (mClient->isAsynchronousClient())
    {
        // NOTE: Async draw doesn't happen in mpeFrameRender for a couple of reasons:
        // 1) They don't sync their render frames with the server.
        // 2) They don't fall within the view space as the Sync clients and don't need a reposition.

        drawAsyncClient();
        return;
    }

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

    // Paint the Master drawable region gray if we're in step with the server, or red if our
    // draw loop is ahead of the server.
    // The App's loop will continue at the normal speed even if we're waiting for data from the
    // server, we just don't update the state.
    // A flickering background means the FPS is faster than the server data-rate.
    if (mClient->isConnected())
    {
        if (isNewFrame)
        {
            gl::color(Color(0.8, 0.75, 0.75));
        }
        else
        {
            gl::color(Color(1, 0.25, 0.25));
        }
    }
    else
    {
        // Gray if we're not connected
        gl::color(Color(0.5, 0.5, 0.5));
    }

    ivec2 masterSize = mClient->getMasterSize();
    Rectf masterFrame = Rectf(0,0,masterSize.x,masterSize.y);
    gl::drawSolidRect(masterFrame);

    gl::color(0.2,0.2,0.2);
    gl::enableAlphaBlending();
    std::ostringstream stringStream;
    stringStream << "Client ID: " << std::to_string(CLIENT_ID) << std::endl;
    stringStream << "FPS: " << std::to_string((int)getAverageFps()) << std::endl;
    stringStream << "Frame Num: " << std::to_string(mClient->getCurrentRenderFrame()) << std::endl;
    stringStream << "Updates Per Second: " << std::to_string((int)mClient->getUpdatesPerSecond());
    string str = stringStream.str();
    vec2 stringSize = mTextureFont->measureStringWrapped(str, Rectf(0,0,400,400));
    vec2 stringOffset(40,40);
    vec2 rectOffset = mClient->getVisibleRect().getUpperLeft() + stringOffset;
    gl::drawSolidRect(Rectf(rectOffset.x - 10,
                            rectOffset.y - 10,
                            rectOffset.x + 20 + stringSize.x,
                            rectOffset.y + 20 + stringSize.y - mTextureFont->getAscent()));
    gl::color(0.9,0.9,0.9);
    mTextureFont->drawStringWrapped(str,
                                    mClient->getVisibleRect(),
                                    vec2(0, mTextureFont->getAscent()) + stringOffset);

    gl::disableAlphaBlending();

    glEnable (GL_DEPTH_TEST);

    for( Ball & ball : mBalls )
    {
        ball.draw( mClient->getIsRendering3D() );
    }

    glDisable (GL_DEPTH_TEST);

}

void MPEBouncingBallApp::drawAsyncClient()
{
    gl::clear(Color(0.2, 0.2, 0.2));
    mGUI->draw();
}

#pragma mark - Input Events

bool MPEBouncingBallApp::buttonResetClicked(MouseEvent event)
{
    mClient->resetAll();
    return true;
}

bool MPEBouncingBallApp::buttonRenderModeClicked(MouseEvent event)
{
    bool render3D = !mClient->getIsRendering3D();
    if (mClient->isConnected())
    {
        mClient->sendMessage(kCommandRenderMode + "," +
                             std::to_string((int)render3D));
    }
    return true;
}

void MPEBouncingBallApp::mouseDown(MouseEvent event)
{
    // Adding a new ball to the scene by sending out a message.
    if (mClient->isConnected())
    {
        if (!mClient->isAsynchronousClient())
        {
            ivec2 pos = event.getPos() + ivec2(mClient->getVisibleRect().getUpperLeft());
            mClient->sendMessage(kCommandNewBall + "," +
                                 std::to_string(pos.x) + "," +
                                 std::to_string(pos.y));
        }
    }
}

void MPEBouncingBallApp::mouseDrag(MouseEvent event)
{
    if (mClient->isConnected())
    {
        if (!mClient->isAsynchronousClient())
        {
            ivec2 pos = event.getPos() + ivec2(mClient->getVisibleRect().getUpperLeft());
            // For testing purposes. Only send drag data to client 1.
            vector<int> toClientIDs = {1,555};
            mClient->sendMessage(std::to_string(pos.x) + "," + std::to_string(pos.y), toClientIDs);
        }
    }
}

void MPEBouncingBallApp::mouseUp(MouseEvent event)
{
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

// If you're deploying to iOS, set the Render antialiasing to 0 for a significant
// performance improvement. This value defaults to 4 (AA_MSAA_4) on iOS and 16 (AA_MSAA_16)
// on the Desktop.
#if defined( CINDER_COCOA_TOUCH )
CINDER_APP_NATIVE( MPEBouncingBallApp, RendererGl(RendererGl::AA_NONE) )
#else
CINDER_APP_NATIVE( MPEBouncingBallApp, RendererGl )
#endif