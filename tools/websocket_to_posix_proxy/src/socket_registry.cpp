#include "socket_registry.h"

#include <map>
#include <vector>
#include <algorithm>

namespace
{
	std::map<int, std::vector<SOCKET_T> > socketsPerProxyConnection;
}

void TrackSocketUsedByConnection(int proxyConnection, SOCKET_T usedSocket)
{
	if (usedSocket == 0) return;
	if (IsSocketPartOfConnection(proxyConnection, usedSocket))
		return;
	socketsPerProxyConnection[proxyConnection].push_back(usedSocket);
}

void CloseSocketByConnection(int proxyConnection, SOCKET_T usedSocket)
{
	if (!IsSocketPartOfConnection(proxyConnection, usedSocket))
		return;
	printf("Closing socket fd %d used by proxy connection %d\n", (int)usedSocket, proxyConnection);
	CLOSE_SOCKET(usedSocket);
	std::vector<SOCKET_T> &sockets = socketsPerProxyConnection[proxyConnection];
	sockets.erase(std::remove(sockets.begin(), sockets.end(), usedSocket), sockets.end());
}

void CloseAllSocketsByConnection(int proxyConnection)
{
	std::vector<SOCKET_T> &sockets = socketsPerProxyConnection[proxyConnection];
	for(size_t i = 0; i < sockets.size(); ++i)
	{
		printf("Closing socket fd %d used by proxy connection %d.\n", (int)sockets[i], proxyConnection);
		shutdown(sockets[i], SHUTDOWN_BIDIRECTIONAL);
		CLOSE_SOCKET(sockets[i]);
	}
	socketsPerProxyConnection.erase(proxyConnection);
}

bool IsSocketPartOfConnection(int proxyConnection, SOCKET_T usedSocket)
{
	if (usedSocket == 0) return true; // Allow all proxy connections to access "socket 0" when/if they need to refer to socket that does not exist.
	if (socketsPerProxyConnection.find(proxyConnection) == socketsPerProxyConnection.end())
		return false;

	std::vector<SOCKET_T> &sockets = socketsPerProxyConnection[proxyConnection];
	return std::find(sockets.begin(), sockets.end(), usedSocket) != sockets.end();
}
