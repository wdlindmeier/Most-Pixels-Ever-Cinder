#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include "Ball.hpp"
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
    //void            touchesBegan(TouchEvent event);
	//void            touchesMoved(TouchEvent event);
	void            touchesEnded(TouchEvent event);
    
    // UIKit Controls
    void            camZValueChanged(float value);
    void            fovValueChanged(float value);
    void            aspectRatioValueChanged(float value);
    void            resetPressed();
    
  private:
    
    MPEClient::Ptr  mClient;
    
    Rand            mRand;
    vector<Ball>    mBalls;
    
    float           mCamZ;
    float           mFOV;
    float           mAspectRatio;
    
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
}

void MPEBouncingBalliOSApp::setup()
{
    // Lil Obj-C++ to disable the screen sleep
    [UIApplication sharedApplication].idleTimerDisabled = YES;

    mClient = MPEClient::New(this);
    mClient->setIsRendering3D(true);
    setFrameRate(30.0f);
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
    mClient->sendMessage(kCommand3DSettings + "," +
                         std::to_string(mFOV) + "," +
                         std::to_string(mCamZ) + "," +
                         std::to_string(mAspectRatio));
}

void MPEBouncingBalliOSApp::resetPressed()
{
    mClient->resetAll();
}

#pragma mark - Balls

void MPEBouncingBalliOSApp::addBallAtPosition(const Vec2f & posBall)
{
    Vec2i sizeMaster = mClient->getMasterSize();
    Vec2f velBall = Vec2f(mRand.nextFloat(-5,5), mRand.nextFloat(-5,5));
    mBalls.push_back(Ball(posBall, velBall, sizeMaster));
}

#pragma mark - Update

void MPEBouncingBalliOSApp::update()
{
    if (!mClient->isConnected() && getElapsedFrames() % 60 == 0)
    {
        console() << "sizeMaster: " << mClient->getMasterSize() << "\n";
        
        mClient->start("10.0.1.19", 9002);
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
    gl::clear(Color(0.5,0.5,0.5));
    
    if (!mClient->isAsynchronousClient())
    {
        // There's no drawing to do for the Async client since it's a native NIB
        mClient->draw();
    }
}


void MPEBouncingBalliOSApp::mpeFrameRender(bool isNewFrame)
{
    gl::clear(Color(0,0,0));

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

    gl::drawString("Client ID: " + std::to_string(CLIENT_ID),
                   Vec2f(mClient->getVisibleRect().getX1() + 20, 20));
    gl::drawString("FPS: " + std::to_string((int)getAverageFps()),
                   Vec2f(mClient->getVisibleRect().getX1() + 20, 50));
    gl::drawString("Frame Num: " + std::to_string(mClient->getCurrentRenderFrame()),
                   Vec2f(mClient->getVisibleRect().getX1() + 20, 80));
    gl::drawString("Updates Per Second: " + std::to_string((int)mClient->getUpdatesPerSecond()),
                   Vec2f(mClient->getVisibleRect().getX1() + 20, 120));

    BOOST_FOREACH(Ball & ball, mBalls)
    {
        ball.draw();
    }
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
    for (int i = 0; i < event.getTouches().size(); ++i )
    {
        Vec2f pos = event.getTouches()[i].getPos() + mClient->getVisibleRect().getUpperLeft();
        string message = kCommandNewBall + "," +
                         std::to_string(pos.x) + "," +
                         std::to_string(pos.y);
        mClient->sendMessage(message);
    }
}

// Woah. Changing RendererGl to RendererGl(0) vastly improves the framerate.
// AA_NONE == Anti Aliasing None == 0
CINDER_APP_NATIVE( MPEBouncingBalliOSApp, RendererGl(RendererGl::AA_NONE) )