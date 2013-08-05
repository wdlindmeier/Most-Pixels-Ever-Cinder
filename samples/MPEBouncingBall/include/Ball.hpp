//
//  Ball.hpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"

/*

 Ball:
 A simple ball that bounces around the screen.
 Ball also has some meaningless internal data that's manipulated every frame
 so we can ensure that the Async client is thread-safe.

*/

const static int kDefaultRadius = 30;

class Ball
{

public:

    Ball(){};
    Ball(const ci::Vec2f & randPosition,
         const ci::Vec2f & randVelocity,
         const ci::Vec2i & sizeClient) :
    mPosition(ci::math<float>::clamp(randPosition.x, kDefaultRadius, sizeClient.x - kDefaultRadius),
              ci::math<float>::clamp(randPosition.y, kDefaultRadius, sizeClient.y - kDefaultRadius)),
    mVelocity(randVelocity),
    mDiameter(kDefaultRadius*2),
    mSizeClient(sizeClient)
    {};

    // This function performs internal data read/write to try and trigger a crash.
    // This is just here to prove that the thread locks are working.
    void manipulateInternalData()
    {
        for (int i = 0; i < 1000; i++)
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
        for (int i = 0; i < 1000; i++)
        {
            std::string vi = mTestVector[i];
            assert(vi == std::to_string(i));
        }
    }

    void calc()
    {
        float radius = mDiameter * 0.5;

        if (mPosition.x < radius || mPosition.x > (mSizeClient.x - radius))
        {
            mVelocity.x = mVelocity.x * -1;
        }
        if (mPosition.y < radius || mPosition.y > (mSizeClient.y - radius))
        {
            mVelocity.y = mVelocity.y * -1;
        }

        mPosition += mVelocity;
    }

    void draw()
    {
        ci::gl::color(0, 0, 0);
        ci::gl::lineWidth(1);
        ci::gl::drawStrokedCircle(mPosition, mDiameter * 0.5, mDiameter);
        ci::gl::color(100,100,100);
        ci::gl::drawCube(ci::Vec3f(mPosition, 5), ci::Vec3f(mDiameter,
                                                            mDiameter,
                                                            mDiameter));
    }

private:

    ci::Vec2f mPosition;
    ci::Vec2f mVelocity;
    ci::Vec2i mSizeClient;
    std::vector<std::string> mTestVector;
    float mDiameter;

};
