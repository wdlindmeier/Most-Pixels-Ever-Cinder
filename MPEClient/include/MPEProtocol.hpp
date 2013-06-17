//
//  MPEProtocol.hpp
//  MPEClient
//
//  Created by William Lindmeier on 6/16/13.
//
//

#include "cinder/Rect.h"

/*
 
 This class specifies the commands that can be sent to the server
 and returns strings that should be sent over TCP.
 
*/

class MPEProtocol
{
    
public:
    
    // NOTE:
    // Making these purely virtual ( e.g. setBlah() = 0; ) is giving me a compile error.
    virtual std::string setLocalViewRect( const ci::Rectf & rect ){ return ""; };
    virtual std::string setMaserDimensions( const ci::Vec2i & rect ){ return ""; };
    virtual std::string setClientID( const int clientID ){ return ""; };
    
};