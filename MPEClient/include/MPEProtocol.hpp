//
//  MPEProtocol.hpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include "cinder/Rect.h"
#include "cinder/Utilities.h"
#include "MPEMessageHandler.hpp"

/*

 An MPE server protocol for Shiffman's Java server, which can be found here:
 https://github.com/shiffman/Most-Pixels-Ever/tree/master/server_jar

 To start the server:
 $ java -jar mpeServer.jar -debug1 -framerate30 -screens2
 or
 $ java -jar mpeServer.jar -framerate60 -screens2

 This class converts actions to/from strings that the server undertands.
 MPEProtocol can be subclassed to use with other servers, if it's ever updated / rewritten.

*/

namespace mpe
{
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

        virtual std::string renderIsComplete(int clientID, long frameNum)
        {
            return "D," + std::to_string(clientID) + "," + std::to_string(frameNum+1) + "\n";
        };

        virtual void parse(const std::string & serverMessage, MPEMessageHandler *handler)
        {
            // ci::app::console() << "Incoming message: " << serverMessage << "\n";

            // Example server messages
            // e.g. IG,19919:blahblahblah
            // e.g. G,7

            std::vector<std::string> messages = ci::split(serverMessage, ":");
            std::string frame = messages[0];
            std::string payload;

            // Get the frame details
            std::vector<std::string> frameInfo = ci::split(frame, ",");
            std::string frameCommand = frameInfo[0];
            std::string frameNum = frameInfo[1];

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
