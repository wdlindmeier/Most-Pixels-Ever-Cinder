//
//  MPEMessageHandler.hpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

/*

 MPEMessageHandler:
 A minimal interface that the MPEProtocol requires when parsing data.
 MPEClient is a subclass of MPEMessageHandler.

*/

namespace mpe
{
    class MPEMessageHandler
    {

    public:

        MPEMessageHandler() : mCurrentRenderFrame(0), mFrameIsReady(false){};

        // The current frame that each client is rendering.
        // This is the only MPEMessageHandler function that the App should ever call.
        long                getCurrentRenderFrame()
        {
            return mCurrentRenderFrame;
        };
        
        // The frame that every client should be rendering.
        // This is set by the protocol and should never by called by your App.
        virtual void        setCurrentRenderFrame(long frameNum)
        {
            mCurrentRenderFrame = frameNum;
        };

        // mFrameIsReady is set to true once the incoming server message is ready.
        // This is set by the protocol and should never by called by your App.
        void                setFrameIsReady(bool isFrameReady)
        {
            mFrameIsReady = isFrameReady;
        };

        // These are overridden in the MPEClient to handle data.
        virtual void        receivedStringMessage(const std::string & dataMessage, int fromClientID = -1){};
        virtual void        receivedResetCommand(){};

    protected:

        long                mCurrentRenderFrame;
        bool                mFrameIsReady;

    };
}
