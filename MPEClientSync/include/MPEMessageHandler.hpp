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

    MPEMessageHandler() : mRenderFrameNum(0), mFrameIsReady(false){};

    void setRenderFrameNum(int frameNum){ mRenderFrameNum = frameNum; };
    void setFrameIsReady(bool isFrameReady) { mFrameIsReady = isFrameReady; };

    // Overload if desired
    virtual void receivedIntData( const std::string & intString ){};
    virtual void receivedByteData( const std::string & byteString ){};

protected:

    int                 mRenderFrameNum;
    bool                mFrameIsReady;

};
