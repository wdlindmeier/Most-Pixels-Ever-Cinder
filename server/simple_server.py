#!/usr/bin/env python

#
#  simple_server.py
#  A simple Most-Pixels-Ever compatable server.
#  Conforms to the MPE 2.0 protocol.
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

portnum = int(args.port_num)
num_clients_required = int(args.num_clients)
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
        # There may be more than 1 message in the mix
        messages = data.split("\n")
        for message in messages:
            if len(message) < 1:
                return

            tokens = message.split("|")
            token_count = len(tokens)
            cmd = tokens[0]

            if cmd == CMD_DID_DRAW:
                # Format
                # D|client_id|last_frame_rendered
                if token_count != 3:
                    print "ERROR: Incorrect param count for CMD %s. " % cmd, data, tokens
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
                if token_count < 3 or token_count > 4:
                    print "ERROR: Incorrect param count for CMD %s. " % cmd, data, tokens
                self.client_id = int(tokens[1])
                self.client_name = tokens[2]
                MPEServer.clients[self.client_id] = self

                client_receives_messages = True
                if cmd == CMD_SYNC_CLIENT_CONNECT:
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
                if token_count < 2 or token_count > 3:
                    print "ERROR: Incorrect param count for CMD %s. " % cmd, data, tokens
                to_client_ids = []
                if token_count == 2:
                    to_client_ids = [c.client_id for c in MPEServer.receiving_clients]            
                elif token_count == 3:
                    to_client_ids = tokens[2].split(",")
                    to_client_ids = [int(client_id) for client_id in to_client_ids]
                
                MPEServer.broadcastMessage(tokens[1], self.client_id, to_client_ids)

            elif cmd == CMD_PAUSE:    
                # Format:
                # P
                if token_count > 1:
                    print "ERROR: Incorrect param count for CMD %s. " % cmd, data, tokens                                    
                MPEServer.togglePause()
            
            elif cmd == CMD_RESET:
                # Format:
                # R
                if token_count > 1:
                    print "ERROR: Incorrect param count for CMD %s. " % cmd, data, tokens
                MPEServer.reset()
            
            else:
                print "Unknown message: " + message

        # print "Received message: ", data, "FROM", self.client_id

    def sendMessage(self, message):
        self.transport.write(message + "\n")

    @staticmethod
    def reset():
        global framecount        
        global is_paused
        framecount = 0
        num_clients_drawn = 0
        MPEServer.message_queue = []
        MPEServer.sendReset()
        if is_paused:
            print "INFO: Reset was called when server is paused."
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
        global num_clients_required
        print "Added client %i (%s)" % (client_id, MPEServer.clients[client_id].client_name)
        num_sync_clients = len(MPEServer.rendering_clients)
        if num_clients_required == -1 or num_sync_clients == num_clients_required:
            MPEServer.reset()
        elif num_sync_clients < num_clients_required:
            print "Waiting for %i more clients." % (num_clients_required - num_sync_clients)
        elif num_sync_clients > num_clients_required:
            print "ERROR: More than MAX clients have connected."
            
    @staticmethod
    def isNextFrameReady():
        global num_clients_drawn
        global num_clients_required
        global is_paused
        num_sync_clients = len(MPEServer.rendering_clients)
        return num_clients_drawn >= num_sync_clients and not is_paused and num_sync_clients >= num_clients_required
        
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
        # print "Broadcasting message: " + message + " to client IDs: ", to_client_ids
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