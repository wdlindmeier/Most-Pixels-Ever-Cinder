#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "MPEApp.hpp"
#include "MPEClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;

class MPEBasicApp : public AppNative, public MPEApp
{
public:
	void setup();
    void mpeReset();
    
	void update();
    void mpeFrameUpdate(long serverFrameNumber);
	
    void draw();
    void mpeFrameRender(bool isNewFrame);
    
    MPEClientRef mClient;
};

void MPEBasicApp::setup()
{
    mClient = MPEClient::Create(this);
}

void MPEBasicApp::mpeReset()
{
    // Reset the state of your app.
    // This will be called when any client connects.
}

void MPEBasicApp::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
}

void MPEBasicApp::mpeFrameUpdate(long serverFrameNumber)
{
    // Your update code.
}

void MPEBasicApp::draw()
{
    mClient->draw();
}

void MPEBasicApp::mpeFrameRender(bool isNewFrame)
{
    gl::clear(Color(0.5,0.5,0.5));
    // Your render code.
}

CINDER_APP_NATIVE( MPEBasicApp, RendererGl )
