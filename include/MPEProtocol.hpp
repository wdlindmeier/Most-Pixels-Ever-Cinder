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

 MPEProtocol:
 This class converts actions to/from strings that the server undertands.
 MPEProtocol can be subclassed to use with other servers, if it's ever updated / modified.

 The MPE protocol suppored below is for Shiffman's Java server, which can be found here:
 https://github.com/shiffman/Most-Pixels-Ever/tree/master/server_jar

 To start the server:
 $ java -jar mpeServer.jar -debug1 -framerate30 -screens2
 or
 $ java -jar mpeServer.jar -framerate60 -screens2

*/

namespace mpe
{

    class MPEProtocol
    {
        
    protected:
        
        const std::string CONNECT_SYNCHRONOUS = "S";
        const std::string DONE_RENDERING = "D";
        const std::string DATA_MESSAGE = "T";
        const std::string NEXT_FRAME = "G";
        
    public:

#pragma mark - Outgoing Messages

        // Send the client ID once the connection has been made.
        virtual std::string setClientID(const int clientID)
        {
            return CONNECT_SYNCHRONOUS +
                   std::to_string(clientID) +
                   outgoingMessageTerminus();
        };

        // Send an arbitrary string to every connected client.
        virtual std::string broadcast(const std::string & msg)
        {
            // Make sure the delimiter isn't in the message
            assert(msg.find(dataMessageDelimiter()) == std::string::npos);
            return DATA_MESSAGE +
                   msg +
                   outgoingMessageTerminus();
        };

        // Send with the client and frame ID when the frame render is complete.
        virtual std::string renderIsComplete(int clientID, long frameNum)
        {
            return DONE_RENDERING + "," +
                   std::to_string(clientID) + "," +
                   std::to_string(frameNum+1) +
                   outgoingMessageTerminus();
        };

#pragma mark - Incoming Messages

        // The TCP client listens to the socket until it reaches the delimiter,
        // at which point the string is parsed.
        virtual std::string incomingMessageDelimiter()
        {
            const static std::string kIncomingMessageDelimiter = "\n";
            return kIncomingMessageDelimiter;
        }

        virtual std::string dataMessageDelimiter()
        {
            const static std::string kDataMessageDelimiter = ":";
            return kDataMessageDelimiter;
        }
        
        virtual std::string outgoingMessageTerminus()
        {
            const static std::string kMessageTerminus = "\n";
            return kMessageTerminus;
        }

        virtual void parse(const std::string & serverMessage, MPEMessageHandler *handler)
        {
            // Example server messages:
            // 1) IG,19919:blahblahblah
            // 2) G,7
            // 3) G,21:blah1:blah2:blah3
            //
            // Format:
            // [command],[frame count]:[data message(s)]...
            //
            // • Commands are identified by 1-2 characters (e.g. 'G' or 'IG').
            //
            // • Frame Count is the current frame tracked by the server.
            //   This is the frame number that each client should be rendering.
            //
            // • Data Messages are arbitrary strings that contains information
            //   from the other clients or server, separated by semicolons.

            std::vector<std::string> messages = ci::split(serverMessage, dataMessageDelimiter());
            std::string frame = messages[0];

            // Get the frame details
            std::vector<std::string> frameInfo = ci::split(frame, ",");
            std::string frameCommand = frameInfo[0];
            std::string frameNum = frameInfo[1];

            handler->setCurrentRenderFrame(stol(frameNum));

            // Get any additional message that was passed along.
            if (messages.size() > 1)
            {
                // Send all of the messages, one by one.
                for (int i=1;i<messages.size();i++)
                {
                    std::string dataMessage = messages[i];
                    handler->receivedStringMessage(dataMessage);
                }
            }

            if (frameCommand != NEXT_FRAME)
            {
                ci::app::console() << "ALERT: Don't know what to do with server message:" << std::endl;
                ci::app::console() << serverMessage << std::endl;
                return;
            }

            handler->setFrameIsReady(true);
        }
    };
}
