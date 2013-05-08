/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @file
 * @ingroup Port
 * @brief Port Library Error Codes
 *
 * When an error is reported by the operating system the port library must translate this OS specific error code to a 
 * portable error code.  All portable error codes are negative numbers.  Not every module of the port library will have
 * error codes dedicated to it's own use, some will use the generic portable error code values.
 *
 * Errors reported by the OS may be recorded by calling the port library functions @ref hyerror.c::hyerror_set_last_error "hyerror_set_last_error()"
 * or @ref hyerror.c::hyerror_set_last_error_with_message "hyerror_set_last_error_with_message()".  The mapping of the OS specific error
 * code to a portable error code is the responsibility of the calling module.
 */
#if !defined(hyporterror_h)
#define hyporterror_h

/**
 * @name Generic Errors
 * Generic error codes for modules that do not have their own specific errors.  Where possible it is recommended that all
 * modules that return error codes have their own set
 *
 * @internal HYPORT_ERROR* range from -1 to -99 avoid overlap
 * @{
 */
#define HYPORT_ERROR_BASE -1
#define HYPORT_ERROR_OPFAILED HYPORT_ERROR_BASE
#define HYPORT_ERROR_EOF HYPORT_ERROR_BASE-1
#define HYPORT_ERROR_NOPERMISSION HYPORT_ERROR_BASE-2
#define HYPORT_ERROR_NOTFOUND HYPORT_ERROR_BASE-3
#define HYPORT_ERROR_NAMETOOLONG HYPORT_ERROR_BASE-4
#define HYPORT_ERROR_DISKFULL HYPORT_ERROR_BASE-5
#define HYPORT_ERROR_EXIST HYPORT_ERROR_BASE-6
#define HYPORT_ERROR_NOTEXIST HYPORT_ERROR_BASE-7
#define HYPORT_ERROR_SYSTEMFULL HYPORT_ERROR_BASE-8
#define HYPORT_ERROR_NOENT HYPORT_ERROR_BASE-9
#define HYPORT_ERROR_DIRECTORY HYPORT_ERROR_BASE-10
#define HYPORT_ERROR_NOTDIRECTORY HYPORT_ERROR_BASE-11
#define HYPORT_ERROR_LOOP HYPORT_ERROR_BASE-12
#define HYPORT_ERROR_BADF HYPORT_ERROR_BASE-13
#define HYPORT_ERROR_WOULDBLOCK HYPORT_ERROR_BASE-14
#define HYPORT_ERROR_INVALID HYPORT_ERROR_BASE-15
/** @} */
/** 
 * @name Port Library startup failure code
 * Failures related to the initialization and startup of the port library.
 *
 * @internal HYPORT_ERROR_STARTUP* range from -100 to -149 to avoid overlap
 * @{
 */
