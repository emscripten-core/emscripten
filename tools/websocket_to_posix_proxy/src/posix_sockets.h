#pragma once

#include <stdio.h>

#if defined(__APPLE__) || defined(__linux__)

#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/tcp.h>

#define SOCKET_T int
#define SHUTDOWN_READ SHUT_RD
#define SHUTDOWN_WRITE SHUT_WR
#define SHUTDOWN_BIDIRECTIONAL SHUT_RDWR
#define SETSOCKOPT_PTR_TYPE const int*
#define SEND_RET_TYPE ssize_t
#define SEND_FORMATTING_SPECIFIER "%ld"
#define CLOSE_SOCKET(x) close(x)

#define CREATE_SOCKET_CALL_RETURNS_AN_ERROR(x) ((x) < 0)
#define SOCKET_API_CALL_RETURNS_AN_ERROR(x) ((x) < 0)
#define GET_SOCKET_ERROR() (errno)

#define PRINT_SOCKET_ERROR(errorCode) do { \
  printf("Call failed! errno: %s(%d)\n", strerror(errorCode), errorCode); \
  } while(0)

#elif defined(_MSC_VER)

#include <winsock2.h>
#include <ws2tcpip.h>

#define SOCKET_T SOCKET
#define SHUTDOWN_READ SD_RECEIVE
#define SHUTDOWN_WRITE SD_SEND
#define SHUTDOWN_BIDIRECTIONAL SD_BOTH
#define SETSOCKOPT_PTR_TYPE const char*
#define SEND_RET_TYPE int
#define SEND_FORMATTING_SPECIFIER "%d"
#define CLOSE_SOCKET(x) closesocket(x)

// Winsock documentation states that the "BSD style of error checking "will not work on Windows":
// https://docs.microsoft.com/en-us/windows/win32/winsock/return-values-on-function-failure-2
// but instead must check against SOCKET_ERROR or INVALID_SOCKET

// On Windows, API calls that return a socket should test against INVALID_SOCKET:
// https://docs.microsoft.com/en-us/windows/win32/winsock/handling-winsock-errors
#define CREATE_SOCKET_CALL_RETURNS_AN_ERROR(x) ((x) == INVALID_SOCKET)
#define SOCKET_API_CALL_RETURNS_AN_ERROR(x) ((x) == SOCKET_ERROR)

#define GET_SOCKET_ERROR() (WSAGetLastError())

static inline void PRINT_SOCKET_ERROR(int errorCode)
{
	void *lpMsgBuf = 0;
	HRESULT hresult = HRESULT_FROM_WIN32(errorCode);
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		0, hresult, 0 /*Default language*/, (LPTSTR)&lpMsgBuf, 0, 0);
	printf("Call failed! WSAGetLastError: %s(%d)\n", (char*)lpMsgBuf, errorCode);
	LocalFree(lpMsgBuf);
}

#endif
