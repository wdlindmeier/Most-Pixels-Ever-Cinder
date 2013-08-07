//
//  MPEProtocol2.hpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#include "MPEProtocol.hpp"

/*

 MPEProtocol2:
 Support for version 2 of the MPE protocol.

*/

namespace mpe
{
    const static std::string CONNECT_ASYNCHRONOUS = "A";
    const static std::string RESET_ALL = "R";
    const static std::string TOGGLE_PAUSE = "P";

    class MPEProtocol2 : public MPEProtocol
    {

    public:

#pragma mark - Outgoing Messages

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

        virtual std::string dataMessageDelimiter()
        {
            const static std::string kDataMessageDelimiter = "|";
            return kDataMessageDelimiter;
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
