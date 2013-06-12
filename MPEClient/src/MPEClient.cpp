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
#include <boost/bind.hpp>

using namespace std;
using namespace ci;
using namespace ci::app;

MPEClient::MPEClient(string settingsFilename, bool shouldResize ) :
mHostname(""),
mPort(0),
mIsStarted(false)
{
    loadSettings(settingsFilename, shouldResize);
}

void MPEClient::loadSettings(string settingsFilename, bool shouldResize)
{
    XmlTree settingsDoc( loadAsset( settingsFilename ) );
    
    try {
        XmlTree debugNode = settingsDoc.getChild( "settings/debug" );
        int isDebug = debugNode.getValue<int>();
        if(isDebug != 0){
            app::console() << "settingsDoc: " << settingsDoc << "\n";
        }
    } catch (XmlTree::ExcChildNotFound e) {
    }
    
    try {
        XmlTree ipNode = settingsDoc.getChild( "settings/server/ip" );
        mHostname = ipNode.getValue<string>();
        mPort = settingsDoc.getChild( "settings/server/port" ).getValue<int>();
    } catch (XmlTree::ExcChildNotFound e) {
        console() << "ERROR: Could not find server and port settings\n";
    }

    try {
        XmlTree widthNode = settingsDoc.getChild( "settings/local_dimensions/width" );
        XmlTree heightNode = settingsDoc.getChild( "settings/local_dimensions/height" );
        int width = widthNode.getValue<int>();
        int height = heightNode.getValue<int>();
        mLocalSize = Vec2i(width, height);
        // Force the window size based on the settings XML
        if( shouldResize ){
            ci::app::setWindowSize(width, height);
        }

    } catch (XmlTree::ExcChildNotFound e) {
        console() << "ERROR: Could not find local dimensions settings\n";
    }

}

void MPEClient::start()
{
    if(mIsStarted){
        stop();
    }    
    mIsStarted = true;
    tcp::resolver resolver(mIOservice);
    tcp::resolver::query query(mHostname, std::to_string(mPort));
    tcp::resolver::iterator iterator = resolver.resolve(query);
    
    mTCPClient = new TCPClient(mIOservice, iterator);
    mClientThread = std::thread(boost::bind(&boost::asio::io_service::run, &mIOservice));
}

void MPEClient::stop()
{    
    mIsStarted = false;
    mClientThread.join();
    if(mTCPClient){
        delete mTCPClient;
        mTCPClient = NULL;
    }
    mIOservice.stop();
}

void MPEClient::ping()
{
    int randNum = arc4random() % 1000;
    mTCPClient->write(std::to_string(randNum));
}

MPEClient::~MPEClient()
{
    if(mIsStarted){
        stop();
    }
}