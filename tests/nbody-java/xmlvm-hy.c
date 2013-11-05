
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

#include "xmlvm.h"
#include "xmlvm-hy.h"


const char* errorMessage (I_32 errorCode)
{
    PortlibPTBuffers_t ptBuffers;
    
    ptBuffers = hyport_tls_peek ();
    if (0 == ptBuffers->errorMessageBufferSize)
    {
        ptBuffers->errorMessageBuffer = XMLVM_ATOMIC_MALLOC(HYERROR_DEFAULT_BUFFER_SIZE);
        if (NULL == ptBuffers->errorMessageBuffer)
        {
            return "";
        }
        ptBuffers->errorMessageBufferSize = HYERROR_DEFAULT_BUFFER_SIZE;
    }
    
    /* Copy from OS to ptBuffers */
#if !defined(ZOS)
    strerror_r(errorCode,
               ptBuffers->errorMessageBuffer, ptBuffers->errorMessageBufferSize);
#else
    /* Do not have strerror_r on z/OS so use port library function instead */
    portLibrary->str_printf(portLibrary, ptBuffers->errorMessageBuffer, ptBuffers->errorMessageBufferSize, strerror(errorCode));
#endif /* ZOS */
    ptBuffers->errorMessageBuffer[ptBuffers->errorMessageBufferSize - 1] = '\0';
    return ptBuffers->errorMessageBuffer;
}


static const char* swapMessageBuffer (PortlibPTBuffers_t ptBuffers, const char *message)
{
    char *tempBuffer = ptBuffers->reportedMessageBuffer;
    U_32 tempBufferSize = ptBuffers->reportedMessageBufferSize;
    
    if (message == NULL)
    {
        return "";
    }
    
    /* Can't swap unknown message buffer */
    if (message != ptBuffers->errorMessageBuffer)
    {
        return message;
    }
    
    /* Save reported information */
    ptBuffers->reportedErrorCode = ptBuffers->portableErrorCode;
    ptBuffers->reportedMessageBuffer = ptBuffers->errorMessageBuffer;
    ptBuffers->reportedMessageBufferSize = ptBuffers->errorMessageBufferSize;
    
    if (tempBufferSize > 0)
    {
        tempBuffer[0] = '\0';
    }
    
    /* Clear pending fields ready for next error */
    ptBuffers->portableErrorCode = 0;
    ptBuffers->errorMessageBuffer = tempBuffer;
    ptBuffers->errorMessageBufferSize = tempBufferSize;
    
    return ptBuffers->reportedMessageBuffer;
}


void* hyport_tls_get ()
{
    java_lang_Thread* curThread = (java_lang_Thread*) java_lang_Thread_currentThread__();
    if (curThread->fields.java_lang_Thread.ptBuffers_ == JAVA_NULL) {
        curThread->fields.java_lang_Thread.ptBuffers_ = XMLVM_MALLOC(sizeof(PortlibPTBuffers_struct));
        XMLVM_BZERO(curThread->fields.java_lang_Thread.ptBuffers_, sizeof(PortlibPTBuffers_struct));
    }
    
    return curThread->fields.java_lang_Thread.ptBuffers_;
}


void* hyport_tls_peek ()
{
    java_lang_Thread* curThread = (java_lang_Thread*) java_lang_Thread_currentThread__();
    return curThread->fields.java_lang_Thread.ptBuffers_;
}


const char* hyerror_last_error_message ()
{
    PortlibPTBuffers_t ptBuffers;
    
    /* Was an error saved ? */
    ptBuffers = hyport_tls_peek ();
    if (NULL == ptBuffers)
    {
        return "";
    }
    
    /* New error ? */
    if (ptBuffers->portableErrorCode != 0)
    {
        const char *message = NULL;
        
        /* Customized message stored ? */
        if (ptBuffers->errorMessageBufferSize > 0)
        {
            if ('\0' != ptBuffers->errorMessageBuffer[0])
            {
                message = ptBuffers->errorMessageBuffer;
            }
        }
        
        /* Call a helper to get the last message from the OS.  */
        if (message == NULL)
        {
            message = errorMessage (ptBuffers->platformErrorCode);
        }
        
        /* Avoid overwrite by internal portlib errors */
        return swapMessageBuffer (ptBuffers, message);
    }
    
    /* Previous message stored ? */
    if (ptBuffers->reportedMessageBufferSize > 0)
    {
        if ('\0' != ptBuffers->reportedMessageBuffer[0])
        {
            return ptBuffers->reportedMessageBuffer;
        }
    }
    
    /* No error.  */
    return "";
}


I_32 hyerror_last_error_number ()
{
    PortlibPTBuffers_t ptBuffers;
    
    /* get the buffers, return failure if not present */
    ptBuffers = hyport_tls_peek ();
    if (NULL == ptBuffers)
    {
        return 0;
    }
    
    /* New error ? */
    if (ptBuffers->portableErrorCode != 0)
    {
        return ptBuffers->portableErrorCode;
    }
    else
    {
        return ptBuffers->reportedErrorCode;
    }
}


I_32 hyerror_set_last_error (I_32 platformCode, I_32 portableCode)
{
    PortlibPTBuffers_t ptBuffers;
    
    /* get the buffers, allocate if necessary.
     * Silently return if not present, what else would the caller do anyway? 
     */
    ptBuffers = hyport_tls_get ();
    if (NULL == ptBuffers)
    {
        return portableCode;
    }
    
    /* Save the last error */
    ptBuffers->platformErrorCode = platformCode;
    ptBuffers->portableErrorCode = portableCode;
    
    /* Overwrite any customized messages stored */
    if (ptBuffers->errorMessageBufferSize > 0)
    {
        ptBuffers->errorMessageBuffer[0] = '\0';
    }
    
    return portableCode;
}


I_32 hyerror_set_last_error_with_message (I_32 portableCode, const char *errorMessage)
{
    PortlibPTBuffers_t ptBuffers;
    U_32 requiredSize;
    
    /* get the buffers, allocate if necessary.
     * Silently return if not present, what else would the caller do anyway? 
     */
    ptBuffers = hyport_tls_get ();
    if (NULL == ptBuffers)
    {
        return portableCode;
    }
    
    /* Save the last error */
    ptBuffers->platformErrorCode = -1;
    ptBuffers->portableErrorCode = portableCode;
    
    /* Store the message, allocate a bigger buffer if required.  Keep the old buffer around
     * just in case memory can not be allocated
     */
    requiredSize = strlen (errorMessage) + 1;
    requiredSize =
    requiredSize <
    HYERROR_DEFAULT_BUFFER_SIZE ? HYERROR_DEFAULT_BUFFER_SIZE : requiredSize;
    if (requiredSize > ptBuffers->errorMessageBufferSize)
    {
        char *newBuffer = XMLVM_ATOMIC_MALLOC(requiredSize);
        if (NULL != newBuffer)
        {
            if (ptBuffers->errorMessageBuffer != NULL)
            {
                XMLVM_FREE(ptBuffers->errorMessageBuffer);
            }
            ptBuffers->errorMessageBuffer = newBuffer;
            ptBuffers->errorMessageBufferSize = requiredSize;
        }
    }
    
    /* Save the message */
    if (ptBuffers->errorMessageBufferSize > 0)
    {
        sprintf(ptBuffers->errorMessageBuffer, "%s", errorMessage);
        ptBuffers->errorMessageBuffer[ptBuffers->errorMessageBufferSize - 1] = '\0';
    }
    
    return portableCode;
}
