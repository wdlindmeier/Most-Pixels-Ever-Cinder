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
 
 The basic MPE protocol found below is for Shiffman's Java server, which can be found here:
 https://github.com/shiffman/Most-Pixels-Ever/tree/master/server_jar

 To start the server:
 $ java -jar mpeServer.jar -debug1 -framerate30 -screens2
 or
 $ java -jar mpeServer.jar -framerate60 -screens2

 This is used by default, but subclassed protocols can also be used by passing them into 
 the MPEClient constructor.

*/

namespace mpe
{
    class MPEProtocol
    {

    public:

        const static char kMessageTerminus = '\n';

#pragma mark - Outgoing Messages

        // S == Start
        // Send the client ID once the connection has been made.
        virtual std::string setClientID( const int clientID )
        {
            return "S" + std::to_string(clientID) + kMessageTerminus;
        };

        // T == daTa
        // Send an arbitrary string to every connected client.
        virtual std::string broadcast( const std::string & msg )
        {
            return "T" + msg + kMessageTerminus;
        };

        // D == Done
        // Send with the client and frame ID when the frame render is complete.
        virtual std::string renderIsComplete(int clientID, long frameNum)
        {
            return "D," + std::to_string(clientID) + "," + std::to_string(frameNum+1) + kMessageTerminus;
        };

        // I == Integers (not supported?)
        virtual boost::asio::const_buffers_1 sendInts(const std::vector<int> & integers)
        {
            // TODO: Make this more efficient
            char command = 'I';
            size_t bufferLength = sizeof(command) + (integers.size() * sizeof(int));
            char data[bufferLength];
            data[0] = command;
            for (int i = 0; i<integers.size(); ++i)
            {
                int idx = sizeof(command) + (i * sizeof(int));
                data[idx] = integers[i];
            }
            return boost::asio::buffer((const void *)data, bufferLength);
        }

        // B == Bytes (not supported?)
        virtual boost::asio::const_buffers_1 sendBytes(const std::vector<char> & bytes)
        {
            // TODO: Make this more efficient
            char command = 'B';
            size_t bufferLength = sizeof(command) + (bytes.size() * sizeof(char));
            char data[bufferLength];
            data[0] = command;
            for (int i = 0; i<bytes.size(); ++i)
            {
                int idx = sizeof(command) + (i * sizeof(char));
                data[idx] = bytes[i];
            }
            return boost::asio::buffer((const void *)data, bufferLength);
        }

#pragma mark - Incoming Messages

        // The TCP client listens to the socket until it reaches the delimiter,
        // at which point the string is parsed.
        virtual std::string incomingMessageDelimiter()
        {
            const static std::string kIncomingMessageDelimiter = "\n";
            return kIncomingMessageDelimiter;
        }

        virtual void parse(const std::string & serverMessage, MPEMessageHandler *handler)
        {
            // Example server messages:
            // 1) IG,19919:blahblahblah
            // 2) G,7
            // 1) G,21:blah1:blah2:blah3
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

            std::vector<std::string> messages = ci::split(serverMessage, ":");
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
                ci::app::console() << "ALERT: Don't know what to do with server message:" << std::endl;
                ci::app::console() << serverMessage << std::endl;
                return;
            }

            handler->setFrameIsReady(true);
        }
    };
}
