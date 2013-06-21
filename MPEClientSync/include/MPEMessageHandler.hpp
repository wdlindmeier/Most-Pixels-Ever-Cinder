//
//  MPEMessageHandler.hpp
//  MPEClient
//
//  Created by William Lindmeier on 6/21/13.
//
//

#pragma once

class MPEMessageHandler
{
 
public:
    
    MPEMessageHandler() : mRenderFrameNum(0), mFrameIsReady(false){};
    
    void setRenderFrameNum(int frameNum){ mRenderFrameNum = frameNum; };
    void setFrameIsReady(bool isFrameReady)
    {
        if (isFrameReady)
        {
            printf("****Frame is ready\n");
        }
        mFrameIsReady = isFrameReady;
    }

    // Overload if desired
    virtual void receivedIntData( const std::string & intString ){};
    virtual void receivedByteData( const std::string & byteString ){};
    
protected:
    
    int                 mRenderFrameNum;
    bool                mFrameIsReady;
    
};