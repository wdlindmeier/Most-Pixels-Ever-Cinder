//
//  MPEProtocol.hpp
//  MPEClient
//
//  Created by William Lindmeier on 6/16/13.
//
//

#include "cinder/Rect.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

/*
 
 An MPE server protocol for Shiffman's Java server, which can be found here:
 https://github.com/shiffman/Most-Pixels-Ever/tree/master/server_jar
 
 To start the server:
 java -jar mpeServer.jar -debug1 -framerate30 -screens2
 
*/

using namespace std;

namespace mpe {
    
    class MPEProtocol
    {
        
    public:
        
        virtual std::string setClientID( const int clientID )
        {
            return "S" + std::to_string(clientID) + "\n";
        };
        
        virtual std::string broadcast( const std::string & msg )
        {
            return "T" + msg + "\n";
        };
        
        virtual std::string renderIsComplete()
        {
            return "D";
        };
        
        // TODO:
        // Read incoming messages        
        virtual void parse( const std::string & serverMessage )
        {
            vector<string> info;
            vector<string> frameMessage;

            istringstream iss(serverMessage);
            copy(istream_iterator<string>(iss),
                 istream_iterator<string>(),
                 back_inserter<vector<string> >(info));
            
            //ci::app::console() << "serverMessage: " << serverMessage << "\n";
            //ci::app::console() << "frameMessages:\n";
            for( int i=0; i<frameMessage.size(); i++ ){
                ci::app::console() << frameMessage[i] << "\n";
            }
            
            /*

            int fc = atoi(frameMessage[0].c_str());
            
            if (info.size() > 1) {
                // there is a message here with the frame event
                info.erase(info.begin());
                dataMessage.clear();
                dataMessage = info;
                bMessageAvailable = true;
            } else {
                bMessageAvailable = false;
            }
            
            // assume no arrays are available
            bIntsAvailable  = false;
            bBytesAvailable = false;
            
            if (fc == frameCount) {
                rendering = true;
                frameCount++;
                
                // calculate new framerate
                float ms = ofGetElapsedTimeMillis() - lastMs;
                fps = 1000.f / ms;
                lastMs = ofGetElapsedTimeMillis();
                
                if (!autoMode) {
                    parent->frameEvent();
                }
            }
             
            */
            
        }
        
    };

}