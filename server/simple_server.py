#!/usr/bin/env python

#
#  simple_server.py
#  A basic Most-Pixels-Ever compatable server.
#  Conforms to the MPE 2.0 protocol.
#
#  Created by William Lindmeier.
#  https://github.com/wdlindmeier/Most-Pixels-Ever-Cinder
#

from twisted.internet.protocol import Factory, Protocol
from twisted.internet import reactor
from math import *
from datetime import *
from copy import copy
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
parser.add_argument('--screens', dest='screens', default=-1, help='The number of clients. The server won\'t start the draw loop until all of the clients are connected.')
parser.add_argument('--port', dest='port_num', default=9002, help='The port number that the clients connect to.')
parser.add_argument('--framerate', dest='framerate', default=60, help='The target framerate.')
args = parser.parse_args()

portnum = int(args.port_num)
screens_required = int(args.screens)
framerate = int(args.framerate)
microseconds_per_frame = (1.0 / framerate) * 1000000
framecount = 0
screens_drawn = 0
is_paused = False
last_frame_time = datetime.now()

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
        if MPEServer.clients[self.client_id]:
            del MPEServer.clients[self.client_id]
        if self.client_id in MPEServer.rendering_client_ids:
            MPEServer.rendering_client_ids.remove(self.client_id)
        if self.client_id in MPEServer.receiving_client_ids:
            MPEServer.receiving_client_ids.remove(self.client_id)
        # It's possible that isNextFrameReady is true after the client disconnects
        # if they were the last client to render and hadn't informed the server.
        if MPEServer.isNextFrameReady():
            MPEServer.sendNextFrame()

    def dataReceived(self, data):
        global screens_drawn
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
                    screens_drawn += 1
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
                    MPEServer.rendering_client_ids.append(self.client_id)
                elif cmd == CMD_ASYNC_CLIENT_CONNECT:
                    client_receives_messages = tokens[3].lower() == 'true'

                if client_receives_messages:
                    print "New client will receive data"
                    MPEServer.receiving_client_ids.append(self.client_id)

                MPEServer.handleClientAdd(self.client_id)

            elif cmd == CMD_BROADCAST:
                # Formats:
                # "T|message message message"
                # "T|message message message|toID_1,toID_2,toID_3"
                if token_count < 2 or token_count > 3:
                    print "ERROR: Incorrect param count for CMD %s. " % cmd, data, tokens
                to_client_ids = []
                if token_count == 2:
                    to_client_ids = MPEServer.receiving_client_ids
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
        screens_drawn = 0
        MPEServer.message_queue = []
        MPEServer.sendReset()
        if is_paused:
            print "INFO: Reset was called when server is paused."
        MPEServer.sendNextFrame()

    @staticmethod
    def sendReset():
        for n in MPEServer.receiving_client_ids:
            MPEServer.clients[n].sendMessage(CMD_RESET)

    @staticmethod
    def togglePause():
        global is_paused
        is_paused = not is_paused
        if MPEServer.isNextFrameReady():
            MPEServer.sendNextFrame()

    @staticmethod
    def handleClientAdd(client_id):
        global framecount
        global screens_required
        print "Added client %i (%s)" % (client_id, MPEServer.clients[client_id].client_name)
        num_sync_clients = len(MPEServer.rendering_client_ids)
        if screens_required == -1 or num_sync_clients == screens_required:
            # NOTE: We don't reset when an async client connects
            if client_id in MPEServer.rendering_client_ids:
                MPEServer.reset()
        elif num_sync_clients < screens_required:
            print "Waiting for %i more clients." % (screens_required - num_sync_clients)
        elif num_sync_clients > screens_required:
            print "ERROR: More than MAX clients have connected."

    @staticmethod
    def isNextFrameReady():
        global screens_drawn
        global screens_required
        global is_paused
        num_sync_clients = len(MPEServer.rendering_client_ids)
        return screens_drawn >= num_sync_clients and not is_paused and num_sync_clients >= screens_required

    @staticmethod
    def sendNextFrame():
        global last_frame_time
        global screens_drawn
        global framecount
        global is_paused
        global framerate
        global microseconds_per_frame

        if is_paused:
            return

        # Slow down if we've exceeded the target FPS
        delta = datetime.now() - last_frame_time
        while delta.seconds < 1 and delta.microseconds < microseconds_per_frame:
            delta = datetime.now() - last_frame_time

        screens_drawn = 0
        framecount += 1
        send_message = CMD_GO + "|%i" % framecount
        # Copy the clients so in case one disconnects during the loop
        clients = copy(MPEServer.clients)
        for client_id in clients:
            c = clients[client_id]
            if client_id in MPEServer.receiving_client_ids:
                client_messages = []
                for m in MPEServer.message_queue:
                    if len(m.to_client_ids) == 0 or client_id in m.to_client_ids:
                        client_messages.append(str(m.from_client_id) + "," + m.body)

                if len(client_messages) > 0:
                    c.sendMessage(send_message + "|" + "|".join(client_messages))
                else:
                    c.sendMessage(send_message)

        MPEServer.message_queue = []
        last_frame_time = datetime.now()

    @staticmethod
    def broadcastMessage(message, from_client_id, to_client_ids):
        #print "Broadcasting message: " + message + " to client IDs: ", to_client_ids
        m = BroadcastMessage(message, from_client_id, to_client_ids)
        MPEServer.message_queue.append(m)

# Start the server
factory = Factory()
factory.protocol = MPEServer

MPEServer.clients = {}
MPEServer.rendering_client_ids = []
MPEServer.receiving_client_ids = []
MPEServer.message_queue = []

reactor.listenTCP(portnum, factory)
print "MPE Server started on port %i" % portnum
print "Running at max %i FPS" % framerate
if screens_required > 0:
    print "Waiting for %i clients." % screens_required
reactor.run()
