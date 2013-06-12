#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "MPEClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;

#if defined( CINDER_MSW )
#define SNPRINTF(...) _snprintf(__VA_ARGS__)
#elif defined( CINDER_MAC )
#define SNPRINTF(...) snprintf(__VA_ARGS__)
#endif

class MPEClientApp : public AppNative {
  
    public:
    
    void setup();
    void shutdown();
    void mouseDown( MouseEvent event );
    void update();
    void resize();
    void draw();
    void prepareSettings( Settings *settings );

    private:
    
    MPEClient *mClient;
    
};

void MPEClientApp::prepareSettings( Settings *settings )
{
    // NOTE: Initially making the window small to prove that
    // the settings.xml forces a resize.
    settings->setWindowSize( 100, 100 );
}

void MPEClientApp::setup()
{
    mClient = new MPEClient("settings.xml");
    mClient->start();
}

void MPEClientApp::shutdown()
{
    if(mClient){
        mClient->stop();
        delete(mClient);
        mClient = NULL;
    }
}

void MPEClientApp::mouseDown( MouseEvent event )
{

}

void MPEClientApp::resize()
{
    Vec2i size = getWindowSize();
    Vec2i pos = getWindowPos();
    // Send this data to the server
    
}

void MPEClientApp::update()
{
    int frameCount = getElapsedFrames();
    
    if(mClient->isConnected()){
        if(frameCount % 60 == 0){
            mClient->ping();
        }
    }else{
        // Try starting up again if we're not
        mClient->start();
    }
}

void MPEClientApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 1, 0, 0 ) );
}

CINDER_APP_NATIVE( MPEClientApp, RendererGl )