#define HYPORT_ERROR_STARTUP_BASE -100
#define HYPORT_ERROR_STARTUP_THREAD (HYPORT_ERROR_STARTUP_BASE)
#define HYPORT_ERROR_STARTUP_MEM (HYPORT_ERROR_STARTUP_BASE -1)
#define HYPORT_ERROR_STARTUP_TLS (HYPORT_ERROR_STARTUP_BASE -2)
#define HYPORT_ERROR_STARTUP_TLS_ALLOC (HYPORT_ERROR_STARTUP_BASE -3)
#define HYPORT_ERROR_STARTUP_TLS_MUTEX (HYPORT_ERROR_STARTUP_BASE -4)
#define HYPORT_ERROR_STARTUP_CPU (HYPORT_ERROR_STARTUP_BASE -5)
#define HYPORT_ERROR_STARTUP_VMEM (HYPORT_ERROR_STARTUP_BASE -6)
#define HYPORT_ERROR_STARTUP_FILE (HYPORT_ERROR_STARTUP_BASE -7)
#define HYPORT_ERROR_STARTUP_TTY (HYPORT_ERROR_STARTUP_BASE -8)
#define HYPORT_ERROR_STARTUP_TTY_HANDLE (HYPORT_ERROR_STARTUP_BASE -9)
#define HYPORT_ERROR_STARTUP_TTY_CONSOLE (HYPORT_ERROR_STARTUP_BASE -10)
#define HYPORT_ERROR_STARTUP_MMAP (HYPORT_ERROR_STARTUP_BASE -11)
#define HYPORT_ERROR_STARTUP_IPCMUTEX (HYPORT_ERROR_STARTUP_BASE -12)
#define HYPORT_ERROR_STARTUP_NLS (HYPORT_ERROR_STARTUP_BASE -13)
#define HYPORT_ERROR_STARTUP_SOCK (HYPORT_ERROR_STARTUP_BASE -14)
#define HYPORT_ERROR_STARTUP_TIME (HYPORT_ERROR_STARTUP_BASE -15)
#define HYPORT_ERROR_STARTUP_GP (HYPORT_ERROR_STARTUP_BASE -16)
#define HYPORT_ERROR_STARTUP_EXIT (HYPORT_ERROR_STARTUP_BASE -17)
#define HYPORT_ERROR_STARTUP_SYSINFO (HYPORT_ERROR_STARTUP_BASE -18)
#define HYPORT_ERROR_STARTUP_SL (HYPORT_ERROR_STARTUP_BASE -19)
#define HYPORT_ERROR_STARTUP_STR (HYPORT_ERROR_STARTUP_BASE -20)
#define HYPORT_ERROR_STARTUP_SHSEM (HYPORT_ERROR_STARTUP_BASE -21)
#define HYPORT_ERROR_STARTUP_SHMEM (HYPORT_ERROR_STARTUP_BASE -22)
#define HYPORT_ERROR_STARTUP_ERROR (HYPORT_ERROR_STARTUP_BASE -23)
#define HYPORT_ERROR_STARTUP_SIGNAL (HYPORT_ERROR_STARTUP_BASE -24)
/** @} */
/**
 * @name Shared Semaphore Errors
 * Error codes for shared semaphore operations.
 *
 * @internal HYPORT_ERROR_SHSEM* range from at -150 to 159 to avoid overlap 
 * @{
 */
#define HYPORT_ERROR_SHSEM_BASE -150
#define HYPORT_ERROR_SHSEM_OPFAILED (HYPORT_ERROR_SHSEM_BASE)
#define HYPORT_ERROR_SHSEM_HANDLE_INVALID (HYPORT_ERROR_SHSEM_BASE-1)
#define HYPORT_ERROR_SHSEM_SEMSET_INVALID (HYPORT_ERROR_SHSEM_BASE-2)
#define HYPORT_ERROR_SHSEM_NOT_EXIST (HYPORT_ERROR_SHSEM_BASE-3)
#define HYPORT_ERROR_SHSEM_NOPERMISSION (HYPORT_ERROR_SHSEM_BASE-4)
#define HYPORT_ERROR_SHSEM_INVALID_INPUT	(HYPORT_ERROR_SHSEM_BASE-5)
#define HYPORT_ERROR_SHSEM_NOSPACE (HYPORT_ERROR_SHSEM_BASE-6)
#define HYPORT_ERROR_SHSEM_ALREADY_EXIST (HYPORT_ERROR_SHSEM_BASE-7)
#define HYPORT_ERROR_SHSEM_DATA_DIRECTORY_FAILED (HYPORT_ERROR_SHSEM_BASE-8)
/** @} */
/**
 * @name Shared Memory Errors
 * Error codes for shared memory semaphore operations.
 *
 * @internal HYPORT_ERROR_SHMEM* range from at -160 to -169 to avoid overlap 
 * @{
 */
