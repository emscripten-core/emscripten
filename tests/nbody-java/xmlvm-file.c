
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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include "xmlvm-hy.h"
#include "xmlvm-file.h"
#include "hycomp.h"


/**
 * This will convert all separators to the proper platform separator
 * and remove duplicates on non POSIX platforms.
 */
void ioh_convertToPlatform (char *path)
{
    char *pathIndex;
#if !(DIR_SEPARATOR == '/')
    size_t length = strlen (path);
#endif
    
    /* Convert all separators to the same type */
    pathIndex = path;
    while (*pathIndex != '\0')
    {
        if ((*pathIndex == '\\' || *pathIndex == '/')
            && (*pathIndex != DIR_SEPARATOR))
            *pathIndex = DIR_SEPARATOR;
        pathIndex++;
    }

#if !(DIR_SEPARATOR == '/')
    /* Remove duplicate initial separators */
    pathIndex = path;
    while ((*pathIndex != '\0') && (*pathIndex == DIR_SEPARATOR))
    {
        pathIndex++;
    }
    if ((pathIndex > path) && ((int)length > (pathIndex - path))
        && (*(pathIndex + 1) == ':'))
    {
        /* For Example '////c:/!*' (! added to avoid gcc warning) */
        size_t newlen = length - (pathIndex - path);
        memmove (path, pathIndex, newlen);
        path[newlen] = '\0';
    }
    else
    {
        if ((pathIndex - path > 3) && ((int)length > (pathIndex - path)))
        {
            /* For Example '////serverName/!*' (! added to avoid gcc warning) */
            size_t newlen = length - (pathIndex - path) + 2;
            memmove (path, pathIndex - 2, newlen);
            path[newlen] = '\0';
        }
    }
    /* This will have to handle extra \'s but currently doesn't */
#endif
    
}


static I_32
EsTranslateOpenFlags (I_32 flags)
{
    I_32 realFlags = 0;
    
    if (flags & HyOpenAppend)
    {
        realFlags |= O_APPEND;
    }
    if (flags & HyOpenTruncate)
    {
        realFlags |= O_TRUNC;
    }
    if (flags & HyOpenCreate)
    {
        realFlags |= O_CREAT;
    }
    if (flags & HyOpenCreateNew)
    {
        realFlags |= O_EXCL | O_CREAT;
    }
#ifdef O_SYNC
	if (flags & HyOpenSync) {
		realFlags |= O_SYNC;
	}
#endif    
    if (flags & HyOpenRead)
    {
        if (flags & HyOpenWrite)
        {
            return (O_RDWR | realFlags);
        }
        return (O_RDONLY | realFlags);
    }
    if (flags & HyOpenWrite)
    {
        return (O_WRONLY | realFlags);
    }
    return -1;
}


static I_32 findError (I_32 errorCode)
{
    switch (errorCode)
    {
        case EACCES:
        case EPERM:
            return HYPORT_ERROR_FILE_NOPERMISSION;
        case ENAMETOOLONG:
            return HYPORT_ERROR_FILE_NAMETOOLONG;
        case ENOENT:
            return HYPORT_ERROR_FILE_NOENT;
        case ENOTDIR:
            return HYPORT_ERROR_FILE_NOTDIR;
        case ELOOP:
            return HYPORT_ERROR_FILE_LOOP;
            
        case EBADF:
            return HYPORT_ERROR_FILE_BADF;
        case EEXIST:
            return HYPORT_ERROR_FILE_EXIST;
        case ENOSPC:
        case EFBIG:
            return HYPORT_ERROR_FILE_DISKFULL;
        default:
            return HYPORT_ERROR_FILE_OPFAILED;
    }
}



I_32 hyfile_close (IDATA fd)
{
    
#if (FD_BIAS != 0)
    if (fd < FD_BIAS) {
        /* Cannot close STD streams, and no other FD's should exist <FD_BIAS */
	    return -1;
    }
#endif
    
    return close ((int) (fd - FD_BIAS));
}



