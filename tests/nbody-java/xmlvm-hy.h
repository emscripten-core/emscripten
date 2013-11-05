/*
 * Copyright (c) 2002-2011 by XMLVM.org
 *
 * Project Info:  http://www.xmlvm.org
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#ifndef __XMLVM_HY_H__

#define __XMLVM_HY_H__

#ifdef __APPLE__
#define MACOSX
#endif

#include "xmlvm.h"
#include "hycomp.h"
#include "hysock.h"
#include "hysocket.h"


#define HYERROR_DEFAULT_BUFFER_SIZE 256 /**< default customized error message size if we need to create one */

#if defined(__linux__)
#define DIR_SEPARATOR '/'
#endif


typedef struct PortlibPTBuffers_struct
{
    I_32 platformErrorCode;         /**< error code as reported by the OS */
    I_32 portableErrorCode;         /**< error code translated to portable format by application */
    char *errorMessageBuffer;       /**< last saved error message, either customized or from OS */
    U_32 errorMessageBufferSize;    /**< error message buffer size */
    I_32 reportedErrorCode;         /**< last reported error code */
    char *reportedMessageBuffer;    /**< last reported error message, either customized or from OS */
    U_32 reportedMessageBufferSize; /**< reported message buffer size */   
    hyfdset_t fdset;                                  
                                   
#if HOSTENT_DATA_R||GLIBC_R||OTHER_R||NO_R
    OSHOSTENT hostent;
#endif
    
#if HOSTENT_DATA_R
    OSHOSTENT_DATA *hostent_data;
#elif GLIBC_R || OTHER_R || NO_R
    int gethostBufferSize;
    char *gethostBuffer;
#endif                          /* HOSTENT_DATA_R */
    
} PortlibPTBuffers_struct;

typedef struct PortlibPTBuffers_struct *PortlibPTBuffers_t;

 
typedef pthread_mutex_t MUTEX;

/* MUTEX_INIT */
#define MUTEX_INIT(mutex) (pthread_mutex_init(&(mutex), NULL) == 0)

/* MUTEX_DESTROY */
#define MUTEX_DESTROY(mutex) pthread_mutex_destroy(&(mutex))

/* MUTEX_ENTER */
#define MUTEX_ENTER(mutex) pthread_mutex_lock(&(mutex))

/*
 *  MUTEX_TRY_ENTER 
 *  returns 0 on success
 */
#define MUTEX_TRY_ENTER(mutex) pthread_mutex_trylock(&(mutex))

/* MUTEX_EXIT */
#define MUTEX_EXIT(mutex) pthread_mutex_unlock(&(mutex))


#if defined(SEEK_SET)
#define HySeekSet SEEK_SET        /* Values for HyFileSeek */
#else
#define HySeekSet 0
#endif

#if defined(SEEK_CUR)
#define HySeekCur SEEK_CUR
#else
#define HySeekCur 1
#endif

#if defined(SEEK_END)
#define HySeekEnd SEEK_END
#else
#define HySeekEnd 2
#endif


typedef JAVA_INT jsize;


void* hyport_tls_get ();
void* hyport_tls_peek ();
const char* hyerror_last_error_message ();
I_32 hyerror_last_error_number ();
I_32 hyerror_set_last_error (I_32 platformCode, I_32 portableCode);
I_32 hyerror_set_last_error_with_message (I_32 portableCode, const char *errorMessage);


#endif
