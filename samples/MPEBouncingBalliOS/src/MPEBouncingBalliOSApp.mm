//
//  MPEBouncingBalliOSApp.mm
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>

#include "Ball.hpp"
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Rand.h"
#include "MPEApp.hpp"
#include "MPEClient.h"

#import <UIKit/UIKit.h>

#import "MPEAsyncViewController.h"

using namespace ci;
using namespace ci::app;

using namespace ci;
using namespace ci::app;
using std::string;
using std::vector;
using namespace mpe;

const static string kCommandNewBall = "BALL++";
const static string kCommand3DSettings = "3D";

/*

 MPEBouncingBalliOSApp:
 A sample Cinder App that uses the Most Pixels Ever client on iOS devices.
 Designed for a 2 iPad (sync) / 1 iPhone (async) configuration.

 Usage:

 1) Start the server (one is located in server/ of the MPE Cinder block) by running:

     $ python simple_server.py

 2) Update the settings.xml files with the IP address of the server.

 3) Build and run the clients in XCode. There are 2 iPad sync clients, and 1 iPhone async
    controller that controls the 3D camera.

 */

class MPEBouncingBalliOSApp : public AppNative, public MPEApp
{

  public:

    // Setup
    void            prepareSettings(Settings *settings);
    void            setup();

    // Balls
    void            addBallAtPosition(const Vec2f & posBall);

    // Update
    void            update();
    void            mpeFrameUpdate(long serverFrameNumber);
    void            send3DSettings();

    // Draw
    void            draw();
    void            mpeFrameRender(bool isNewFrame);

    // MPE App
    void            mpeMessageReceived(const std::string & message, const int fromClientID);
    void            mpeReset();
    std::string     mpeSettingsFilename();

    // Touch
    void            touchesEnded(TouchEvent event);

    // UIKit Controls
    void            camZValueChanged(float value);
    void            fovValueChanged(float value);
    void            aspectRatioValueChanged(float value);
    void            resetPressed();

  private:

    MPEClient::Ptr      mClient;

    Rand                mRand;
    vector<Ball>        mBalls;

    float               mCamZ;
    float               mFOV;
    float               mAspectRatio;

    Font				mFont;
    gl::TextureFontRef	mTextureFont;

};

void MPEBouncingBalliOSApp::prepareSettings(Settings *settings)
{
    if (CLIENT_ID == 555)
    {
        // Add the Async view controller
        MPEAsyncViewController *viewController = [MPEAsyncViewController new];
        viewController.cameraZValueCallback = bind(&MPEBouncingBalliOSApp::camZValueChanged, this, _1);
        viewController.fovValueCallback = bind(&MPEBouncingBalliOSApp::fovValueChanged, this, _1);
        viewController.aspectRatioValueCallback = bind(&MPEBouncingBalliOSApp::aspectRatioValueChanged, this, _1);
        viewController.resetCallback = std::bind(&MPEBouncingBalliOSApp::resetPressed, this);
        settings->prepareWindow( Window::Format().rootViewController( viewController ) );
    }
    mCamZ = kInitialCameraZ;
    mFOV = kInitialFOV;
    mAspectRatio = kInitialAspectRatio;
}

void MPEBouncingBalliOSApp::setup()
{
    // Disable the screen sleep
    [UIApplication sharedApplication].idleTimerDisabled = YES;

    mFont = Font( "Helvetica Bold", 12 );
    mTextureFont = gl::TextureFont::create( mFont );

    mClient = MPEClient::New(this);
    mClient->setIsRendering3D(true);
    mClient->set3DAspectRatio(mAspectRatio);
    mClient->set3DFieldOfView(mFOV);
    mClient->set3DCameraZ(mCamZ);
    setFrameRate(60.0f);
}

#pragma mark - UIControl callback

void MPEBouncingBalliOSApp::camZValueChanged(float value)
{
    mCamZ = value;
    send3DSettings();
}

void MPEBouncingBalliOSApp::fovValueChanged(float value)
{
    mFOV = value;
    send3DSettings();
}

void MPEBouncingBalliOSApp::aspectRatioValueChanged(float value)
{
    mAspectRatio = value;
    send3DSettings();
}

void MPEBouncingBalliOSApp::send3DSettings()
{
    if (mClient->isConnected())
    {
        mClient->sendMessage(kCommand3DSettings + "," +
                             std::to_string(mFOV) + "," +
                             std::to_string(mCamZ) + "," +
                             std::to_string(mAspectRatio));
    }
}

void MPEBouncingBalliOSApp::resetPressed()
{
    if (mClient->isConnected())
    {
        mClient->resetAll();
    }
}

#pragma mark - Balls

