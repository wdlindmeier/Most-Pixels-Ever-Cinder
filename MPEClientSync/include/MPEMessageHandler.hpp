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
        mFrameIsReady = isFrameReady;
        /*
        if (mFrameIsReady)
        {
            printf("****Frame is ready\n");
        }
        */
    }

    // Overload if desired
    virtual void receivedIntData( const std::string & intString ){};
    virtual void receivedByteData( const std::string & byteString ){};
    
protected:
    
    int                 mRenderFrameNum;
    bool                mFrameIsReady;
    
};