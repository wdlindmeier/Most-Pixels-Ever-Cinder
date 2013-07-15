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
    void        mouseDown(MouseEvent event);
    void        mouseDrag(MouseEvent event);
    void        keyDown(KeyEvent event);
    void        sendMousePosition();
    void        update();
    void        draw();
    void        drawViewport(bool dra);
//    void        clientUpdate();
//    void        clientDraw();
    void        frameEvent();

    // Data Callbacks
    void        stringDataReceived(const std::string & message);
    void        integerDataReceived(const std::vector<int> & integers);
    void        bytesDataReceived(const std::vector<char> & bytes);
    
    private:

#if USE_ASYNC
    // NOTE: mutex is not copyable so I can't
    // use a standard member variable and just copy a new one
    // in setup. So we're using a pointer.
    MPEAsyncClient *mClient;
#else
    MPEClient   *mClient;
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
    mClient->setStringDataCallback(boost::bind(&MPEClientApp::stringDataReceived, this, _1));
    mClient->setIntegerDataCallback(boost::bind(&MPEClientApp::integerDataReceived, this, _1));
    mClient->setBytesDataCallback(boost::bind(&MPEClientApp::bytesDataReceived, this, _1));

#else
    
    mClient = new MPEClient(SettingsFileName);
    
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

void MPEClientApp::mouseDown(MouseEvent event)
{
    sendMousePosition();
}

void MPEClientApp::mouseDrag(MouseEvent event)
{
    sendMousePosition();
}

void MPEClientApp::sendMousePosition()
{
    if (mClient->isConnected())
    {
        Vec2i pos = getMousePos();
        mClient->sendStringData(std::to_string(pos.x) + "," + std::to_string(pos.y));
    }
}

void MPEClientApp::keyDown(KeyEvent event)
{
    // NOTE:
    // Int/Byte arrays are not yet supported
    if (mClient->isConnected())
    {
        if (event.getChar() == 'i')
        {
            std::vector<int> ints = {1,2,3,4,5};
            mClient->sendIntegerData(ints);
        }
        else if (event.getChar() == 'b')
        {
            std::vector<char> bytes = {'a','b','c','d','e'};
            mClient->sendBytesData(bytes);
        }
    }
}

#pragma mark - Data

void MPEClientApp::stringDataReceived(const std::string & message)
{
    console() << "stringDataReceived: " << message << "\n";
}

void MPEClientApp::integerDataReceived(const std::vector<int> & integers)
{
    string outp = "";
    for (int i = 0; i < integers.size(); ++i)
    {
        outp += std::to_string(integers[i]);
    }
    console() << "integerDataReceived: " << outp << "\n";
}

void MPEClientApp::bytesDataReceived(const std::vector<char> & bytes)
{
    string outp = "";
    for (int i = 0; i < bytes.size(); ++i)
    {
        outp += std::to_string(bytes[i]);
    }
    console() << "bytesDataReceived: " << outp << "\n";
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

    // NOTE: Async client should update the app state in frameEvent,
    // not update.
        
#if !USE_ASYNC
        // It will just stall until it's ready to draw
        bool isNewDataAvailable = mClient->shouldUpdate();

        if (isNewDataAvailable)
        {
            //console() << "isNewDataAvailable\n";
            mBall.calc(true);
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
    // NOTE: The MPEClient will handle the repositioning in a
    // default way, but if you want to handle this yourself,
    // just pass clientDraw a NULL render handler.
    //
    // E.g. Draw without repositioning:
        // drawViewport(true);
        // mClient->draw(NULL);
    //
    // Draw with repositioning:
    
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
