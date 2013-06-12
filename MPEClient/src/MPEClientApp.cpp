#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "TCPClient.h"
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

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
    void draw();

    private:
    
    TCPClient *mClient;
    boost::asio::io_service mIOservice;
    std::thread mClientThread;

};

void MPEClientApp::setup()
{
    try
    {
        std::string hostname = "localhost";
        //std::string port = "9002"; // Java server
        std::string port = "7777"; // Python server

        tcp::resolver resolver(mIOservice);
        tcp::resolver::query query(hostname, port);
        tcp::resolver::iterator iterator = resolver.resolve(query);
        mClient = new TCPClient(mIOservice, iterator);
        mClientThread = std::thread(boost::bind(&boost::asio::io_service::run, &mIOservice));
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

void MPEClientApp::shutdown()
{
    mClientThread.join();
    if(mClient){
        delete(mClient);
        mClient = NULL;
    }
    mIOservice.stop();
}

void MPEClientApp::mouseDown( MouseEvent event )
{
}

void MPEClientApp::update()
{
    int frameCount = getElapsedFrames();
    
    if(mClient->isConnected()){
        if(frameCount % 60 == 0){
            // Send over the heartbeat
            std::string heartbeat = std::to_string(frameCount);
            console() << "Sending heartbeat: " << heartbeat << "\n";
            mClient->write(heartbeat);
        }
    }
}

void MPEClientApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 1, 0, 0 ) );
}

CINDER_APP_NATIVE( MPEClientApp, RendererGl )
