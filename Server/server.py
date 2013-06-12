#!/usr/bin/env python

from twisted.internet.protocol import Factory, Protocol
from twisted.internet import reactor
from math import *
 
class MPEServer(Protocol):
    
    def connectionMade(self):
        self.factory.clients.append(self)
        print "clients are ", self.factory.clients
 
    def connectionLost(self, reason):
        self.factory.clients.remove(self)
 
    def dataReceived(self, data):        
        print "Received message: " + data;
        self.broadcastMessage(("RCVD: " + data + "\nXXXXXXXXXX")[:10]);
    
    def sendMessage(self, message):        
        self.transport.write(message)
          
    def broadcastMessage(self, message):         
        for c in self.factory.clients:
            c.sendMessage(message)  
                    
                 
factory = Factory()
factory.protocol = MPEServer
factory.clients = []
portnum = 7777
reactor.listenTCP(portnum, factory)
print "MPE Server started on port %i" % portnum
reactor.run()