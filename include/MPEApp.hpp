//
//  MPEApp.hpp
//  MPEBouncingBall
//
//  Created by William Lindmeier on 7/21/13.
//
//

#pragma once

#include "MPEProtocol2.hpp"

/*
 
 MPEApp:
 Add MPEApp as a superclass to your Cinder App. The MPEClient takes an MPEApp in it's constructor
 and will call the methods below. Your Cinder app should override these methods as appropriate.
 
*/

namespace mpe
{
    class MPEApp
    {
     
    public:
        
        /*
         REQUIRED
         This is the update callback, called whenever the server sends a new frame.
         App state changes should only happen in mpeFrameUpdate, rather than in
         App::update(), so that all of the clients stay in sync.
         */
        virtual void mpeFrameUpdate(long serverFrameNumber) = 0;
        
        /*
         RECOMMENDED
         This is the draw callback. The client will position the viewport before
         calling mpeFrameRender and tells the server that the frame has been rendered
         after the callback.
         */
        virtual void mpeFrameRender(bool isNewFrame)
        {
        }
        
        /*
         OPTIONAL
         This will be called when string data is received from any of the connected
         clients (including yourself). The sender's client ID is also passed in.
         */
        virtual void mpeDataReceived(const std::string & message, const int fromClientID)
        {
        }
        
        /*
         REQUIRED
         Called whenever a new client joins or the server resets the simulation for
         any other reason. All accumulated state and data must be discarded and returned
         to frame 0.
         */
        virtual void mpeReset() = 0;
        
        /*
         The version of the protocol that you're using. Defaults to MPE 2.0.
        */
        virtual std::shared_ptr<MPEProtocol> mpeProtocol()
        {
            return std::shared_ptr<MPEProtocol>(new MPEProtocol2());
        };
        
        /*
         The name of the Settings xml file found in assets/
        */
        virtual std::string mpeSettingsFilename()
        {
            return "settings.xml";
        }
    };
}