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
        
#pragma mark - Outgoing Messages
        
        // S == Start
        // Send the client ID once the connection has been made.
        virtual std::string setClientID( const int clientID )
        {
            return "S" + std::to_string(clientID) + "\n";
        };

        // T == daTa
        // Send an arbitrary string to every connected client.
        virtual std::string broadcast( const std::string & msg )
        {
            return "T" + msg + "\n";
        };

        // D == Done
        // Send with the client and frame ID when the frame render is complete.
        virtual std::string renderIsComplete(int clientID, long frameNum)
        {
            return "D," + std::to_string(clientID) + "," + std::to_string(frameNum+1) + "\n";
        };

#pragma mark - Incoming Messages
        
        virtual void parse(const std::string & serverMessage, MPEMessageHandler *handler)
        {
            // ci::app::console() << "Incoming message: " << serverMessage << "\n";

            // Example server messages:
            // 1) IG,19919:blahblahblah
            // 2) G,7
            //
            // Format:
            // [command],[frame count]:[data message]
            //
            // • Commands are identified by 1-2 characters (e.g. 'G' or 'IG').
            //
            // • Frame Count is the current frame tracked by the server.
            //   This is the frame number that each client should be rendering.
            //
            // • Data Message is an arbitrary string that contains information
            //   from the other clients or server.

            std::vector<std::string> messages = ci::split(serverMessage, ":");
            std::string frame = messages[0];

            // Get the frame details
            std::vector<std::string> frameInfo = ci::split(frame, ",");
            std::string frameCommand = frameInfo[0];
            std::string frameNum = frameInfo[1];

            handler->setCurrentRenderFrame(stoi(frameNum));

            // Get any additional message that was passed along.
            if (messages.size() > 1)
            {
                std::string dataMessage = messages[1];
                handler->receivedBroadcast(dataMessage);
            }

            if (frameCommand == "G")
            {
                // This is simply the current frame
            }
            else if (frameCommand == "IG")
            {
                // There are ints available.
                // The next message will be in the following format:
                // [n]iiiiiiiiiiii...
                // The first 4 bytes is an int declaring the length of the subsequent payload.
                // Keep reading [i] ints for n*4 bytes.
            }
            //
            else if (frameCommand == "BG")
            {
                // There are bytes available.
                // The next message will be in the following format:
                // [n]bbbbbbbbbbbb...
                // The first 4 bytes is an int declaring the length of the subsequent payload.
                // Keep reading [b] bytes for n bytes.
            }
            else
            {
                ci::app::console() << "ALERT: Don't know what to do with server message:\n";
                ci::app::console() << serverMessage << "\n";
                return;
            }

            handler->setFrameIsReady(true);
        }
    };
}
