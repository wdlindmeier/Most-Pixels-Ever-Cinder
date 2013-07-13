//
//  MPEClientApp.cpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#include "Ball.hpp"
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "ClientSettings.h"

#define USE_ASYNC   1

#if USE_ASYNC
#include "MPEAsyncClient.h"
#else
#include "MPEClient.h"
#endif

using namespace ci;
using namespace ci::app;
//using namespace std;
using std::string;
using namespace mpe;

class MPEClientApp : public AppNative
{

    public:

    void        prepareSettings( Settings *settings );
    void        setup();
    void        shutdown();
    void        mouseDown( MouseEvent event );
    void        mouseDrag( MouseEvent event );
    void        sendMousePosition();
    void        update();
    void        draw();
    void        drawViewport(bool dra);
//    void        clientUpdate();
//    void        clientDraw();
    void        frameEvent();

    private:

#if USE_ASYNC
    // NOTE: mutex is not copyable so I can't
    // use a standard member variable and just copy a new one
    // in setup. So we're using a pointer.
    MPEAsyncClient *mClient;
#else
    MPEClient   mClient;
#endif
    
    Rand        mRand;
    Ball        mBall;
    /*
    // For demonstration purposes:
    // Drag the window around the screen to change the
    // position of the client.
    Vec2i       mScreenSize;
    Vec2i       mScreenPos;
    */
};

void MPEClientApp::prepareSettings( Settings *settings )
{
    // NOTE: Initially making the window small to prove that
    // the settings.xml forces a resize.
    settings->setWindowSize( 100, 100 );
}

void MPEClientApp::setup()
{
    console() << "Loading settings from " << SettingsFileName << "\n";
    
#if USE_ASYNC
    mClient = new MPEAsyncClient(SettingsFileName);
    mClient->setFrameUpdateHandler(boost::bind(&MPEClientApp::frameEvent, this));
#else
    mClient = MPEClient(SettingsFileName);
#endif
    
    // The same as the processing sketch.
    // Does Processing Rand work the same as Cinder Rand as OF Rand?
    mRand.seed(1);

    Vec2i sizeMaster = mClient->getMasterSize();
    Vec2f posBall = Vec2f(mRand.nextFloat(sizeMaster.x), mRand.nextFloat(sizeMaster.y));
    Vec2f velBall = Vec2f(mRand.nextFloat(-5,5), mRand.nextFloat(-5,5));

    console() << "Creating ball with master size: " << sizeMaster << "\n";
    mBall = Ball(posBall, velBall, sizeMaster);

    mClient->start();
}

void MPEClientApp::shutdown()
{
    delete mClient;
    mClient = NULL;
}

void MPEClientApp::mouseDown( MouseEvent event )
{
    sendMousePosition();
}

void MPEClientApp::mouseDrag( MouseEvent event )
{
    sendMousePosition();
}

void MPEClientApp::sendMousePosition()
{
    if (mClient->isConnected())
    {
        Vec2i pos = getMousePos();
        mClient->broadcast(std::to_string(pos.x) + "," + std::to_string(pos.y));
    }
}

#pragma mark - Loop

#if USE_ASYNC

void MPEClientApp::frameEvent()
{
    mBall.calc(true);
}

#endif

void MPEClientApp::update()
{
    int frameCount = getElapsedFrames();

    
    if (mClient->isConnected())
    {

#if USE_ASYNC
        
        // Don't do anything here.
        // The async client will call the frameEvent function from
        // another thread.
#else                
 
        // It will just stall until it's ready to draw
        bool isNewDataAvailable = mClient->syncUpdate();

        if (isNewDataAvailable)
        {
            mBall.calc();            
        }

        /*
        Vec2i size = getWindowSize();
        Vec2i pos = getWindowPos();

        if (mScreenSize != size || mScreenPos != pos)
        {
            // The position has changed.
            // Update the renderable area.
            mClient->setVisibleRect(ci::Rectf(pos.x, pos.y, pos.x + size.x, pos.y + size.y));
            console() << "Visible Rect: " << mClient->getVisibleRect() << "\n";
            mScreenSize = size;
            mScreenPos = pos;
        }
        */
#endif
        
    }
    else
    {
        // Attempt to reconnect every 60 frames
        if (frameCount % 60 == 0)
        {
            mClient->start();
        }
    }
    
}

void MPEClientApp::draw()
{
    // App drawing should be done in frameEvent.
    mClient->draw(boost::bind(&MPEClientApp::drawViewport, this, _1));
}

void MPEClientApp::drawViewport(bool isNewFrame)
{
    // Just for the hell of it to see if we can crash
    mBall.calc(false);
    
    if (isNewFrame)
    {
        gl::clear(Color( 1, 0, 0 ));
    }
    else
    {
        gl::clear(Color( 1, 0, 1 ));
    }
    
    gl::color(0,0,0);
    gl::drawString(std::to_string(getElapsedFrames()), Vec2f(100, 100));
    gl::drawString(std::to_string(getAverageFps()), Vec2f(200, 100));
    mBall.draw();
}

CINDER_APP_NATIVE( MPEClientApp, RendererGl )
