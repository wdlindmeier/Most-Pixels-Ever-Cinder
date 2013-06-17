//
//  MPEProtocolClassic.hpp
//  MPEClient
//
//  Created by William Lindmeier on 6/16/13.
//
//

#pragma once

#include "MPEProtocol.hpp"
#include "cinder/Rect.h"

using namespace std;

/*
 
 An MPE server protocol for Shiffman's Java server, which can be found here:
 https://github.com/shiffman/Most-Pixels-Ever/tree/master/server_jar
 
 To start the server:
 java -jar mpeServer.jar -debug1 -framerate30 -screens2
 
*/

class MPEProtocolClassic : public MPEProtocol {
    
public:
    
    //MPEProtocolClassic(){}
    
    std::string setLocalViewRect( ci::Rectf & rect )
    {
        return "local " +
                std::to_string(rect.getX1()) + "," +
                std::to_string(rect.getY1()) + "," +
                std::to_string(rect.getWidth()) + "," +
                std::to_string(rect.getHeight()) + "\n";
    }

    std::string setMaserDimensions( ci::Vec2i & size )
    {
        return "master " + std::to_string(size.x) + "," + std::to_string(size.y) + "\n";
    }
    
    std::string setClientID( const int clientID )
    {
        return "S" + std::to_string(clientID) + "\n";
    }
    
};