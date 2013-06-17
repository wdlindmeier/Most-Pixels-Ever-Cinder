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
    //void resize();
    void draw();
    void prepareSettings( Settings *settings );

    private:
    
    MPEClient *mClient;
    // For demonstration purposes:
    // Drag the window around the screen to change the
    // position of the client.
    Vec2i      mScreenSize;
    Vec2i      mScreenPos;

    
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

void MPEClientApp::update()
{

    if(mClient->isConnected()){
        
        Vec2i size = getWindowSize();
        Vec2i pos = getWindowPos();

        if( mScreenSize != size || mScreenPos != pos){
            // The position has changed.
            // Inform the server.
            mClient->sendLocalScreenRect(ci::Rectf(pos.x, pos.y, size.x, size.y));
            mScreenSize = size;
            mScreenPos = pos;
        }
        
        /*
        int frameCount = getElapsedFrames();         
        if(frameCount % 60 == 0){
            mClient->sendPing();
        }
        */
        
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