#define HYPORT_ERROR_SHMEM_BASE -160
#define HYPORT_ERROR_SHMEM_OPFAILED (HYPORT_ERROR_SHMEM_BASE)
#define HYPORT_ERROR_SHMEM_HANDLE_INVALID (HYPORT_ERROR_SHMEM_BASE-1)
#define HYPORT_ERROR_SHMEM_NOT_EXIST (HYPORT_ERROR_SHMEM_BASE-2)
#define HYPORT_ERROR_SHMEM_NOPERMISSION (HYPORT_ERROR_SHMEM_BASE-3)
#define HYPORT_ERROR_SHMEM_INVALID_INPUT	(HYPORT_ERROR_SHMEM_BASE-4)
#define HYPORT_ERROR_SHMEM_NOSPACE (HYPORT_ERROR_SHMEM_BASE-5)
#define HYPORT_ERROR_SHMEM_ALREADY_EXIST (HYPORT_ERROR_SHMEM_BASE-6)
#define HYPORT_ERROR_SHMEM_TOOBIG (HYPORT_ERROR_SHMEM_BASE-7)
#define HYPORT_ERROR_SHMEM_ATTACH_FAILED (HYPORT_ERROR_SHMEM_BASE-8)
#define HYPORT_ERROR_SHMEM_DATA_DIRECTORY_FAILED (HYPORT_ERROR_SHMEM_BASE-9)
/** @} */
/* -170 to -199 available for use */
/**
 * @name Socket Errors
 * Error codes for socket operations
 *
 * @internal HYPORT_ERROR_SOCKET* range from -200 to -299 avoid overlap
 * @{
 */
