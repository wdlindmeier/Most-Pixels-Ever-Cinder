//
//  Ball.hpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"

/*

 TODO: Describe Ball class.

*/

class Ball
{

public:

    Ball(){};
    Ball(const ci::Vec2f & randPosition,
         const ci::Vec2f & randVelocity,
         const ci::Vec2i & sizeClient) :
    mPosition(randPosition),
    mVelocity(randVelocity),
    mDiameter(36),
    mSizeClient(sizeClient)
    {};

    void calc(bool isUpdating)
    {
        // Some random vector access to try and trigger a crash.
        // This is designed to prove the thread locks.

        for (int i = 0; i < 10000; i++)
        {
            if (mTestVector.size() > i)
            {
                mTestVector[i] = std::to_string(i);
            }
            else
            {
                mTestVector.push_back(std::to_string(i));
            }
        }
        for (int i = 0; i < 10000; i++)
        {
            std::string vi = mTestVector[i];
            assert(vi == std::to_string(i));
        }
        
        if (isUpdating)
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
    }

    void draw()
    {
        ci::gl::color(0, 0, 0);
        ci::gl::lineWidth(1);
        ci::gl::drawStrokedCircle(mPosition, mDiameter * 0.5, mDiameter);
        ci::gl::color(100,100,100);
        ci::gl::drawSolidCircle(mPosition, mDiameter * 0.5, mDiameter);
    }

private:

    ci::Vec2f mPosition;
    ci::Vec2f mVelocity;
    ci::Vec2i mSizeClient;
    std::vector<std::string> mTestVector;
    float mDiameter;

};
