//
//  MPEClient.cpp
//  MPEClient
//
//  Created by William Lindmeier on 6/12/13.
//
//

#include "MPEClient.h"
#include "cinder/CinderResources.h"
#include "cinder/Xml.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#import <OpenGL/OpenGL.h>
#import <GLUT/GLUT.h>

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mpe;

MPEClient::MPEClient(string settingsFilename, bool shouldResize) :
MPEMessageHandler(),
mHostname(""),
mPort(0),
mIsStarted(false),
mIsRendering3D(false),
mClientID(-1)
{
    loadSettings(settingsFilename, shouldResize);
}

MPEClient::~MPEClient()
{
    if (mIsStarted){
        stop();
    }
}

#pragma mark - Connection

void MPEClient::start()
{
    if (mIsStarted)
    {
        stop();
    }
 
    mIsStarted = true;
    mTCPClient = new TCPClient();

    // Open the client
    if (mTCPClient->open(mHostname, mPort))
    {
        tcpConnected();
    }
    else
    {
        stop();
    }
}

void MPEClient::tcpConnected()
{
    ci::app::console() << "Connected.\n";
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

void MPEClient::update()
{
    printf("update\n");
    
    // This will just stall the loop until we get
    // a message from the server.    
    if (mIsStarted && mTCPClient && mTCPClient->isConnected())
    {
        mFrameIsReady = false;
        while (!mFrameIsReady)
        {
            printf("Frame is not ready\n");
            mProtocol.parse(mTCPClient->read(), this);
        }
    }
}

#pragma mark - Drawing

void MPEClient::draw(FrameEventCallback renderFrameHandler)
{
    glPushMatrix();
    
    // Only show the area of the view we're interested in.
    positionViewport();

    // Tell the app to draw.
    renderFrameHandler();

    glPopMatrix();
    
    // Tell the server we're ready for the next.
    doneRendering();
}

void MPEClient::positionViewport()
{
    if (mRepositionCallback)
    {
        mRepositionCallback( mLocalViewportRect, mIsRendering3D );
    }
    else if (mIsRendering3D)
    {
        positionViewport3D();
    }
    else
    {
        positionViewport2D();
    }
}

void MPEClient::positionViewport2D()
{
//    console() << "mLocalViewportRect: " << mLocalViewportRect << "\n";
    glTranslatef(mLocalViewportRect.getX1() * -1,
                 mLocalViewportRect.getY1() * -1,
                 0);
}

void MPEClient::positionViewport3D()
{
    float mWidth = mMasterSize.x;
    float mHeight = mMasterSize.y;
    float lWidth = mLocalViewportRect.getWidth();
    float lHeight = mLocalViewportRect.getHeight();
    float xOffset = mLocalViewportRect.getX1();
    float yOffset = mLocalViewportRect.getY1();
    
    // TMP
    // TODO: Make this real.
    // Checkout restoreCamera & setFieldOfView
    float cameraZ = 0;
    
    gluLookAt(mWidth/2.f, mHeight/2.f, cameraZ,
              mWidth/2.f, mHeight/2.f, 0,
              0, 1, 0);    
    
    // The frustum defines the 3D clipping plane for each Client window!
    float mod = .1f;
    float left   = (xOffset - mWidth/2)*mod;
    float right  = (xOffset + lWidth - mWidth/2)*mod;
    float top    = (yOffset - mHeight/2)*mod;
    float bottom = (yOffset + lHeight-mHeight/2)*mod;
    float near   = cameraZ*mod;
    float far    = 10000;
    glFrustum(left, right,
              top, bottom,
              near, far);
}

#pragma mark - Server Com

void MPEClient::broadcast(const std::string & message)
{
    mTCPClient->write(mProtocol.broadcast(message));
}

void MPEClient::sendClientID()
{
    mTCPClient->write(mProtocol.setClientID(mClientID));
}

void MPEClient::doneRendering()
{
    mTCPClient->write(mProtocol.renderIsComplete(mClientID, mRenderFrameNum));
}

/*
//  Ping is not a supported call. For testing only.
*/
void MPEClient::sendPing()
{
    // Ad-hoc
    std::string sendMsg = "P" + std::to_string(mClientID) + "\n";
    mTCPClient->write(sendMsg);
}

#pragma mark - Settings

void MPEClient::loadSettings(string settingsFilename, bool shouldResize)
{
    XmlTree settingsDoc( loadAsset( settingsFilename ) );
    
    try
    {
        XmlTree debugNode = settingsDoc.getChild( "settings/debug" );
        int isDebug = debugNode.getValue<int>();
        if(isDebug != 0){
            // app::console() << "settingsDoc: " << settingsDoc << "\n";
        }
    }
    catch (XmlTree::ExcChildNotFound e)
    {
    }
    
    try
    {
        XmlTree ipNode = settingsDoc.getChild( "settings/server/ip" );
        mHostname = ipNode.getValue<string>();
        mPort = settingsDoc.getChild( "settings/server/port" ).getValue<int>();
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        console() << "ERROR: Could not find server and port settings\n";
    }
    
    try
    {
        XmlTree ipNode = settingsDoc.getChild( "settings/client_id" );
        mClientID = ipNode.getValue<int>();
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        console() << "ERROR: Could not find client ID\n";
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
        console() << "mLocalViewportRect: " << mLocalViewportRect << "\n";

        // Force the window size based on the settings XML.
        if( shouldResize ){
            ci::app::setWindowSize(width, height);
        }
    }
    catch (XmlTree::ExcChildNotFound e)
    {
        console() << "ERROR: Could not find local dimensions settings\n";
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
        console() << "ERROR: Could not find master dimensions settings\n";
    }
}