#define HYPORT_ERROR_SOCKET_BASE -200
#define HYPORT_ERROR_SOCKET_BADSOCKET HYPORT_ERROR_SOCKET_BASE						/* generic error */
#define HYPORT_ERROR_SOCKET_NOTINITIALIZED HYPORT_ERROR_SOCKET_BASE-1					/* socket library uninitialized */
#define HYPORT_ERROR_SOCKET_BADAF HYPORT_ERROR_SOCKET_BASE-2								/* bad address family */
#define HYPORT_ERROR_SOCKET_BADPROTO HYPORT_ERROR_SOCKET_BASE-3						/* bad protocol */
#define HYPORT_ERROR_SOCKET_BADTYPE HYPORT_ERROR_SOCKET_BASE-4							/* bad type */
#define HYPORT_ERROR_SOCKET_SYSTEMBUSY HYPORT_ERROR_SOCKET_BASE-5					/* system busy handling requests */
#define HYPORT_ERROR_SOCKET_SYSTEMFULL HYPORT_ERROR_SOCKET_BASE-6					/* too many sockets */
#define HYPORT_ERROR_SOCKET_NOTCONNECTED HYPORT_ERROR_SOCKET_BASE-7				/* socket is not connected */
#define HYPORT_ERROR_SOCKET_INTERRUPTED	HYPORT_ERROR_SOCKET_BASE-8					/* the call was cancelled */
#define HYPORT_ERROR_SOCKET_TIMEOUT	HYPORT_ERROR_SOCKET_BASE-9							/* the operation timed out */
#define HYPORT_ERROR_SOCKET_CONNRESET HYPORT_ERROR_SOCKET_BASE-10					/* the connection was reset */
#define HYPORT_ERROR_SOCKET_WOULDBLOCK	 HYPORT_ERROR_SOCKET_BASE-11			/* the socket is marked as nonblocking operation would block */
#define HYPORT_ERROR_SOCKET_ADDRNOTAVAIL HYPORT_ERROR_SOCKET_BASE-12				/* address not available */
#define HYPORT_ERROR_SOCKET_ADDRINUSE HYPORT_ERROR_SOCKET_BASE-13					/* address already in use */
#define HYPORT_ERROR_SOCKET_NOTBOUND HYPORT_ERROR_SOCKET_BASE-14						/* the socket is not bound */
#define HYPORT_ERROR_SOCKET_UNKNOWNSOCKET HYPORT_ERROR_SOCKET_BASE-15		/* resolution of fileDescriptor to socket failed */
#define HYPORT_ERROR_SOCKET_INVALIDTIMEOUT HYPORT_ERROR_SOCKET_BASE-16			/* the specified timeout is invalid */
#define HYPORT_ERROR_SOCKET_FDSETFULL HYPORT_ERROR_SOCKET_BASE-17					/* Unable to create an FDSET */
#define HYPORT_ERROR_SOCKET_TIMEVALFULL HYPORT_ERROR_SOCKET_BASE-18					/* Unable to create a TIMEVAL */
#define HYPORT_ERROR_SOCKET_REMSOCKSHUTDOWN HYPORT_ERROR_SOCKET_BASE-19	/* The remote socket has shutdown gracefully */
#define HYPORT_ERROR_SOCKET_NOTLISTENING HYPORT_ERROR_SOCKET_BASE-20				/* listen() was not invoked prior to accept() */
#define HYPORT_ERROR_SOCKET_NOTSTREAMSOCK HYPORT_ERROR_SOCKET_BASE-21			/* The socket does not support connection-oriented service */
#define HYPORT_ERROR_SOCKET_ALREADYBOUND HYPORT_ERROR_SOCKET_BASE-22			/* The socket is already bound to an address */
#define HYPORT_ERROR_SOCKET_NBWITHLINGER HYPORT_ERROR_SOCKET_BASE-23				/* The socket is marked non-blocking & SO_LINGER is non-zero */
#define HYPORT_ERROR_SOCKET_ISCONNECTED HYPORT_ERROR_SOCKET_BASE-24				/* The socket is already connected */
#define HYPORT_ERROR_SOCKET_NOBUFFERS HYPORT_ERROR_SOCKET_BASE-25					/* No buffer space is available */
#define HYPORT_ERROR_SOCKET_HOSTNOTFOUND HYPORT_ERROR_SOCKET_BASE-26			/* Authoritative Answer Host not found */
#define HYPORT_ERROR_SOCKET_NODATA HYPORT_ERROR_SOCKET_BASE-27							/* Valid name, no data record of requested type */
#define HYPORT_ERROR_SOCKET_BOUNDORCONN HYPORT_ERROR_SOCKET_BASE-28				/* The socket has not been bound or is already connected */
#define HYPORT_ERROR_SOCKET_OPNOTSUPP HYPORT_ERROR_SOCKET_BASE-29					/* The socket does not support the operation */
#define HYPORT_ERROR_SOCKET_OPTUNSUPP HYPORT_ERROR_SOCKET_BASE-30					/* The socket option is not supported */
#define HYPORT_ERROR_SOCKET_OPTARGSINVALID HYPORT_ERROR_SOCKET_BASE-31			/* The socket option arguments are invalid */
#define HYPORT_ERROR_SOCKET_SOCKLEVELINVALID HYPORT_ERROR_SOCKET_BASE-32		/* The socket level is invalid */
#define HYPORT_ERROR_SOCKET_TIMEOUTFAILURE HYPORT_ERROR_SOCKET_BASE-33			
#define HYPORT_ERROR_SOCKET_SOCKADDRALLOCFAIL HYPORT_ERROR_SOCKET_BASE-34	/* Unable to allocate the sockaddr structure */
#define HYPORT_ERROR_SOCKET_FDSET_SIZEBAD HYPORT_ERROR_SOCKET_BASE-35				/* The calculated maximum size of the file descriptor set is bad */
#define HYPORT_ERROR_SOCKET_UNKNOWNFLAG HYPORT_ERROR_SOCKET_BASE-36				/* The flag is unknown */
#define HYPORT_ERROR_SOCKET_MSGSIZE HYPORT_ERROR_SOCKET_BASE-37						/* The datagram was too big to fit the specified buffer & was truncated. */
#define HYPORT_ERROR_SOCKET_NORECOVERY HYPORT_ERROR_SOCKET_BASE-38				/* The operation failed with no recovery possible */
#define HYPORT_ERROR_SOCKET_ARGSINVALID HYPORT_ERROR_SOCKET_BASE-39					/* The arguments are invalid */
#define HYPORT_ERROR_SOCKET_BADDESC HYPORT_ERROR_SOCKET_BASE-40						/* The socket argument is not a valid file descriptor */
#define HYPORT_ERROR_SOCKET_NOTSOCK HYPORT_ERROR_SOCKET_BASE-41						/* The socket argument is not a socket */
#define HYPORT_ERROR_SOCKET_HOSTENTALLOCFAIL HYPORT_ERROR_SOCKET_BASE-42		/* Unable to allocate the hostent structure */
#define HYPORT_ERROR_SOCKET_TIMEVALALLOCFAIL HYPORT_ERROR_SOCKET_BASE-43		/* Unable to allocate the timeval structure */
#define HYPORT_ERROR_SOCKET_LINGERALLOCFAIL HYPORT_ERROR_SOCKET_BASE-44			/* Unable to allocate the linger structure */
#define HYPORT_ERROR_SOCKET_IPMREQALLOCFAIL HYPORT_ERROR_SOCKET_BASE-45		 	/* Unable to allocate the ipmreq structure */
#define HYPORT_ERROR_SOCKET_FDSETALLOCFAIL HYPORT_ERROR_SOCKET_BASE-46 			/* Unable to allocate the fdset structure */
#define HYPORT_ERROR_SOCKET_OPFAILED HYPORT_ERROR_SOCKET_BASE-47
#define HYPORT_ERROR_SOCKET_VALUE_NULL HYPORT_ERROR_SOCKET_BASE-48 					/* The value indexed was NULL */
#define HYPORT_ERROR_SOCKET_CONNECTION_REFUSED	 HYPORT_ERROR_SOCKET_BASE-49	/* connection was refused */
#define HYPORT_ERROR_SOCKET_ENETUNREACH HYPORT_ERROR_SOCKET_BASE-50					/* network is not reachable */
#define HYPORT_ERROR_SOCKET_EACCES HYPORT_ERROR_SOCKET_BASE-51							/* permissions do not allow action on socket */
#define HYPORT_ERROR_SOCKET_WAS_CLOSED (HYPORT_ERROR_SOCKET_BASE-52)
#define HYPORT_ERROR_SOCKET_EINPROGRESS (HYPORT_ERROR_SOCKET_BASE-53)
#define HYPORT_ERROR_SOCKET_ALREADYINPROGRESS (HYPORT_ERROR_SOCKET_BASE-54)

