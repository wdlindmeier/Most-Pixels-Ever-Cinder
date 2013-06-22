//
//  MPEProtocol.hpp
//  MPEClient
//
//  Created by William Lindmeier on 6/16/13.
//
//

#include "cinder/Rect.h"
#include "cinder/Utilities.h"
#include "MPEMessageHandler.hpp"
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

using std::string;
using std::vector;
using namespace boost;

namespace mpe {
    
    class MPEProtocol
    {
        
    public:
        
        virtual string setClientID( const int clientID )
        {
            return "S" + std::to_string(clientID) + "\n";
        };
        
        virtual string broadcast( const string & msg )
        {
            return "T" + msg + "\n";
        };
        
        virtual string renderIsComplete(int clientID, long frameNum)
        {
            return "D," + std::to_string(clientID) + "," + std::to_string(frameNum+1) + "\n";
        };
        
        virtual void parse(const string & serverMessage, MPEMessageHandler *handler)
        {            
            // ci::app::console() << "Incoming message: " << serverMessage << "\n";
            
            // Example server messages
            // e.g. IG,19919:blahblahblah
            // e.g. G,7
            
            vector<string> messages = ci::split(serverMessage, ":");
            string frame = messages[0];
            string payload;
            
            // Get the frame details
            vector<string> frameInfo = ci::split(frame, ",");
            string frameCommand = frameInfo[0];
            string frameNum = frameInfo[1];
            
            // Get any additional message that was passed along
            bool hasPayload = messages.size() > 1;
            if (hasPayload)
            {
                payload = messages[1];
            }
            
            if (frameCommand == "G")
            {
                // Just sending frame num
                // Nothing else to do here.
            }
            else if (frameCommand == "IG")
            {
                if (hasPayload)
                {
                    handler->receivedIntData(payload);
                }
            }
            else if (frameCommand == "BG")
            {
                if (hasPayload)
                {
                    handler->receivedByteData(payload);
                }
            }
            else
            {
                ci::app::console() << "ALERT: Don't know what to do with server message:\n";
                ci::app::console() << serverMessage << "\n";
                return;
            }

            handler->setRenderFrameNum(stoi(frameNum));
            handler->setFrameIsReady(true);
        }
    };
}