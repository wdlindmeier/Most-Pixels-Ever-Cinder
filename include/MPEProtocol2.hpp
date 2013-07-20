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
    class MPEProtocol2 : public MPEProtocol
    {
        
    protected:
        
        const std::string CONNECT_ASYNCHRONOUS = "A";
        const std::string RESET_ALL = "R";
        const std::string PAUSE_ALL = "P";
        
    public:
                
#pragma mark - Outgoing Messages
        
        virtual std::string setClientID(const int clientID)
        {
            return CONNECT_SYNCHRONOUS +
                   dataMessageDelimiter() +
                   std::to_string(clientID) +
                   outgoingMessageTerminus();
        };
        
        virtual std::string setAsyncClientID(const int clientID, bool shouldReceiveDataMessages)
        {
            return CONNECT_ASYNCHRONOUS +
                   dataMessageDelimiter() +
                   std::to_string(clientID) +
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
                   std::to_string(frameNum+1) +
                   outgoingMessageTerminus();
        };
        
        virtual std::string resetAll()
        {
            return RESET_ALL +
                   outgoingMessageTerminus();
        };

        virtual std::string pauseAll()
        {
            return PAUSE_ALL +
                   outgoingMessageTerminus();
        };

        virtual std::string broadcast(const std::string & msg)
        {
            return broadcast(msg, std::vector<int>());
        };

        virtual std::string broadcast(const std::string & msg, const std::vector<int> & toClientIDs)
        {
            assert(msg.find(dataMessageDelimiter()) == std::string::npos);
            std::string sendMessage = DATA_MESSAGE + msg;
            for (int i = 0; i < toClientIDs.size(); ++i)
            {
                sendMessage += dataMessageDelimiter() + std::to_string(toClientIDs[i]);
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
            
            if (comand != NEXT_FRAME)
            {
                ci::app::console() << "ALERT: Don't know what to do with server message:" << std::endl;
                ci::app::console() << serverMessage << std::endl;
                return;
            }
            
            std::string frameNum = tokens[1];
            handler->setCurrentRenderFrame(stol(frameNum));
            
            int numTokens = tokens.size();
            if (numTokens > 2)
            {
                // There are additional client messages
                for (int i=2;i<numTokens;i++)
                {
                    std::string dataMessage = tokens[i];
                    std::vector<std::string> messageTokens = ci::split(dataMessage, ",");
                    int clientID = stoi(messageTokens[0]);
                    std::string messageData = messageTokens[1];
                    handler->receivedStringMessage(messageData, clientID);
                }
            }
        }
    };
}
