This directory contain alternate implementations of
WebSockets-to-TCP-Socket proxies (for noVNC).

## websockify.c (C)

### Description

This is a C version of the original websockify. It is more limited in
functionality than the original.


## websockify.js

### Description

This is a Node.JS (server-side event driven Javascript) implementation
of websockify.


## kumina.c (C)

### Description

The upstream source of the kumina proxy is [here](https://github.com/kumina/wsproxy).

[This article](http://blog.kumina.nl/2011/06/proxying-and-multiplexing-novnc-using-wsproxy/)
describes the kumina proxy.

kumina is an application that is run from inetd, which allows noVNC
to connect to an unmodified VNC server. Furthermore, it makes use of
the recently added support in noVNC for file names. The file name is
used to denote the port number. Say, you connect to:

  ws://host:41337/25900

The kumina proxy opens a connection to:

  vnc://host:25900/

The address to which kumina connects, is the same as the address to
which the client connected (using getsockname()).

### Configuration

kumina can be enabled by adding the following line to inetd.conf:

  41337 stream tcp nowait nobody /usr/sbin/kumina kumina 25900 25909

The two parameters of kumina denote the minimum and the maximum allowed
port numbers. This allows a single kumina instance to multiplex
connections to multiple VNC servers.
