#!/usr/bin/env python

#
#  simple_server.py
#  A simple Most-Pixels-Ever compatable server.
#  Used for testing purposes. This may not have 100%
#  feature parody with the Java server.
#
#  Created by William Lindmeier.
#  https://github.com/wdlindmeier/Most-Pixels-Ever-Cinder
#

from twisted.internet.protocol import Factory, Protocol
from twisted.internet import reactor
from math import *
import sys
import argparse

# Commands
CMD_DID_DRAW = "D"
CMD_SYNC_CLIENT_CONNECT = "S"
CMD_ASYNC_CLIENT_CONNECT = "A"
CMD_BROADCAST = "T"
CMD_PAUSE = "P"
CMD_RESET = "R"
CMD_GO = "G"

# Parse the command line arguments
parser = argparse.ArgumentParser(description='Most Pixels Ever Server, conforms to protocol version 2.0')
parser.add_argument('--num-clients', dest='num_clients', default=-1, help='The number of clients. The server won\'t start the draw loop until all of the clients are connected.')
parser.add_argument('--port', dest='port_num', default=9002, help='The port number that the clients connect to.')
args = parser.parse_args()

portnum = args.port_num
num_clients_required = args.num_clients
framecount = 0
num_clients_drawn = 0
is_paused = False

class BroadcastMessage:
    
    def __init__(self, body, from_client_id, to_client_ids = []):
        self.body = body
        self.from_client_id = from_client_id
        self.to_client_ids = to_client_ids
        
class MPEServer(Protocol):

    client_id = -1
    client_name = ""
    
    def connectionMade(self):
        print "Client connected. Total Clients: %i" % (len(MPEServer.clients) + 1)

    def connectionLost(self, reason):
        print "Client disconnected"
        del MPEServer.clients[self.client_id]
        if self in MPEServer.rendering_clients:
            MPEServer.rendering_clients.remove(self)

    def dataReceived(self, data):
        global num_clients_drawn
        global framecount
        tokens = data[:-1].split("|")
        cmd = tokens[0]

        if cmd == CMD_DID_DRAW:
            # Format
            # D|client_id|last_frame_rendered
            client = int(tokens[1])
            frame_id = int(tokens[2])
            if frame_id >= framecount:
                num_clients_drawn += 1
                if MPEServer.isNextFrameReady():
                    # all of the frames are drawn, send out the next frames
                    MPEServer.sendNextFrame()

        elif (cmd == CMD_SYNC_CLIENT_CONNECT) or (cmd == CMD_ASYNC_CLIENT_CONNECT):
            # Formats 
            # "S|client_id|client_name"
            # "A|client_id|client_name|should_receive_broadcasts"
            self.client_id = int(tokens[1])
            self.client_name = tokens[2]
            MPEServer.clients[self.client_id] = self

            client_receives_messages = True
            if cmd == CMD_ASYNC_CLIENT_CONNECT:
                MPEServer.rendering_clients.append(self)
            elif cmd == CMD_ASYNC_CLIENT_CONNECT:
                client_receives_messages = tokens[3].lower() == 'true'
                                
            if client_receives_messages:
                MPEServer.receiving_clients.append(self)         
                
            MPEServer.handleClientAdd(self.client_id)

        elif cmd == CMD_BROADCAST:
            # Formats: 
            # "T|message message message"
            # "T|message message message|toID_1,toID_2,toID_3"
            to_client_ids = []
            if len(tokens) < 3:
                to_client_ids = [c.client_id for c in MPEServer.receiving_clients]  
            else:
                to_client_ids = tokens[2].split(",")
                to_client_ids = [int(client_id) for client_id in to_client_ids]
                
            MPEServer.broadcastMessage(tokens[1], self.client_id, to_client_ids)

        elif cmd == CMD_PAUSE:    
            # Format:
            # P
            MPEServer.togglePause()
            
        elif cmd == CMD_RESET:
            # Format:
            # R
            MPEServer.reset()
            
        else:
            print "Unknown message: " + data

        # print "Received message: ", data, "FROM", self.client_id

    def sendMessage(self, message):
        self.transport.write(message + "\n")

    @staticmethod
    def reset():
        global framecount        
        framecount = 0
        num_clients_drawn = 0
        MPEServer.message_queue = []
        MPEServer.sendReset()
        MPEServer.sendNextFrame()
    
    @staticmethod   
    def sendReset():
        for c in MPEServer.receiving_clients:
            c.sendMessage(CMD_RESET)
        
    @staticmethod
    def togglePause():
        global is_paused
        is_paused = not is_paused
        if MPEServer.isNextFrameReady():
            MPEServer.sendNextFrame()
        
    @staticmethod
    def handleClientAdd(client_id):
        global num_clients
        global framecount
        print "Added client %i (%s)" % (client_id, MPEServer.clients[client_id].client_name)
        if num_clients_required == -1 or len(MPEServer.rendering_clients) == num_clients_required:
            MPEServer.reset()
        elif len(MPEServer.rendering_clients) < num_clients_required:
            print "Waiting for more clients."
        elif len(MPEServer.rendering_clients) > num_clients_required:
            print "ERROR: More than MAX clients have connected."
            
    @staticmethod
    def isNextFrameReady():
        global num_clients_drawn
        global is_paused
        return num_clients_drawn >= len(MPEServer.rendering_clients) and not is_paused
        
    @staticmethod
    def sendNextFrame():
        if is_paused:
            return
        global num_clients_drawn
        global framecount
        num_clients_drawn = 0
        framecount += 1        
        send_message = CMD_GO + "|%i" % framecount
        for client_id in MPEServer.clients:
            c = MPEServer.clients[client_id]
            client_messages = []
            for m in MPEServer.message_queue:
                if len(m.to_client_ids) == 0 or c.client_id in m.to_client_ids:
                    client_messages.append(str(m.from_client_id) + "," + m.body)
            if len(client_messages) > 0:
                c.sendMessage(send_message + "|" + "|".join(client_messages))
            else:
                c.sendMessage(send_message)
                
        MPEServer.message_queue = []
        
    @staticmethod
    def broadcastMessage(message, from_client_id, to_client_ids):
        print "Broadcasting message: " + message
        m = BroadcastMessage(message, from_client_id, to_client_ids)
        MPEServer.message_queue.append(m)
            
# Start the server
factory = Factory()
factory.protocol = MPEServer

MPEServer.clients = {}
MPEServer.rendering_clients = []
MPEServer.receiving_clients = []
MPEServer.message_queue = []

reactor.listenTCP(portnum, factory)
print "MPE Server started on port %i" % portnum
if num_clients_required > 0:
    print "Waiting for %i clients." % num_clients_required
reactor.run()