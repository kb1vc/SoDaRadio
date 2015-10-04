#!/bin/python

import socket
import sys
import time

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
except socket.error:
    print 'Failed to create socket'
    sys.exit()

host = '192.168.10.2'
port = 49172

#sock.setblocking(0)
msg = ("ON00NO\r", "ST00TS\r")
sel = 0
while(1):

    try :
        print 'About to send [' + msg[sel] + ']'
        sock.sendto(msg[sel], (host, port))
        print 'Sent'

        time.sleep(1)

        #time.sleep(5)
        #(reply, rport) = sock.recvfrom(1024)
        #print 'Got reply : ' + reply
        #print 'On port ' + str(rport)
        #(reply, rport) = sock.recvfrom(1024)
    except socket.error, erm:
        print 'Error Code : ' + str(erm[0]) + ' Message ' + erm[1]
        sys.exit()
        
    if sel == 0:
        sel = 1
    else:
        sel = 0
        
