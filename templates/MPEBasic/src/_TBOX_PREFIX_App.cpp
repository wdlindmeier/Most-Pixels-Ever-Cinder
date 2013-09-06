#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "MPEApp.hpp"
#include "MPEClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;

// NEXT STEP: Create multiple targets
// https://github.com/wdlindmeier/Most-Pixels-Ever-Cinder/wiki/MPE-Setup-Tutorial-for-Cinder#3-create-multiple-targets

class _TBOX_PREFIX_App : public AppNative, public MPEApp
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

void _TBOX_PREFIX_App::setup()
{
    mClient = MPEClient::Create(this);
}

void _TBOX_PREFIX_App::mpeReset()
{
    // Reset the state of your app.
    // This will be called when any client connects.
}

void _TBOX_PREFIX_App::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
}

void _TBOX_PREFIX_App::mpeFrameUpdate(long serverFrameNumber)
{
    // Your update code.
}

void _TBOX_PREFIX_App::draw()
{
    mClient->draw();
}

void _TBOX_PREFIX_App::mpeFrameRender(bool isNewFrame)
{
    gl::clear(Color(0.5,0.5,0.5));
    // Your render code.
}

// If you're deploying to iOS, set the Render antialiasing to 0 for a significant
// performance improvement. This value defaults to 4 (AA_MSAA_4) on iOS and 16 (AA_MSAA_16)
// on the Desktop.
#if defined( CINDER_COCOA_TOUCH )
CINDER_APP_NATIVE( _TBOX_PREFIX_App, RendererGl(RendererGl::AA_NONE) )
#else
CINDER_APP_NATIVE( _TBOX_PREFIX_App, RendererGl )
#endif