IDATA hyfile_open (const char *path, I_32 flags, I_32 mode)
{
    struct stat buffer;
    I_32 fd;
    I_32 realFlags = EsTranslateOpenFlags (flags);
    I_32 fdflags;
    
    // Trc_PRT_file_open_Entry (path, flags, mode);
    
    if (realFlags == -1)
    {
        // Trc_PRT_file_open_Exit1 (flags);
        hyerror_set_last_error(EINVAL, findError(EINVAL));
        return -1;
    }
    
    if ( ( flags&HyOpenRead && !(flags&HyOpenWrite) )  && !stat (path, &buffer))
    {
        if (S_ISDIR (buffer.st_mode))
        {
            hyerror_set_last_error_with_message(findError(EEXIST), "Is a directory");
            // Trc_PRT_file_open_Exit4 ();
            return -1;
        }
    }
    
    fd = open (path, realFlags, mode);
    
    if (-1 == fd)
    {
        // Trc_PRT_file_open_Exit2 (errno, findError (errno));
        int rc = errno;
        hyerror_set_last_error(rc, findError(rc));
        return -1;
    }
    
    /* Tag this descriptor as being non-inheritable */
    fdflags = fcntl (fd, F_GETFD, 0);
    fcntl (fd, F_SETFD, fdflags | FD_CLOEXEC);
    
    fd += FD_BIAS;
    // Trc_PRT_file_open_Exit (fd);
    return (IDATA) fd;
}



IDATA hyfile_read (IDATA fd, void *buf, IDATA nbytes)
{
    IDATA result;
    if (nbytes == 0)
    {
        return 0;
    }
    
#ifdef ZOS
    if (fd == HYPORT_TTY_IN) {
        result = fread(buf, sizeof(char), nbytes, stdin);
    }  else	if (fd < FD_BIAS) {
        /* Cannot read from STDOUT/ERR, and no other FD's should exist <FD_BIAS */
        return -1;
    } else
#endif /* ZOS */
    {
        result = read ((int) (fd - FD_BIAS), buf, nbytes);
    }
    
    if (result == 0)
    {
        return -1;
    }
    else
    {
        return result;
    }
}



I_64 hyfile_seek (IDATA inFD, I_64 offset, I_32 whence)
{
  int fd = (int)inFD;
  off_t localOffset = (off_t) offset;

  if ((whence < HySeekSet) || (whence > HySeekEnd))
    {
      return -1;
    }

  /* If file offsets are 32 bit, truncate the seek to that range */
  if (sizeof (off_t) < sizeof (I_64))
    {
      if (offset > 0x7FFFFFFF)
        {
          localOffset = 0x7FFFFFFF;
        }
      else if (offset < -0x7FFFFFFF)
        {
          localOffset = -0x7FFFFFFF;
        }
    }

#if (FD_BIAS != 0)
	if (fd < FD_BIAS) {
		/* Cannot seek on STD streams, and no other FD's should exist <FD_BIAS */
		return -1;
	}
#endif

  return (I_64) lseek ((int) (fd - FD_BIAS), localOffset, whence);
}



IDATA hyfile_write (IDATA fd, const void *buf, IDATA nbytes)
{
    IDATA rc = 0;
    
#ifdef ZOS
    if (fd == HYPORT_TTY_OUT) {
        rc = fwrite(buf, sizeof(char), nbytes, stdout);
    } else if (fd == HYPORT_TTY_ERR) {
        rc = fwrite(buf, sizeof(char), nbytes, stderr);
    } else if (fd < FD_BIAS) {
        /* Cannot fsync STDIN, and no other FD's should exist <FD_BIAS */
        return -1;
    } else 
#endif /* ZOS */
    {
        /* write will just do the right thing for HYPORT_TTY_OUT and HYPORT_TTY_ERR */
        rc = write ((int) (fd - FD_BIAS), buf, nbytes);
    }
    
    if (rc == -1)
    {
        int rc = errno;
        return hyerror_set_last_error(rc, findError(rc));
    }
    
    return rc;
}
