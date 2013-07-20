//
//  MPEAsyncClient.h
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#include "MPEClient.h"
#include "TCPAsyncClient.h"

/*

 MPEAsyncClient:
 An asynchronous version of the MPEClient. Generally speaking this is the client you want to use,
 unless the multithreaded nature poses problems for your app.

 Since frame events happen as messages are received from the server, update() doesn't need to be
 called on the Async client.

*/

namespace mpe
{
    class MPEAsyncClient : public MPEClient
    {

    public:

        MPEAsyncClient() : MPEClient(){};
        MPEAsyncClient(const std::string & settingsFilename, bool shouldResize = true);
        MPEAsyncClient(const std::string & settingsFilename, MPEProtocol protocol, bool shouldResize = true);
        ~MPEAsyncClient(){};

        // Connection
        void                    start();

        // Loop
        void                    draw();
        void                    update(); // NOTE: Update has no effect in the Async client.

        // Recieving Messages
        void                    serverMessageReceived(const std::string & message);
        void                    receivedStringMessage(const std::string & dataMessage,
                                                      const int fromClientID = -1);
        void                    readIncomingIntegers();
        void                    readIncomingBytes();

    private:

        void                    tcpDidConnect(bool didConnect, const boost::system::error_code & error);

        // This lock is to protect the client data that's being updated
        // on one thread (that's communicating with the server)
        // and accessed for drawing on another thread.
        std::mutex              mClientDataMutex;
        bool                    mShouldUpdate;

    };
}
