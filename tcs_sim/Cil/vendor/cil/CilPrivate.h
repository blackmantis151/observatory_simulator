/*
** Header File Name:
**    CilPrivate.h
**
** Purpose:
**    Header file for internal CIL declarations, etc.
**
** Description:
**    This header file contains definitions for the Communications
**    Interface Library.
**
** Authors:
**    plw: Paul L. Wright
**    djm: Derek J. McKay
**
** Version:
**    $Id: CilPrivate.h,v 1.18 2005/08/18 12:53:52 mjf Exp $
**
** History:
**    $Log: CilPrivate.h,v $
**    Revision 1.18  2005/08/18 12:53:52  mjf
**    Set size of UDP receive and send buffers during CilSetup.
**    Receive buffer set above default to help balance communications
**    load during OPERATIONAL ON/OFF.
**
**    Revision 1.17  2005/06/20 13:25:56  mjf
**    Tidied up to aid porting to Linux - no functional changes.
**
**    Revision 1.16  2004/04/06 10:16:54  sxf
**    Changes for QNX v6 porting.
**
**    Revision 1.15  2001/03/02 15:52:55  mjf
**    Addition of default CIL name for functions in CIL utility file.
**
**    Revision 1.14  2001/01/17 10:12:23  mjf
**    Changed to use CFU for CIL map parsing.
**
**    Revision 1.13  2000/10/17 07:50:46  mjf
**    Addition of 'eCilFlush' function to flush the incoming message buffer.
**
**    Revision 1.12  2000/08/31 09:57:17  asp
**    *** empty log message ***
**
**    Revision 1.11  2000/08/11 13:16:28  mjf
**    Moving of _CIL_IDLEN macro (max length of CIL name) from private header
**    file into public header file.
**
**    Revision 1.10  2000/07/21 10:49:33  djm
**    Changed prototype of iCilCreateUdpSocket. Rationalised some
**    of the structures and tidied up the code layout.
**
**    Revision 1.9  2000/07/14 10:09:54  djm
**    Minor change to allow port to Linux/GNU-C.
**
**    Revision 1.8  2000/05/08 12:59:03  djm
**    Minor changes to allow for the new CIL header block.
**
**    Revision 1.7  2000/04/26 16:06:42  djm
**    Added iCilReport() function prototype.
**
**    Revision 1.6  2000/04/03 10:35:55  djm
**    Converted CIL IDs from "int" to "Uint32_t".
**
**    Revision 1.5  2000/03/27 16:36:43  djm
**    Modified for new version of Cil.c/Cil.h.
**    Some formatting changes made in order to bring it in line
**    with the TTL coding standard.
**
**    27-Mar-2000 djm Reworked header comment block.
**    27-Mar-2000 djm Modified for new programmer API.
**    ??-Feb-2000 plw Updated for UDP only implementation.
**    13-Mat-1999 plt Initial file creation
*/


#ifndef CIL_PRIVATE_H_DEFINED
#define CIL_PRIVATE_H_DEFINED


/* Predefined limits */

#define I_CIL_LINELEN      255 /* Maximum no. permitted entries in CIL map */
#define I_CIL_HDRBLK_SIZE  28  /* Number of bytes used in CIL header */
#define I_CIL_MAX_FLUSH    1024/* Maximum number of rx messages to flush */
#define I_CIL_TMP_SIZE     32  /* Size of temporary buffer during flush */

/* Parameters in the CIL map */

#define I_CIL_PARAM_NAME   1
#define I_CIL_PARAM_HOST   2
#define I_CIL_PARAM_PORT   3

/* Default CIL map name */

#ifdef E_WFL_OS_QNX4
#define I_CIL_DFLT_CIL_MAP "/opt/ttl/etc/Cil.map"
#elif defined E_WFL_OS_QNX6
#define I_CIL_DFLT_CIL_MAP "/ttl/sw/etc/Cil.map"
#else
#define I_CIL_DFLT_CIL_MAP E_SYS_DFLT_CFG_PATH "/Cil.map"
#endif

/* Size of UDP receive buffer (default QNX value is 41600). */
#define I_CIL_RCVBUF_SIZE  42624
/* Size of UDP send buffer (default QNX value is 9216). */
#define I_CIL_SNDBUF_SIZE  9216

/* Masks for portions of IP address for 'dynamic' functionality */

#define I_CIL_DYN_LS_MASK  0x0000ffff
#define I_CIL_DYN_MS_MASK  0xffff0000


/* Internal data structures */

typedef struct iCilSock_s {
   int         Socket;       /* File handler for socket */
   Uint32_t    IpAddress;    /* IP address */
   Uint16_t    Port;         /* Port number */
} iCilSock_t;

typedef struct iCilDest_s {
   Int32_t     ID;
   char        TextId[E_CIL_IDLEN + 1];
   Uint32_t    IpAddress;
   Uint16_t    Port;
   struct iCilDest_s *Next;
} iCilDest_t;

typedef struct iCilDynamic_s
{
   unsigned    Address : 16;
   unsigned    Port   :  16;
} iCilDynamic_t;

/* Prototypes for internal functions */

static Status_t iCilCreateUdpSocket(iCilSock_t *MySock);
static Bool_t iCilSocketHasData(int Socket, int Timeout);
static void iCilClearUp(void);
static void iCilReport(char *FormatPtr, ...);


#endif   /* CIL_PRIVATE_H_DEFINED */


/* EOF */
