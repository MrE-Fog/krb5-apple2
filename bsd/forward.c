/*
 * appl/bsd/forward.c
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* General-purpose forwarding routines. These routines may be put into */
/* libkrb5.a to allow widespread use */ 

#if defined(KERBEROS) || defined(KRB5)
#include <stdio.h>
#include <pwd.h>
#include <netdb.h>

#include <krb5/krb5.h>
#include <krb5/asn1.h>
#include <krb5/los-proto.h>
#include <krb5/ext-proto.h>

/* Decode, decrypt and store the forwarded creds in the local ccache. */
krb5_error_code
rd_and_store_for_creds(context, inbuf, ticket, lusername)
     krb5_context context;
     krb5_data *inbuf;
     krb5_ticket *ticket;
     char *lusername;
{
    krb5_encrypt_block eblock;
    krb5_creds creds;
    krb5_error_code retval;
    char ccname[35];
    krb5_ccache ccache = NULL;
    struct passwd *pwd;

    if (retval = krb5_rd_cred(inbuf, ticket->enc_part2->session, 
			 &creds, 0, 0)) {
	return(retval);
    }
    
    if (!(pwd = (struct passwd *) getpwnam(lusername))) {
	return -1;
    }

    /* Set the KRB5CCNAME ENV variable to keep sessions 
     * seperate. Use the process id of this process which is 
     * the rlogind or rshd. Set the environment variable as well.
     */
  
    sprintf(ccname, "FILE:/tmp/krb5cc_p%d", getpid());
    setenv("KRB5CCNAME", ccname, 0);
  
    if (retval = krb5_cc_resolve(context, ccname, &ccache)) {
	return(retval);
    }

    if (retval = krb5_cc_initialize(context, ccache,
				    ticket->enc_part2->client)) {
	return(retval);
    }

    if (retval = krb5_cc_store_cred(context, ccache, &creds)) {
	return(retval);
    }

    if (retval = chown(ccname+5, pwd->pw_uid, -1)) {
	return(retval);
    }

    return retval;
}

#endif /* KERBEROS */
