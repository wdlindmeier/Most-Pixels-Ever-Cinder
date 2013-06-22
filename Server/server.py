#!/usr/bin/env python

from twisted.internet.protocol import Factory, Protocol
from twisted.internet import reactor
from math import *
import sys

# TODO: Replace with argparse
num_clients = 2
if len(sys.argv) > 1:
    num_clients = int(sys.argv[1])

# Commands
CMD_DID_DRAW = "D"
CMD_CLIENT_CONNECT = "S"
CMD_BROADCAST = "T"

framecount = 0
num_clients_drawn = 0    
 
class MPEServer(Protocol):
    
    client_id = -1;
    
    def connectionMade(self):
        self.factory.clients.append(self)
        print "Added client:", self.factory.clients
 
    def connectionLost(self, reason):
        self.factory.clients.remove(self)
 
    def dataReceived(self, data):        
        global num_clients_drawn
        cmd = data[:1]
        payload = data[1:]
        if cmd == CMD_DID_DRAW:
            num_clients_drawn += 1
            if num_clients_drawn >= len(self.factory.clients):
                # all of the frames are drawn, send out the next frames
                self.sendNextFrame();
                
        elif cmd == CMD_CLIENT_CONNECT:
            self.handleClientAdd(payload)
            
        elif cmd == CMD_BROADCAST:
            self.broadcastMessage(payload)
            
        else:
            print "Unknown message: " + payload
            
        # print "Received message: ", data, "FROM", self.client_id;        
            
    def handleClientAdd(self, client_id):
        global num_clients
        global framecount
        # NOTE: We're not paying attention to the client ID at the moment
        framecount = 0
        print "Adding client " + client_id;
        self.client_id = client_id;
        if len(self.factory.clients) < num_clients:
            print "Waiting for more clients"
        elif len(self.factory.clients) == num_clients:
            print "All clients are connected. Starting draw loop"
            self.sendNextFrame();
        elif len(self.factory.clients) > num_clients:
            print "ERROR: More than MAX clients have connected"                    
                
    def sendNextFrame(self):
        global num_clients_drawn
        global framecount
        num_clients_drawn = 0
        self.broadcastMessage("G,%i" % framecount)
        framecount += 1
    
    def sendMessage(self, message):
        self.transport.write(message + "\n")
          
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