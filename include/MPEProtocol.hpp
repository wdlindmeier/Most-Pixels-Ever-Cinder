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
 
 */

namespace mpe
{
    const static std::string CONNECT_SYNCHRONOUS = "S";
    const static std::string DONE_RENDERING = "D";
    const static std::string DATA_MESSAGE = "T";
    const static std::string NEXT_FRAME = "G"; 
    const static std::string CONNECT_ASYNCHRONOUS = "A";
    const static std::string RESET_ALL = "R";
    const static std::string TOGGLE_PAUSE = "P";

    class MPEProtocol
    {
    public:

#pragma mark - Outgoing Messages
        
        virtual std::string cleanMessage(std::string message)
        {
            // Make sure the delimiter isn't in the message
            if (message.find(dataMessageDelimiter()) != std::string::npos)
            {
                ci::app::console() << "WARNING: '" << dataMessageDelimiter()
                << "' are not allowed in broadcast messages."
                << " Replacing with an underscore." << std::endl;
                std::replace(message.begin(), message.end(), dataMessageDelimiter().at(0), '_');
            }
            if (message.find(outgoingMessageTerminus()) != std::string::npos)
            {
                std::string termID = "Newlines";
                if (outgoingMessageTerminus() != "\n")
                {
                    termID = "'" + outgoingMessageTerminus() + "'";
                }
                ci::app::console() << "WARNING: " << termID
                << " are not allowed in broadcast messages."
                << " Replacing with an underscore." << std::endl;
                std::replace(message.begin(), message.end(), dataMessageDelimiter().at(0), '_');
            }
            return message;
        }

        virtual std::string setClientID(const int clientID)
        {
            return setClientID(clientID, "Rendering Client " + std::to_string(clientID));
        };

        virtual std::string setClientID(const int clientID, const std::string & name)
        {
            return CONNECT_SYNCHRONOUS +
                   dataMessageDelimiter() +
                   std::to_string(clientID) +
                   dataMessageDelimiter() +
                   name +
                   outgoingMessageTerminus();
        };

        virtual std::string setAsyncClientID(const int clientID)
        {
            return setAsyncClientID(clientID, "Non-Rendering Client " + std::to_string(clientID));
        }

        virtual std::string setAsyncClientID(const int clientID,
                                             const std::string & clientName,
                                             bool shouldReceiveDataMessages = false)
        {
            return CONNECT_ASYNCHRONOUS +
                   dataMessageDelimiter() +
                   std::to_string(clientID) +
                   dataMessageDelimiter() +
                   clientName +
                   dataMessageDelimiter() +
                   (shouldReceiveDataMessages ? "true" : "false") +
                   outgoingMessageTerminus();
        };

        virtual std::string renderIsComplete(int clientID, long frameNum)
        {
            return DONE_RENDERING +
                   dataMessageDelimiter() +
                   std::to_string(clientID) +
                   dataMessageDelimiter() +
                   std::to_string(frameNum) +
                   outgoingMessageTerminus();
        };

        virtual std::string resetAll()
        {
            return RESET_ALL +
                   outgoingMessageTerminus();
        };

        virtual std::string togglePause()
        {
            return TOGGLE_PAUSE +
                   outgoingMessageTerminus();
        };

        virtual std::string broadcast(const std::string & msg)
        {
            return broadcast(msg, std::vector<int>());
        };

        virtual std::string broadcast(const std::string & msg, const std::vector<int> & toClientIDs)
        {
            std::string sanitizedMessage = cleanMessage(msg);
            std::string sendMessage = DATA_MESSAGE + dataMessageDelimiter() + sanitizedMessage;
            for (int i = 0; i < toClientIDs.size(); ++i)
            {
                if (i == 0)
                {
                    sendMessage += dataMessageDelimiter();
                }
                else
                {
                    sendMessage += ",";
                }
                sendMessage += std::to_string(toClientIDs[i]);
            }
            sendMessage += outgoingMessageTerminus();
            return sendMessage;
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
            const static std::string kDataMessageDelimiter = "|";
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
            // 1) G|19919|fromID,blahblahblah
            // 2) G|7
            // 3) G|21|fromID,blah1234|fromID,blah210|fromID,blah345623232
            //
            // Format:
            // [command]|[frame count]|[data message(s)]...
            //
            // • Command is always NEXT_FRAME ("G") in v.2
            //
            // • Data Messages will start with the senders Client ID followed by a comma.
            //

            std::vector<std::string> tokens = ci::split(serverMessage, dataMessageDelimiter());
            std::string comand = tokens[0];

            if (comand == RESET_ALL)
            {
                handler->setCurrentRenderFrame(0);
                handler->receivedResetCommand();
            }
            else if (comand == NEXT_FRAME)
            {
                std::string frameNum = tokens[1];
                handler->setCurrentRenderFrame(stol(frameNum));

                int numTokens = tokens.size();
                if (numTokens > 2)
                {
                    // There are additional client messages
                    for (int i=2;i<numTokens;i++)
                    {
                        // Iterate over the messages and send them out
                        std::string dataMessage = tokens[i];
                        size_t firstComma = dataMessage.find_first_of(",");
                        if (firstComma != std::string::npos)
                        {
                            int clientID = stoi(dataMessage.substr(0, firstComma));
                            std::string messageData = dataMessage.substr(firstComma+1);
                            handler->receivedStringMessage(messageData, clientID);
                        }
                        else
                        {
                            ci::app::console() << "ERROR: Couldn't parse data message "
                            << dataMessage << std::endl;
                        }
                    }
                }

                handler->setFrameIsReady(true);
            }
            else
            {
                ci::app::console() << "ALERT: Don't know what to do with server message: "
                                   << serverMessage << std::endl;
                return;
            }
        }
    };
}
