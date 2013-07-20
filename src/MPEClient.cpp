//
//  MPEClient.cpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <GLUT/GLUT.h>
#include <OpenGL/OpenGL.h>
#include <boost/filesystem.hpp>
#include "cinder/CinderResources.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"
#include "cinder/Xml.h"
#include "MPEClient.h"
#include "MPEProtocol2.hpp"

using std::string;
using std::vector;
using namespace ci;
using namespace ci::app;
using namespace mpe;

MPEClient::MPEClient(const std::string & settingsFilename, bool shouldResize) :
MPEClient(settingsFilename, MPEProtocol2(), shouldResize)
{
}

MPEClient::MPEClient(const string & settingsFilename, MPEProtocol protocol, bool shouldResize) :
MPEMessageHandler(),
mProtocol(protocol),
mHostname(""),
mPort(0),
mIsStarted(false),
mIsRendering3D(false),
mClientID(-1),
mIsDebug(false),
mLastFrameConfirmed(-1)
{
    loadSettings(settingsFilename, shouldResize);
}

#pragma mark - Accessors

ci::Rectf MPEClient::getVisibleRect()
{
    return mLocalViewportRect;
}

void MPEClient::setVisibleRect(const ci::Rectf & rect)
{
    mLocalViewportRect = rect;
}

ci::Vec2i MPEClient::getMasterSize()
{
    return mMasterSize;
}

bool MPEClient::getIsRendering3D()
{
    return mIsRendering3D;
}

void MPEClient::setIsRendering3D(bool is3D)
{
    mIsRendering3D = is3D;
}

#pragma mark - Callbacks

void MPEClient::setStringDataCallback(const StringDataCallback & callback)
{
    mStringDataCallback = callback;
}

void MPEClient::setFrameUpdateCallback( const FrameUpdateCallback & callback)
{
    mUpdateCallback = callback;
}

void MPEClient::setDrawCallback( const FrameRenderCallback & callback)
{
    mRenderCallback = callback;
}

#pragma mark - Connection

void MPEClient::start()
{
    if (mIsStarted)
    {
        stop();
    }

    mIsStarted = true;
    mTCPClient = new TCPClient(mProtocol.incomingMessageDelimiter());

    // Open the client
    if (mTCPClient->open(mHostname, mPort))
    {
        tcpDidConnect();
    }
    else
    {
        stop();
    }
}

// TCP Connection Callback
void MPEClient::tcpDidConnect()
{
    if (mIsDebug)
    {
        console() << "Established synchronous connection to server: "
                  << mHostname << ":" << mPort << std::endl;
    }
    sendClientID();
}

void MPEClient::stop()
{
    mIsStarted = false;
    if (mTCPClient)
    {
        mTCPClient->close();
        delete mTCPClient;
        mTCPClient = NULL;
    }
}

bool  MPEClient::isConnected()
{
    return mTCPClient && mTCPClient->isConnected();
}

#pragma mark - Update

void MPEClient::update()
{
    mFrameIsReady = false;

    // This will just stall the loop until we get
    // a message from the server.
    if (mIsStarted && isConnected())
    {
        bool isDataAvailable = true;
        while (isDataAvailable)
        {
            string data = mTCPClient->read(isDataAvailable);
            if (isDataAvailable)
            {
                mProtocol.parse(data, this);
            }
        }

        if (mFrameIsReady && mUpdateCallback)
        {
            mUpdateCallback(this->getCurrentRenderFrame());
        }
        else if (!mUpdateCallback)
        {
            console() << "WARNING: The FrameUpdateCallback has not been set." << std::endl;
        }
    }
}

#pragma mark - Drawing

void MPEClient::draw()
{
    if (mRenderCallback)
    {
        glPushMatrix();

        // Only show the area of the view we're interested in.
        positionViewport();

        // Tell the app to draw.
        mRenderCallback(mFrameIsReady);

        glPopMatrix();
    }

    if (isConnected())
    {
        // Tell the server we're ready for the next.
        doneRendering();
    }
}

void MPEClient::positionViewport()
{
    if (mIsRendering3D)
    {
        positionViewport3D();
    }
    else
    {
        positionViewport2D();
    }
}

// 2D Positioning

void MPEClient::positionViewport2D()
{
    glTranslatef(mLocalViewportRect.getX1() * -1,
                 mLocalViewportRect.getY1() * -1,
                 0);
}

// 3D Positioning
// Ported from ofxMostPixelsEver:
const static float k3DMod = 0.1f; // What is this?

void MPEClient::positionViewport3D()
{
    float mWidth = mMasterSize.x;
    float mHeight = mMasterSize.y;
    float lWidth = mLocalViewportRect.getWidth();
    float lHeight = mLocalViewportRect.getHeight();
    float xOffset = mLocalViewportRect.getX1();
    float yOffset = mLocalViewportRect.getY1();

    gluLookAt(mWidth/2.f, mHeight/2.f, mCameraZ,
              mWidth/2.f, mHeight/2.f, 0,
              0, 1, 0);

    // Client frustum
    float left   = (xOffset - mWidth/2)*k3DMod;
    float right  = (xOffset + lWidth - mWidth/2)*k3DMod;
    float top    = (yOffset - mHeight/2)*k3DMod;
    float bottom = (yOffset + lHeight-mHeight/2)*k3DMod;
    float near   = mCameraZ*k3DMod;
    float far    = 10000.0f;
    glFrustum(left, right,
              top, bottom,
              near, far);
}

