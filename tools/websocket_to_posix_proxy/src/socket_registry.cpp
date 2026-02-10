#include "socket_registry.h"

#include <map>
#include <vector>
#include <algorithm>
#include "threads.h"

namespace {
  MUTEX_T socketRegistryLock;
  std::map<int, std::vector<SOCKET_T> > socketsPerProxyConnection;
}

void InitWebSocketRegistry() {
  CREATE_MUTEX(&socketRegistryLock);
}

void TrackSocketUsedByConnection(int proxyConnection, SOCKET_T usedSocket) {
  if (usedSocket == 0) return;
  if (IsSocketPartOfConnection(proxyConnection, usedSocket))
    return;

  LOCK_MUTEX(&socketRegistryLock);

  socketsPerProxyConnection[proxyConnection].push_back(usedSocket);

  UNLOCK_MUTEX(&socketRegistryLock);
}

void CloseSocketByConnection(int proxyConnection, SOCKET_T usedSocket) {
  if (!IsSocketPartOfConnection(proxyConnection, usedSocket))
    return;

  printf("Closing socket fd %d used by proxy connection %d\n", (int)usedSocket, proxyConnection);

  LOCK_MUTEX(&socketRegistryLock);

  CLOSE_SOCKET(usedSocket);
  std::vector<SOCKET_T> &sockets = socketsPerProxyConnection[proxyConnection];
  sockets.erase(std::remove(sockets.begin(), sockets.end(), usedSocket), sockets.end());

  UNLOCK_MUTEX(&socketRegistryLock);
}

void CloseAllSocketsByConnection(int proxyConnection) {
  LOCK_MUTEX(&socketRegistryLock);

  std::vector<SOCKET_T> &sockets = socketsPerProxyConnection[proxyConnection];
  for (size_t i = 0; i < sockets.size(); ++i) {
    printf("Closing socket fd %d used by proxy connection %d.\n", (int)sockets[i], proxyConnection);
    shutdown(sockets[i], SHUTDOWN_BIDIRECTIONAL);
    CLOSE_SOCKET(sockets[i]);
  }
  socketsPerProxyConnection.erase(proxyConnection);

  UNLOCK_MUTEX(&socketRegistryLock);
}

bool IsSocketPartOfConnection(int proxyConnection, SOCKET_T usedSocket) {
  bool result;
  if (usedSocket == 0) return true; // Allow all proxy connections to access "socket 0" when/if they need to refer to socket that does not exist.

  LOCK_MUTEX(&socketRegistryLock);

  if (socketsPerProxyConnection.find(proxyConnection) == socketsPerProxyConnection.end()) {
    UNLOCK_MUTEX(&socketRegistryLock);
    return false;
  }

  std::vector<SOCKET_T> &sockets = socketsPerProxyConnection[proxyConnection];
  result = std::find(sockets.begin(), sockets.end(), usedSocket) != sockets.end();

  UNLOCK_MUTEX(&socketRegistryLock);

  return result;
}
