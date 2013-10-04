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

        MPEMessageHandler() :
        mCurrentRenderFrame(0),
        mFrameIsReady(false),
        mAvgUpdateDuration(0),
        mTimeLastMessage(0),
        mUpdateSampleInterval(5)
        {};
        virtual ~MPEMessageHandler(){};

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

            if (mCurrentRenderFrame % mUpdateSampleInterval == 0)
            {
                calculateDFPS();
            }
        };

        // These are overridden in the MPEClient to handle data.
        virtual void        receivedStringMessage(const std::string & dataMessage, int fromClientID = -1){};
        virtual void        receivedResetCommand(){};

        // Average data FPS (i.e. the number of server updates per second).
        float               getUpdatesPerSecond()
        {
            return mAvgUpdateDuration > 0 ? (1.0f / mAvgUpdateDuration) : 0;
        }

    protected:

        long                mCurrentRenderFrame;
        bool                mFrameIsReady;

    private:

        void calculateDFPS()
        {
            double now = ci::app::getElapsedSeconds();
            double frameDuration = (now - mTimeLastMessage) / mUpdateSampleInterval;
            if (frameDuration > 0)
            {
                mAvgUpdateDuration = (mAvgUpdateDuration * 0.9) + (frameDuration * 0.1);
            }
            else
            {
                mAvgUpdateDuration = frameDuration;
            }

            mTimeLastMessage = now;
        }

        float               mAvgUpdateDuration;
        double              mTimeLastMessage;
        int                 mUpdateSampleInterval;

    };
}