void MPEClient::set3DFieldOfView(float fov)
{
    mFieldOfView = fov;
    mCameraZ = (mLocalViewportRect.getHeight() / 2.f) / tanf(M_PI * mFieldOfView/360.f);
}

float MPEClient::get3DFieldOfView()
{
    return mFieldOfView;
}

void MPEClient::restore3DCamera()
{
    Vec2f viewSize = mLocalViewportRect.getSize();
    gluLookAt(viewSize.x/2.f, viewSize.y/2.f, mCameraZ,
              viewSize.x/2.f, viewSize.y/2.f, 0,
              0, 1, 0);
    
    glFrustum(-(viewSize.x/2.f)*k3DMod, (viewSize.y/2.f)*k3DMod,
              -(viewSize.x/2.f)*k3DMod, (viewSize.y/2.f)*k3DMod,
              mCameraZ*k3DMod, 10000.0f);
}

#pragma mark - Sending Messages

void MPEClient::sendClientID()
{
    mTCPClient->write(mProtocol.setClientID(mClientID));
}

void MPEClient::sendStringData(const std::string & message)
{
    mTCPClient->write(mProtocol.broadcast(message));
}

void MPEClient::doneRendering()
{
    // Only inform the server if this is a new frame. It's possible that a given frame is
    // rendered multiple times if the server update is slower than the app loop.
    if (mLastFrameConfirmed < mCurrentRenderFrame)
    {
        mTCPClient->write(mProtocol.renderIsComplete(mClientID, mCurrentRenderFrame));
        mLastFrameConfirmed = mCurrentRenderFrame;
    }
}

#pragma mark - MPEMessageHandler

void MPEClient::setCurrentRenderFrame(long frameNum)
{
    MPEMessageHandler::setCurrentRenderFrame(frameNum);
    // mLastFrameConfirmed has to reset when the current render frame is.
    mLastFrameConfirmed = mCurrentRenderFrame - 1;
}

#pragma mark - Receiving Messages

void MPEClient::receivedStringMessage(const std::string & dataMessage, const int fromClientID)
{
    if (mStringDataCallback)
    {
        mStringDataCallback(dataMessage, fromClientID);
    }
}

#pragma mark - Settings

void MPEClient::loadSettings(string settingsFilename, bool shouldResize)
{
    // Make sure the settings file exists.
    assert(boost::filesystem::exists(getAssetPath(settingsFilename)));
    
    XmlTree settingsDoc(loadAsset(settingsFilename));
    
    try
    {
        XmlTree debugNode = settingsDoc.getChild( "settings/debug" );
        mIsDebug = debugNode.getValue<int>();
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        // Ignore
    }

    try
    {
        XmlTree ipNode = settingsDoc.getChild( "settings/server/ip" );
        mHostname = ipNode.getValue<string>();
        mPort = settingsDoc.getChild( "settings/server/port" ).getValue<int>();
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        console() << "ERROR: Could not find server and port settings." << std::endl;
    }

    try
    {
        XmlTree ipNode = settingsDoc.getChild( "settings/client_id" );
        mClientID = ipNode.getValue<int>();
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        console() << "ERROR: Could not find client ID." << std::endl;
    }

    try
    {
        XmlTree widthNode = settingsDoc.getChild( "settings/local_dimensions/width" );
        XmlTree heightNode = settingsDoc.getChild( "settings/local_dimensions/height" );
        XmlTree xNode = settingsDoc.getChild( "settings/local_location/x" );
        XmlTree yNode = settingsDoc.getChild( "settings/local_location/y" );
        int width = widthNode.getValue<int>();
        int height = heightNode.getValue<int>();
        int x = xNode.getValue<int>();
        int y = yNode.getValue<int>();
        mLocalViewportRect = Rectf( x, y, x+width, y+height );

        // Force the window size based on the settings XML.
        if (shouldResize){
            ci::app::setWindowSize(width, height);
        }
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        console() << "ERROR: Could not find local dimensions settings." << std::endl;
    }

    try
    {
        XmlTree widthNode = settingsDoc.getChild( "settings/master_dimensions/width" );
        XmlTree heightNode = settingsDoc.getChild( "settings/master_dimensions/height" );
        int width = widthNode.getValue<int>();
        int height = heightNode.getValue<int>();
        mMasterSize = Vec2i(width, height);
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        console() << "ERROR: Could not find master dimensions settings" << std::endl;
    }

    try
    {
        XmlTree fullscreenNode = settingsDoc.getChild("settings/go_fullscreen");
        string boolStr = fullscreenNode.getValue<string>();
        std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);
        if (boolStr == "true" && shouldResize)
        {
            ci::app::setFullScreen(true);
        }
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        console() << "ERROR: Could not find master dimensions settings" << std::endl;
    }
    
    try
    {
        XmlTree fullscreenNode = settingsDoc.getChild("settings/offset_window");
        string boolStr = fullscreenNode.getValue<string>();
        std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);
        if (boolStr == "true" && shouldResize)
        {
            // Reposition the screen
            ci::app::setWindowPos(Vec2i(mLocalViewportRect.x1, mLocalViewportRect.y1));
        }
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        console() << "ERROR: Could not find master dimensions settings" << std::endl;
    }
}