void MPEBouncingBalliOSApp::addBallAtPosition(const Vec2f & posBall)
{
    Vec2i sizeMaster = mClient->getMasterSize();
    Vec2f velBall = Vec2f(mRand.nextFloat(-10,10), mRand.nextFloat(-10,10));
    mBalls.push_back(Ball(posBall, velBall, sizeMaster));
}

#pragma mark - Update

void MPEBouncingBalliOSApp::update()
{
    if (!mClient->isConnected() && getElapsedFrames() % 60 == 0)
    {
        console() << "sizeMaster: " << mClient->getMasterSize() << "\n";

        mClient->start();
        mClient->setIsRendering3D(true);

    }
}

void MPEBouncingBalliOSApp::mpeFrameUpdate(long serverFrameNumber)
{
    BOOST_FOREACH(Ball & ball, mBalls)
    {
        ball.calc();
    }
}

#pragma mark - Draw

void MPEBouncingBalliOSApp::draw()
{
    gl::clear(Color(0,0,0));

    if (!mClient->isAsynchronousClient())
    {
        // There's no drawing to do for the Async client since it's a native NIB
        mClient->draw();
    }
}

void MPEBouncingBalliOSApp::mpeFrameRender(bool isNewFrame)
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (isNewFrame)
    {
        gl::color(Color(0.6, 0.525, 0.525));
    }
    else
    {
        gl::color(Color(1, 0.25, 0.25));
    }

    Vec2i masterSize = mClient->getMasterSize();
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
    Vec2f stringSize = mTextureFont->measureStringWrapped(str, Rectf(0,0,400,400));
    Vec2f stringOffset(40,40);
    Vec2f rectOffset = mClient->getVisibleRect().getUpperLeft() + stringOffset;
    gl::drawSolidRect(Rectf(rectOffset.x - 10,
                            rectOffset.y - 10,
                            rectOffset.x + 20 + stringSize.x,
                            rectOffset.y + 20 + stringSize.y - mTextureFont->getAscent()));
    gl::color(0.9,0.9,0.9);
    mTextureFont->drawStringWrapped(str,
                                    mClient->getVisibleRect(),
                                    Vec2f(0, mTextureFont->getAscent()) + stringOffset);

    glEnable (GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_position[] = { mClient->getMasterSize().x * 0.5f, 0, mCamZ, 1.0 };
    glLightfv( GL_LIGHT0, GL_POSITION, light_position );

    BOOST_FOREACH(Ball & ball, mBalls)
    {
        ball.draw();
    }

    glDisable (GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}

#pragma mark - MPE App

void MPEBouncingBalliOSApp::mpeReset()
{
    console() << "RESETTING\n";

    // Set the random seed to a known value so all of the clients are using the same rand values.
    mRand.seed(1);

    // Clear out the previous state
    mBalls.clear();

    // Add the first ball
    Vec2i sizeMaster = mClient->getMasterSize();
    addBallAtPosition(Vec2f(mRand.nextFloat(sizeMaster.x), mRand.nextFloat(sizeMaster.y)));
}

std::string MPEBouncingBalliOSApp::mpeSettingsFilename()
{
    // CLIENT_ID is a preprocessor macro defined in the Target build settings
    return "assets/settings." + std::to_string(CLIENT_ID) + ".xml";
}

void MPEBouncingBalliOSApp::mpeMessageReceived(const std::string & message, const int fromClientID)
{
    vector<string> tokens = split(message, ",");

    if (tokens.size() > 0)
    {
        string command = tokens[0];
        if (command == kCommandNewBall)
        {
            Vec2f posNewBall = Vec2f(stoi(tokens[1]),stoi(tokens[2]));
            addBallAtPosition(posNewBall);
            console() << "Adding a ball to " << posNewBall << ". Is on screen? "
                      << mClient->isOnScreen(posNewBall) << std::endl;
        }
        else if (command == kCommand3DSettings)
        {
            float fov = stof(tokens[1]);
            float camZ = stof(tokens[2]);
            float aspectRatio = stof(tokens[3]);
            mClient->set3DFieldOfView(fov);
            mClient->set3DCameraZ(camZ);
            mClient->set3DAspectRatio(aspectRatio);
        }
    }
}

#pragma mark - Touch

void MPEBouncingBalliOSApp::touchesEnded(TouchEvent event)
{
    if (mClient->isConnected())
    {
        for (int i = 0; i < event.getTouches().size(); ++i )
        {
            Vec2f pos = event.getTouches()[i].getPos() + mClient->getVisibleRect().getUpperLeft();
            string message = kCommandNewBall + "," +
                             std::to_string(pos.x) + "," +
                             std::to_string(pos.y);
            mClient->sendMessage(message);
        }
    }
}

// Woah. Changing RendererGl to RendererGl(0) vastly improves the framerate.
// AA_NONE == Anti Aliasing None == 0
CINDER_APP_NATIVE( MPEBouncingBalliOSApp, RendererGl(RendererGl::AA_NONE) )
