//
//  Ball.hpp
//  MPEClient
//
//  Created by William Lindmeier on 6/22/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"

using namespace ci;

class Ball
{
    
public:
    
    Ball(){};
    Ball(const Vec2f & randPosition,
         const Vec2f & randVelocity,
         const Vec2i & sizeClient) :
    mPosition(randPosition),
    mVelocity(randVelocity),
    mDiameter(36),
    mSizeClient(sizeClient)
    {};
    
    void calc()
    {
        if (mPosition.x < 0 || mPosition.x > mSizeClient.x)
        {
            mVelocity.x = mVelocity.x * -1;
        }
        if (mPosition.y < 0 || mPosition.y > mSizeClient.y)
        {
            mVelocity.y = mVelocity.y * -1;
        }
        mPosition += mVelocity;
    }
    
    void draw()
    {
        gl::color(0, 0, 0);
        gl::lineWidth(1);
        gl::drawStrokedCircle(mPosition, mDiameter * 0.5, mDiameter);
        gl::color(100,100,100);
        gl::drawSolidCircle(mPosition, mDiameter * 0.5, mDiameter);
    }
    
private:
    
    Vec2f mPosition;
    Vec2f mVelocity;
    Vec2i mSizeClient;
    float mDiameter;
    
};