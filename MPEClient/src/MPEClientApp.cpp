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
    
    void        setup();
    void        shutdown();
    void        mouseDown( MouseEvent event );
    void        mouseDrag( MouseEvent event );
    void        sendMousePosition();
    void        update();
    void        draw();
    void        frameEvent();
    void        prepareSettings( Settings *settings );

    private:
    
    MPEClient   mClient;
    // For demonstration purposes:
    // Drag the window around the screen to change the
    // position of the client.
    Vec2i       mScreenSize;
    Vec2i       mScreenPos;

    
};

void MPEClientApp::prepareSettings( Settings *settings )
{
    // NOTE: Initially making the window small to prove that
    // the settings.xml forces a resize.
    settings->setWindowSize( 100, 100 );
}

void MPEClientApp::setup()
{
    mClient = MPEClient("settings.xml");
    mClient.start( boost::bind(&MPEClientApp::frameEvent, this) );
}

void MPEClientApp::shutdown()
{
    mClient.stop();
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
    if( mClient.isConnected() ){
        Vec2i pos = getMousePos();
        mClient.broadcast(std::to_string(pos.x) + "," + std::to_string(pos.y));
    }
}

void MPEClientApp::update()
{
    int frameCount = getElapsedFrames();

    if(mClient.isConnected()){
        
        Vec2i size = getWindowSize();
        Vec2i pos = getWindowPos();

        if( mScreenSize != size || mScreenPos != pos){
            // The position has changed.
            // Update the renderable area.
            mClient.setVisibleRect(ci::Rectf(pos.x, pos.y, pos.x + size.x, pos.y + size.y));
            mScreenSize = size;
            mScreenPos = pos;
        }
        
        /*
        if(frameCount % 60 == 0){
            mClient->sendPing();
        }
        */
        
    }else{
        
        // Try starting up each 60 frames
        if(frameCount % 60 == 0){
            mClient.start( boost::bind(&MPEClientApp::frameEvent, this) );
        }
        
    }
}

void MPEClientApp::frameEvent()
{
    gl::clear( Color( 1, 0, 0 ) );
    gl::color(0,0,0);
    gl::drawString(std::to_string(getElapsedFrames()), Vec2f(10, 10));
}

void MPEClientApp::draw()
{
    // Do nothing
}

CINDER_APP_NATIVE( MPEClientApp, RendererGl )
