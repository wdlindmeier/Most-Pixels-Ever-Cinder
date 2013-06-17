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
#include <boost/lambda/lambda.hpp>

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

MPEClient::~MPEClient()
{
    if(mIsStarted){
        stop();
    }
}

#pragma mark - Connection

void MPEClient::start()
{
    if(mIsStarted){
        stop();
    }
    
    mIsStarted = true;
    
    mTCPClient = new TCPClient();
    
    OpenedCallback callback = boost::bind(&MPEClient::handleTCPConnect,
                                          this,
                                          boost::lambda::_1,
                                          boost::lambda::_2);
    mTCPClient->open(mHostname, mPort, callback);

}

void MPEClient::handleTCPConnect(bool didConnect, const boost::system::error_code& error)
{
    if(didConnect){
        console() << "TCP Client Connected\n";
        sendClientID();
    }else{
        stop();
        console() << "Error connecting: " << error.message() << "\n";
    }
}

void MPEClient::stop()
{    
    mIsStarted = false;
    if(mTCPClient){
        mTCPClient->close();
        delete mTCPClient;
        mTCPClient = NULL;
    }    
}

#pragma mark - Server Com

void MPEClient::sendLocalScreenRect(ci::Rectf localRect)
{
    std::string sendMsg = mProtocol.setLocalViewRect(localRect);
    mTCPClient->write(sendMsg);
}

void MPEClient::sendClientID()
{
    std::string sendMsg = mProtocol.setClientID(mClientID);
    mTCPClient->write(sendMsg);
}

void MPEClient::sendPing()
{
    int randNum = arc4random() % 1000;
    mTCPClient->write(std::to_string(randNum));
}

#pragma mark - Settings

void MPEClient::loadSettings(string settingsFilename, bool shouldResize)
{
    XmlTree settingsDoc( loadAsset( settingsFilename ) );
    
    try {
        XmlTree debugNode = settingsDoc.getChild( "settings/debug" );
        int isDebug = debugNode.getValue<int>();
        if(isDebug != 0){
            // app::console() << "settingsDoc: " << settingsDoc << "\n";
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
        XmlTree ipNode = settingsDoc.getChild( "settings/client_id" );
        mClientID = ipNode.getValue<int>();
    } catch (XmlTree::ExcChildNotFound e) {
        console() << "ERROR: Could not find client ID\n";
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
