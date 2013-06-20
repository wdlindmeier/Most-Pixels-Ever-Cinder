#!/usr/bin/env python

from twisted.internet.protocol import Factory, Protocol
from twisted.internet import reactor
from math import *
import sys

#framecount = 0
#num_clients_drawn = 0

# TODO: Replace with argparse
num_clients = 2
if len(sys.argv) > 0:
    num_clients = int(sys.argv[1])

# Commands
CMD_DID_DRAW = "D"
CMD_CLIENT_CONNECT = "S"
CMD_BROADCAST = "T"

# Keep the padding as long as MAX_PACKET_LENGTH
MAX_PACKET_LENGTH = 10 
PACKET_PADDING = "\nXXXXXXXXXX"
 
class MPEServer(Protocol):
    
    framecount = 0
    num_clients_drawn = 0
    
    def connectionMade(self):
        self.factory.clients.append(self)
        print "Added client:", self.factory.clients
 
    def connectionLost(self, reason):
        self.factory.clients.remove(self)
 
    def dataReceived(self, data):        
        #print "Received message: " + data;
        cmd = data[:1]
        payload = data[1:]
        if cmd == CMD_DID_DRAW:
            self.num_clients_drawn += 1
            if self.num_clients_drawn >= len(self.factory.clients):
                # all of the frames are drawn, send out the next frames
                self.sendNextFrame();
                
        elif cmd == CMD_CLIENT_CONNECT:
            self.handleClientAdd(payload)
            
        elif cmd == CMD_BROADCAST:
            self.broadcastMessage(payload)
            
    def handleClientAdd(self, client_id):
        global num_clients
        # NOTE: We're not paying attention to the client ID at the moment
        self.framecount = 0
        if len(self.factory.clients) < num_clients:
            print "Waiting for more clients"
        elif len(self.factory.clients) == num_clients:
            print "All clients are connected. Starting draw loop"
            self.sendNextFrame();
        elif len(self.factory.clients) > num_clients:
            print "ERROR: More than MAX clients have connected"                    
                
    def sendNextFrame(self):
        self.num_clients_drawn = 0
        self.broadcastMessage("G,%i" % self.framecount)
    
    def sendMessage(self, message):
        # Always send out MAX_PACKET_LENGTH chars
        packet = (message + PACKET_PADDING)[:MAX_PACKET_LENGTH]
        self.transport.write(packet)
          
    # TODO: Don't send a message back to the sender      
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