#define HYPORT_ERROR_SOCKET_FIRST_ERROR_NUMBER HYPORT_ERROR_SOCKET_BASE
#define HYPORT_ERROR_SOCKET_LAST_ERROR_NUMBER HYPORT_ERROR_SOCKET_VALUE_NULL /* Equals last used error code */
/**
 * @name File Errors 
 * Error codes for file operations.
 *
 * @internal HYPORT_ERROR_FILE* range from -300 to -349 avoid overlap
 * @{
 */
#define HYPORT_ERROR_FILE_BASE -300
#define HYPORT_ERROR_FILE_OPFAILED (HYPORT_ERROR_FILE_BASE)
#define HYPORT_ERROR_FILE_EOF (HYPORT_ERROR_FILE_BASE-1)
#define HYPORT_ERROR_FILE_NOPERMISSION (HYPORT_ERROR_FILE_BASE-2)
#define HYPORT_ERROR_FILE_NOTFOUND (HYPORT_ERROR_FILE_BASE-3)
#define HYPORT_ERROR_FILE_NAMETOOLONG (HYPORT_ERROR_FILE_BASE-4)
#define HYPORT_ERROR_FILE_DISKFULL (HYPORT_ERROR_FILE_BASE-5)
#define HYPORT_ERROR_FILE_EXIST (HYPORT_ERROR_FILE_BASE-6)
#define HYPORT_ERROR_FILE_SYSTEMFULL (HYPORT_ERROR_FILE_BASE-7)
#define HYPORT_ERROR_FILE_NOENT (HYPORT_ERROR_FILE_BASE-8)
#define HYPORT_ERROR_FILE_NOTDIR (HYPORT_ERROR_FILE_BASE-9)
#define HYPORT_ERROR_FILE_LOOP (HYPORT_ERROR_FILE_BASE-10)
#define HYPORT_ERROR_FILE_BADF (HYPORT_ERROR_FILE_BASE-11)
#define HYPORT_ERROR_FILE_WOULDBLOCK (HYPORT_ERROR_FILE_BASE-12)
#define HYPORT_ERROR_FILE_INVAL (HYPORT_ERROR_FILE_BASE-13)
#define HYPORT_ERROR_FILE_LOCKED (HYPORT_ERROR_FILE_BASE-14)
/** @} */

#endif     /* hyporterror_h */
