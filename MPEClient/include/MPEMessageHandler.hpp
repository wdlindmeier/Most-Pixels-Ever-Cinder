//
//  MPEMessageHandler.hpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

/*

 TODO: Describe MPEMessageHandler class.

*/

class MPEMessageHandler
{

public:

    MPEMessageHandler() : mCurrentRenderFrame(0), mFrameIsReady(false){};

    // The frame that every client should be rendering.
    void setCurrentRenderFrame(int frameNum)
    {
        mCurrentRenderFrame = frameNum;
    };
    
    // mFrameIsReady is set to true once the incoming server message is ready.
    void setFrameIsReady(bool isFrameReady)
    {
        mFrameIsReady = isFrameReady;
    };

    // Overload these functions in the subclass to receive data.

    // This will be a broadcast.
    virtual void receivedStringMessage(const std::string & dataMessage)
    {};
    // Integers are waiting in the connection.
    virtual void readIncomingIntegers()
    {};
    // Bytes are waiting in the connection.
    virtual void readIncomingBytes()
    {};

protected:

    int                 mCurrentRenderFrame;
    bool                mFrameIsReady;

};
