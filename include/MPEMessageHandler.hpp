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

        // The frame that every client should be rendering.
        // This is set by the protocol and should never by called by your App.
        virtual void setCurrentRenderFrame(long frameNum)
        {
            mCurrentRenderFrame = frameNum;
        };

        long getCurrentRenderFrame()
        {
            return mCurrentRenderFrame;
        };

        // mFrameIsReady is set to true once the incoming server message is ready.
        void setFrameIsReady(bool isFrameReady)
        {
            mFrameIsReady = isFrameReady;
        };

        // Overload these functions in the subclass to receive data.

        virtual void receivedStringMessage(const std::string & dataMessage, int fromClientID = -1)
        {};

    protected:

        long                mCurrentRenderFrame;
        bool                mFrameIsReady;

    };
}
