/*
 * pty_getpty: open a PTY master.
 * and utmp entries.
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * 
 *Permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 */

#include "mit-copyright.h"
#include <com_err.h>
#include "libpty.h"
#include "pty-int.h"

long pty_getpty (fd, slave, slavelength)
    int slavelength;
    int *fd; char *slave;
{
    char c;
    char *p;
    int i,ptynum;
    struct stat stb;
char slavebuf[1024];


#ifdef HAVE_OPENPTY
    int slavefd;

    if(openpty(fd, &slavefd, slave, (struct termios *) 0,
         (struct winsize *) 0)) return 1;
close(slavefd);
    return 0;
#else

    *fd = open("/dev/ptmx", O_RDWR|O_NDELAY);	/* Solaris, IRIX */
    if (*fd < 0) *fd = open("/dev/ptc", O_RDWR|O_NDELAY); /* AIX */
    if (*fd < 0) *fd = open("/dev/pty", O_RDWR|O_NDELAY); /* sysvimp */

    if (*fd >= 0) {

#ifdef HAVE_GRANTPT
	if (grantpt(*fd) || unlockpt(*fd)) return PTY_GETPTY_STREAMS;
#endif
    
#ifdef HAVE_PTSNAME
	p = ptsname(*fd);
#else
#ifdef	HAVE_TTYNAME
	p = ttyname(*fd);
#else
	/* XXX If we don't have either what do we do */
#endif
#endif
	if (p) {
	    if ( strlen(p) > slavelength)
		{
		    close (*fd);
		    *fd = -1;
		    return PTY_GETPTY_SLAVE_TOOLONG;
		}
	    
	    strcpy(slave, p);
	    return 0;
	}

	if (fstat(*fd, &stb) < 0) {
	    close(*fd);
	    return PTY_GETPTY_FSTAT;
	}
	ptynum = (int)(stb.st_rdev&0xFF);
    sprintf(slavebuf, "/dev/ttyp%x", ptynum);
    if ( strlen(slavebuf) > slavelength) {
	close(*fd);
	*fd = -1;
	return PTY_GETPTY_SLAVE_TOOLONG;
    }
    strncpy ( slave, slavebuf, slavelength);
	return 0;

    } else {
    
	for (c = 'p'; c <= 's'; c++) {
	    sprintf(slavebuf,"/dev/ptyXX");
	    slavebuf[strlen("/dev/pty")] = c;
	    slavebuf[strlen("/dev/ptyp")] = '0';
	    if (stat(slavebuf, &stb) < 0)
		break;
	    for (i = 0; i < 16; i++) {
		slavebuf[sizeof("/dev/ptyp") - 1] = "0123456789abcdef"[i];
		*fd = open(slavebuf, O_RDWR);
		if (*fd < 0) continue;

		/* got pty */
		slavebuf[strlen("/dev/")] = 't';
		if ( strlen(slavebuf) < slavelength ) {
		    close ( *fd);
		    *fd = -1;
return PTY_GETPTY_SLAVE_TOOLONG;
		}
		strncpy ( slave, slavebuf, slavelength);
		
		return 0;
	    }
	}
	return PTY_GETPTY_NOPTY;
    }
#endif /* HAVE_OPENPTY */
}

