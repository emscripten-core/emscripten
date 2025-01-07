#pragma once

#include "posix_sockets.h"

#ifdef __cplusplus
extern "C" {
#endif

// Socket Registry remembers all the sockets created by incoming proxy connections, so that those sockets can be properly
// shut down when an incoming proxy connection disconnects.
void InitWebSocketRegistry();

// Tracks that the given socket is part of the specified proxy connection. When proxyConnection disconnects, all sockets
// used by it are shut down.
void TrackSocketUsedByConnection(int proxyConnection, SOCKET_T usedSocket);

// Untracks the given socket - the proxy connection has shut it down.
void CloseSocketByConnection(int proxyConnection, SOCKET_T usedSocket);

// Given proxy connection has disconnected - shut down all the sockets it had created.
void CloseAllSocketsByConnection(int proxyConnection);

// Returns if the given socket is known to be owned by the specified proxy connection.
// This is used to gate socket connections so that two proxy connections can not access
// each others' sockets.
bool IsSocketPartOfConnection(int proxyConnection, SOCKET_T usedSocket);

#ifdef __cplusplus
}
#endif
