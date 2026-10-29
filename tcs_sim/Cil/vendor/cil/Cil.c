/*
** Module Name:
**    Cil.c
**
** Purpose:
**    Provides abstract user access to Inter Process Communications.
**
** Description:
**    This module contains CIL (Communication Interface Library) code
**    for communicating between modules. It presently abstracts a
**    mechanism that is based upon socket calls (using UDP).
**    The calls within this module are split into two parts. The calls
**    that may be made by external software are at the start of the
**    module and are prefixed with an "e". The internal functions are
**    defined in the second half of this module and start with "i".
**
** Authors:
**    apb: Aidan Pilkington-Burrows (TTL)
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**
** Version:
**    $Id: Cil.c,v 1.58 2007/03/12 15:07:30 sxf Exp $
**
** History:
**    $Log: Cil.c,v $
**    Revision 1.58  2007/03/12 15:07:30  sxf
**    Minor modifications to build under linux.
**
**    Revision 1.57  2006/08/23 10:55:26  mjf
**    Portability tweak to make VMS the exception.
**
**    Revision 1.56  2006/07/10 15:13:51  mjf
**    Code portability enhanced.
**
**    Revision 1.55  2005/08/18 14:01:56  mjf
**    VMS-specific 'unsigned' integer declaration required.
**
**    Revision 1.54  2005/08/18 12:53:52  mjf
**    Set size of UDP receive and send buffers during CilSetup.
**    Receive buffer set above default to help balance communications
**    load during OPERATIONAL ON/OFF.
**
**    Revision 1.53  2004/05/06 15:47:51  mjf
**    Don't return a send error if destination host is down.
**
**    Revision 1.52  2001/03/02 15:53:39  mjf
**    Moving of eCilNameString functions and eCilAlreadySetup functions
**    to source file CilUtil.c
**
**    Revision 1.51  2001/03/01 09:22:09  mjf
**    During an eCilReceive, correction to check for DataLen <= 0, as
**    DataLen is of type 'size_t' which is 'unsigned'.
**
**    Revision 1.50  2001/01/18 10:12:52  mjf
**    Change to system includes for VMS porting. CilSetup now accepts a
**    const filename.
**
**    Revision 1.49  2001/01/17 14:17:05  mjf
**    Attempt to match a hostname from the CIL map, if unsuccessful use
**    the entry as an IP address.
**
**    Revision 1.48  2001/01/17 10:12:23  mjf
**    Changed to use CFU for CIL map parsing.
**
**    Revision 1.47  2000/12/14 12:14:20  mjf
**    Addition of function to return whether a CIL setup has been performed.
**
**    Revision 1.46  2000/12/09 14:55:10  mjf
**    Addition of function to return the CIL ID in use by the calling process.
**
**    Revision 1.45  2000/12/04 11:25:15  mjf
**    Write time-stamp into packet headers during an eSclSend.
**
**    Revision 1.44  2000/10/26 16:01:57  djm
**    Minor change for porting to Linux.
**
**    Revision 1.43  2000/10/17 07:50:46  mjf
**    Addition of 'eCilFlush' function to flush the incoming message buffer.
**
**    Revision 1.42  2000/10/10 10:05:07  mjf
**    Addition of function 'eCilNameString' to return a string from memory
**    containing the CIL name for a specified CIL ID.
**
**    Revision 1.41  2000/10/05 15:24:18  asp
**    Changes to allow DAT
**
**    Revision 1.40  2000/08/18 15:35:24  djm
**    Modified the clean-up routine to allow complete clearing of the
**    destination list. This allows multiple calls to eCilSetup() in
**    the event of failure.
**
**    Revision 1.39  2000/08/11 13:17:42  mjf
**    Change to name of _CIL_IDLEN from I_ to E_ due to moving from private to
**    public header file.
**
**    Revision 1.38  2000/08/11 12:49:03  mjf
**    In function eCilLookup, match CIL name on full string, and not just the
**    first 'n' characters.
**
**    Revision 1.37  2000/08/07 08:48:19  sxf
**    Add #define _SOCKADDR_LEN to use BSD 4.4 compatible socket functions for VMS
**
**    Revision 1.36  2000/07/21 10:46:48  djm
**    Change to socket binding. Increased level of checking and error
**    reporting. Now processes must be on the correct host in order
**    to set-up with a CIL ID.
**
**    Revision 1.35  2000/07/14 16:28:14  djm
**    Changed handling of the recvfrom code for porting to Linux/GNU-C.
**
**    Revision 1.34  2000/06/22 17:28:10  djm
**    Improved comment header block for eCilConvert32bitArray().
**
**    Revision 1.33  2000/06/22 17:05:19  djm
**    Added byte-order conversion function.
**
**    Revision 1.32  2000/06/16 16:26:57  djm
**    Added cehck in eCilSend() for negative data lengths.
**    This should not normally be required as size_t is unsigned
**    no most systems. However, we are being extra-safe here.
**
**    Revision 1.31  2000/06/15 10:02:18  djm
**    Added new CIL return codes.
**
**    Revision 1.30  2000/06/14 12:02:20  djm
**    Changed a typecast in eCilReceive() for VMS compatibility.
**
**    Revision 1.29  2000/06/02 15:43:49  djm
**    Added eCilRawSend() function for transmitting raw data via UDP.
**
**    Revision 1.28  2000/05/18 10:14:42  djm
**    Added fclose() calls to eCilSetup to close the Cil.map file
**    after use. Also tidied up the code.
**
**    Revision 1.27  2000/05/09 10:59:31  djm
**    Removed assertion preventing zero-length data blocks to be sent.
**
**    Revision 1.26  2000/05/08 12:54:35  djm
**    Implemented new CIL header code.
**
**    Revision 1.25  2000/04/26 16:07:08  djm
**    Replaced iCilLogErr() with iCilReport().
**
**    Revision 1.24  2000/04/26 11:14:26  djm
**    Slight tidy-up of iCilCreateUdpSocket().
**
**    Revision 1.23  2000/04/26 08:48:28  djm
**    Added eCilName() function.
**
**    Revision 1.22  2000/04/20 09:53:59  djm
**    Added eCilLookup() function.
**
**    Revision 1.21  2000/04/19 12:46:57  djm
**    Added some additional diagnostic code.
**
**    Revision 1.20  2000/04/14 13:31:22  djm
**    Fixed problem where eCilReceive was returning a value too
**    large in the MsgPtr->DataLen field. This was due to an
**    incorrect calculation of the number of bytes received, failing
**    to take into account the CIL message header.
**
**    Revision 1.19  2000/04/14 08:11:15  djm
**    Slight change to diagnostic messages.
**
**    Revision 1.18  2000/04/13 13:46:43  djm
**    Fixed problem in eCilSetup() where the linked list IDs were
**    accidentally started at zero, rather than E_CIL_BOL+1.
**
**    Revision 1.17  2000/04/13 11:31:23  djm
**    Imporved error reporting in eCilReceive().
**
**    Revision 1.16  2000/04/11 13:11:25  djm
**    Provided better error messages in eCilSetup().
**    Made checks for CIL entries before E_CIL_BOL.
**
**    Revision 1.15  2000/04/05 14:02:41  djm
**    VMS compatibility not correctly implemented.
**    Casting changed to correct the problem.
**
**    Revision 1.14  2000/04/05 13:46:53  djm
**    Put some slight changes in to make the code compile
**    without retouching on an OpenVMS system.
**
**    Revision 1.13  2000/04/03 10:34:51  djm
**    Converted CIL IDs from "int" to "Uint32_t".
**
**    Revision 1.12  2000/03/27 16:30:16  djm
**    Changed API, so that there is no longer a eCilAssociate() call.
**    Did some work to bring it in line with the TTL coding standard.
**
**
**    Reworked and simplified. Removed eCilAssociate() call.
**    PLW & DJM 27-Mar-2000.
**
**    Added message structure (source, destination, body) to eCilSend and
**    eCilReceive calls. DJM 22-Mar-2000.
**
**    UDP only version for motion node development PLW Feb 2000.
**    All FLEET functionality etc removed.
**
**    May 1999: PLW Added 1st pass UDP functionality.
**    Needs conditional compilation switches for QNX, UNIX and VMS plus
**    restructuring and cleaning up. This version based on APB's original 
**    QNX only version.
**
*/


/* Standard headers */

#include <stdio.h>           /* UNIX 'Standard I/O' Definitions   */
#include <stdlib.h>          /* General Utilities */
#include <string.h>          /* String handling function definitions */
#include <stdarg.h>          /* For variable arg-list in iCilReport() */
#include <sys/time.h>
#include <errno.h>           /* For decoding socket bind errors */
#include <netdb.h>           /* For converting host names */


/* Define the following variable for verbose packet reporting */ 

#define nM_CIL_MSG_PRINT    TRUE
#define nM_CIL_MSG_REPORT   TRUE

#ifdef __VMS

#define  _SOCKADDR_LEN       /* Use BSD 4.4 compatible socket functions */
#include <unixio.h>          /* Prototypes for UNIX emulation functions */
#include <socket.h>          /* TCP UDP header */
#include <in.h>              /* TCP UDP header */
#include <inet.h>            /* TCP UDP header */

#else

#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/socket.h>      /* TCP UDP headers */
#include <netinet/in.h>      /* TCP UDP headers */
#include <arpa/inet.h>       /* TCP UDP headers */

#endif  /* __VMS */


/* System headers */

#include "Tim.h"
#include "Cfu.h"


/* Custom Headers */

#include "Cil.h"
#include "CilPrivate.h"


/* Package Global Data Declarations */

static iCilSock_t MySock;                   /* Socket identifier */
static Int32_t mMyCilId = E_CIL_INVALID;    /* CIL identifier in use */
static Uint32_t mCilIpAddress;              /* IP address of this instance */
static void mCilPrintMsg(char *FnStr, char *BufferPtr, size_t BufferLen);

/* Pointer to linked list */

extern iCilDest_t *iCilDestListPtr;

/* Lst of CIL names created during 'eCilSetup' returned by 'eCilName' */

extern char iCilName[ E_CIL_EOL + 1 ][ E_CIL_IDLEN ];



/* Public Functions, declared in Cil.h */

/*
** Function Name:
**    eCilSetup
**
** Type:
**    Status_t
**
** Purpose:
**    Establishes necessary setup for CIL operation.
**
** Description:
**    Initialises the library using the suppled name and the data
**    in the map file. It reads te entries sequentially from the 
**    map file. The other argument that is provided is the identity
**    code of the calling task. It is used to setup up a socket on
**    the corresponding IP address and port from the map.
**
** Arguments:
**    const char *MapFileNamePtr       (in)
**       Pointer to a character string containing the name of the
**       CIL map to be used.
**    Int32_t MyId                     (in)
**       ID code of the calling task. This should be a symbolic
**       constant, as defined in Cil.h, with a corresponging
**       entry in the CIL map file.
**
** Authors:
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**
** History:
**    17-Jan-2001 mjf Use CFU and use host names before IP addresses.
**    15-Jun-2000 djm Changed reporting of errors.
**    18-May-2000 djm Added fclose() to close Cil.map after use.
**    26-Apr-2000 djm Changed error logging calls to iCilReport().
**    27-Mar-2000 djm Modified original by PLW.
** 
*/

Status_t eCilSetup
(
   const char *MapFileNamePtr,
   Int32_t MyId
)
{
   char EntryName[ I_CIL_LINELEN + 1 ]; /* String for CIL name from file */
   char EntryHost[ I_CIL_LINELEN + 1 ]; /* String for host name from file */
   char EntryPort[ I_CIL_LINELEN + 1 ]; /* String for port name from file */
   char *TxtNamePtr;         /* Pointer to ID in text from CIL entry */
   char *TxtHostPtr;         /* Pointer to host in CIL entry text */
   char *TxtPortPtr;         /* Pointer to port number in CIL entry text */
   iCilDest_t *NewDestPtr;   /* Linked list pointer */
   iCilDest_t *LastDestPtr;  /* Linked list pointer */
   int DestCount;            /* Counter for number of CIL destinations */
   int IdCounter = E_CIL_BOL;/* ID counter (starts at 0 (c.f. Cil.h) */
   int LineCounter;          /* For detecting where map errors occur */
   Status_t Status;          /* Function return variable */
   Status_t CfuStatus;       /* Status returned by CFU functions */
   struct hostent *HostPtr;  /* Host entry - address details */


   /* If already setup just exit */
   if (iCilDestListPtr != NULL)
   {
      iCilReport("Multiple calls to Setup");
      return E_CIL_ALREADY_SETUP;
   }

   /* Make sure we have a correct map filename specification */
   if( (MapFileNamePtr == NULL) || (strlen(MapFileNamePtr) == 0) )
   {
      return E_CIL_NO_MAP_GIVEN;
   }

   /* Check to make sure our own ID is valid */
   if( (MyId <= E_CIL_BOL) || (MyId >= E_CIL_EOL) )
   {
      iCilReport("Setup parameter error. Task ID outside expected range");
      iCilReport("The requested ID for this task is not "
                 "within the recognised CIL ID range");
      return E_CIL_ID_INVALID;
   }

   /* Open the specified map file for reading */
   CfuStatus = eCfuSetup( MapFileNamePtr );
   if ( CfuStatus != SYS_NOMINAL )
   {
      iCilReport("Failed to open map file (%s)", MapFileNamePtr);
      eCfuComplete();
      return E_CIL_MAP_NOT_FOUND;
   }

   /* Set up basic data items */
   MySock.Socket = -1;
   DestCount = 0;

   /* Clear static array of CIL names */
   memset( iCilName, '\0', sizeof( iCilName ) );

   /* Read thru map file setting up destination table and local port */
   LineCounter = 1;

   while ( CfuStatus == SYS_NOMINAL )
   {
      /* Get a line from the file */
      CfuStatus = eCfuGetLine( NULL );

      /* If at least one argument found on the line */
      if ( eCfuParamCount() > 0 )
      {
         /* If Parse line into expected components */
         TxtNamePtr = NULL;
         TxtHostPtr = NULL;
         TxtPortPtr = NULL;

         if ( eCfuGetNumParam( I_CIL_PARAM_NAME, EntryName ) == SYS_NOMINAL )
         {
            TxtNamePtr = EntryName;
         }
         if ( eCfuGetNumParam( I_CIL_PARAM_HOST, EntryHost ) == SYS_NOMINAL )
         {
            TxtHostPtr = EntryHost;
         }
         if ( eCfuGetNumParam( I_CIL_PARAM_PORT, EntryPort ) == SYS_NOMINAL )
         {
            TxtPortPtr = EntryPort;
         }

         /* If valid entries read in correctly */
         if ((TxtNamePtr == NULL) || (strlen(TxtNamePtr) > E_CIL_IDLEN) ||
             (TxtHostPtr == NULL) || (TxtPortPtr == NULL))
         {
            iCilReport(
               "Map file format error on line %d of map file %s",
               LineCounter, MapFileNamePtr
            );

            iCilClearUp();
            eCfuComplete();
            return (E_CIL_MAP_ERROR);
         }

         /* Create new destination entry and add to the list */
         NewDestPtr = TTL_MALLOC(sizeof(iCilDest_t));
         if (NewDestPtr == NULL)
         {
            iCilReport("Destination creation failure");
            iCilClearUp();
            eCfuComplete();
            return (E_CIL_NEW_DEST_FAIL);
         }

         if (iCilDestListPtr == NULL)
         {
            iCilDestListPtr = LastDestPtr = NewDestPtr;
         }
         else
         {
            LastDestPtr->Next = NewDestPtr;
            LastDestPtr = NewDestPtr;
         }

         DestCount++;

         /* Fill in new entry */
         NewDestPtr->Next = NULL;
         NewDestPtr->ID = ++IdCounter;
         strncpy(NewDestPtr->TextId, TxtNamePtr, sizeof(NewDestPtr->TextId));

         /* Attempt to match the host name */
         HostPtr = gethostbyname( TxtHostPtr );
         /* If the host name matched in the host table */
         if ( HostPtr != NULL )
         {
            NewDestPtr->IpAddress = *( ( Uint32_t *) HostPtr->h_addr );
         }
         /* Else assume entry is an IP address */
         else
         {
            NewDestPtr->IpAddress = inet_addr(TxtHostPtr);
         }

         if (NewDestPtr->IpAddress == INADDR_NONE)
         {
            iCilReport(
               "Error creating new entry for line %d of map file %s",
               LineCounter, MapFileNamePtr
            );
            iCilClearUp();
            eCfuComplete();
            return (E_CIL_MAP_ERROR);
         }
         NewDestPtr->Port = htons(atoi(TxtPortPtr));

         /* Copy CIL name into static array */
         strcpy( iCilName[ NewDestPtr->ID ], NewDestPtr->TextId );
         /* Ensure string is zero-terminated */
         iCilName[ NewDestPtr->ID ][ E_CIL_IDLEN - 1 ] = '\0';

#ifdef M_CIL_MSG_PRINT
printf("Address of task %s -> %d %x %d\n", 
TxtIdPtr, IdCounter, NewDestPtr->IpAddress, ntohs(NewDestPtr->Port));
#endif


         /* If the map file entry matches the supplied id set up local socket */
         if (NewDestPtr->ID == MyId)
         {
            MySock.IpAddress = NewDestPtr->IpAddress;
            MySock.Port = NewDestPtr->Port;


#ifdef M_CIL_MSG_PRINT
printf("Address of this task --> %s %d %x %d\n", 
TxtIdPtr, IdCounter, MySock.IpAddress, ntohs(MySock.Port));
#endif


            Status = iCilCreateUdpSocket(&MySock);
            if (Status != SYS_NOMINAL)
            {
               iCilReport("Local socket creation failure");
               iCilClearUp();
               eCfuComplete();
               return Status;
            }

         }

      }   /* End of if eCfuParamCount() > 0 */

      LineCounter++;

   }   /* End of while CfuStatus == SYS_NOMINAL */

   /* Close the map file */
   eCfuComplete();
   

   /*
   ** If we didn't find an entry matching the name given we
   ** won't have created a local socket so won't be able to
   ** communicate with anyone.
   */

   if (MySock.Socket == -1)
   {
      iCilReport("Specified local name not found in map");
      iCilClearUp();
      return (E_CIL_UNKNOWN_NAME);
   }

   /*
   ** If we didn't find at least two entries in the map file
   ** no comms will be possible (i.e. we could only send 
   ** messages to ourself).
   */

   if (DestCount < 2)
   {
      iCilReport("Not enough entries in map file");
      iCilClearUp();
      return (E_CIL_MAP_TOO_SMALL);
   }

   /* Take copy of CIL identifier in use */
   mMyCilId = MyId;

   /* Take local copy of IP address in use */
   mCilIpAddress = ntohl( MySock.IpAddress );

   /* Setup successful - return success. */
   return (SYS_NOMINAL);

}  /* End of eCilSetup() */



/*
** Function Name:
**    eCilLookup
**
** Type:
**    Status_t
**
** Purpose:
**    Determines a CIL ID number from a text string and CIL map.
**
** Description:
**    This function will take a text string and look it up in the
**    specified CIL map. This allows the application to determine
**    the CIL ID using the text strings within the CIL map, rather
**    than using the hard-coded values that are available in Cil.h.
**
** Arguments:
**    const char *MapFileNamePtr       (in)
**       Pointer to a character string containing the name of the
**       CIL map to be used. This string is constant. It is also
**       expected to be null-terminated.
**    const char *CilNamePtr           (in)
**       Pointer to a null-terminated text string that contains the
**       name of the CIL ID that the application is trying to
**       determine. This string is constant.
**    Int32_t *CilIdPtr                (out)
**       A pointer to a variable that the eCilLookup function will
**      put a value of the ID matching the requested CIL ID code.
**
** Authors:
**    djm: Derek J. McKay (TTL)
**
** History:
**    11-Aug-2000 mjf Attempt string match on entire CIL name string.
**    15-Jun-2000 djm Changes to error return codes.
**    26-Apr-2000 djm Changed error logging calls to iCilReport().
**    26-Apr-2000 djm Added fclose() functions to tidy-up on return.
**    20-Apr-2000 djm Initial creation.
** 
*/

Status_t eCilLookup
(
   const char *MapFileNamePtr,
   const char *CilNamePtr,
   Int32_t *CilIdPtr
)
{
   char EntryName[ I_CIL_LINELEN + 1 ]; /* String for CIL name from file */
   char EntryHost[ I_CIL_LINELEN + 1 ]; /* String for host name from file */
   char EntryPort[ I_CIL_LINELEN + 1 ]; /* String for port name from file */
   char *TxtNamePtr;         /* Pointer to ID in text from CIL entry */
   char *TxtHostPtr;         /* Pointer to host in CIL entry text */
   char *TxtPortPtr;         /* Pointer to port number in CIL entry text */
   int IdCounter = E_CIL_BOL;/* ID counter (starts at 0 (c.f. Cil.h) */
   int LineCounter;          /* For detecting where map errors occur */
   Status_t CfuStatus;       /* Status returned by CFU functions */



   /* Do some basic sanity checks on the input parameters. */
   if ((MapFileNamePtr == NULL) || (strlen(MapFileNamePtr) == 0))
   {
      iCilReport("No CIL map file name was specified");
      return (E_CIL_NO_MAP_GIVEN);
   }

   if ((CilNamePtr == NULL) || (strlen(CilNamePtr) == 0))
   {
      iCilReport("No CIL name was specified");
      return (E_CIL_NO_NAME);
   }

   /* Open the specified map file for reading. */
   CfuStatus = eCfuSetup( MapFileNamePtr );
   if ( CfuStatus != SYS_NOMINAL )
   {
      iCilReport("Failed to open map file (%s)", MapFileNamePtr);
      eCfuComplete();
      return E_CIL_MAP_NOT_FOUND;
   }


   /* Read through the CIL map file, looking for the appropriate entry */
   LineCounter = 1;

   while ( CfuStatus == SYS_NOMINAL )
   {
      /* Get a line from the file */
      CfuStatus = eCfuGetLine( NULL );

      /* If at least one argument found on the line */
      if ( eCfuParamCount() > 0 )
      {
         /* If Parse line into expected components */
         TxtNamePtr = NULL;
         TxtHostPtr = NULL;
         TxtPortPtr = NULL;

         if ( eCfuGetNumParam( I_CIL_PARAM_NAME, EntryName ) == SYS_NOMINAL )
         {
            TxtNamePtr = EntryName;
         }
         if ( eCfuGetNumParam( I_CIL_PARAM_HOST, EntryHost ) == SYS_NOMINAL )
         {
            TxtHostPtr = EntryHost;
         }
         if ( eCfuGetNumParam( I_CIL_PARAM_PORT, EntryPort ) == SYS_NOMINAL )
         {
            TxtPortPtr = EntryPort;
         }

         /* If valid entries read in correctly */
         if ((TxtNamePtr == NULL) || (strlen(TxtNamePtr) > E_CIL_IDLEN) ||
             (TxtHostPtr == NULL) || (TxtPortPtr == NULL))
         {
            iCilReport(
               "Map file format error on line %d of map file %s",
               LineCounter, MapFileNamePtr
            );

            iCilClearUp();
            eCfuComplete();
            return (E_CIL_MAP_ERROR);
         }

         /* Increment the counter (which ID we are up to) */
         IdCounter++;

         /* Check here for match. */
         if ( strcmp( TxtNamePtr, CilNamePtr ) == 0 )
         {
            *CilIdPtr = IdCounter;
            eCfuComplete();
            return SYS_NOMINAL;
         }

      }   /* End of if eCfuParamCount() > 0 */

      LineCounter++;

   }   /* End of while CfuStatus == SYS_NOMINAL */

   /* If we get here, then we didn't find the required name. */
   iCilReport("The specified name could not found in CIL map");
   eCfuComplete();
   return (E_CIL_UNKNOWN_NAME);


}  /* End of eCilLookup() */


/*
** Function Name:
**    eCilName
**
** Type:
**    Status_t
**
** Purpose:
**    Looks up the CIL name in the CIL map for the given CIL ID.
**
** Description:
**    This function will lookup a CIL name in the specified CIL 
**    map that corresponds to the provided CIL ID. If the CIL ID 
**    has no corresponding name, then the function returns an error
**    and the CIL name variable is set to an empty string ("\0").
**    If the function performs successfully, it returns SYS_NOMINAL.
**    This function opens the Cil.map file, and thus, with file I/O
**    it should not be used in time critical code. In those cases, 
**    it is advised to create a lookup table in the application
**    prior to starting the "tight-code", of any CIL names that are
**    required.
**
** Arguments:
**    const char *MapFileNamePtr       (in)
**       Pointer to a character string containing the name of the
**       CIL map to be used. This string is constant. It is also
**       expected to be null-terminated.
**    Int32_t CilId                    (in)
**       A variable containing the CIL ID to decode.
**    size_t CilNameLen                (in)
**       Number of characters of storage in CilNamePtr. If this is
**       less than or equal to 1, an error is returned.
**    char *CilNamePtr                 (out)
**       Pointer to a text string in which to store the output
**       CIL name. It should have sufficient storage space to hold
**       CilNameLen characters (including a null termination). The
**       null character is placed in the CilNameLen'th character of
**       CilNamePtr, if the CIL name exceeds the size limit.
**
** Authors:
**    djm: Derek J. McKay (TTL)
**
** History:
**    15-Jun-2000 djm Changed returned error codes
**    26-Apr-2000 djm Changed error logging calls to iCilReport().
**    26-Apr-2000 djm Initial creation.
** 
*/

Status_t eCilName
(
   const char *MapFileNamePtr,
   Int32_t CilId,
   size_t CilNameLen,
   char *CilNamePtr
)
{
   char EntryName[ I_CIL_LINELEN + 1 ]; /* String for CIL name from file */
   char EntryHost[ I_CIL_LINELEN + 1 ]; /* String for host name from file */
   char EntryPort[ I_CIL_LINELEN + 1 ]; /* String for port name from file */
   char *TxtNamePtr;         /* Pointer to ID in text from CIL entry */
   char *TxtHostPtr;         /* Pointer to host in CIL entry text */
   char *TxtPortPtr;         /* Pointer to port number in CIL entry text */
   int IdCounter = E_CIL_BOL;/* ID counter (starts at 0 (c.f. Cil.h) */
   int LineCounter;          /* For detecting where map errors occur */
   Status_t CfuStatus;       /* Status returned by CFU functions */



   /* Do some basic sanity checks on the input parameters. */
   if ((MapFileNamePtr == NULL) || (strlen(MapFileNamePtr) == 0))
   {
      iCilReport("No CIL map file name was specified");
      return (E_CIL_NO_MAP_GIVEN);
   }

   if (CilNameLen < 2)
   {
      iCilReport("Insufficient storage space in output string");
      return (E_CIL_PARAM_ERR);
   }

   if ((CilId <= E_CIL_BOL) || (CilId >= E_CIL_EOL))
   {
      iCilReport("CIL ID does not exist (outside limits)");
      return (E_CIL_ID_INVALID);
   }

   /* Open the specified map file for reading */
   CfuStatus = eCfuSetup( MapFileNamePtr );
   if ( CfuStatus != SYS_NOMINAL )
   {
      iCilReport("Failed to open map file (%s)", MapFileNamePtr);
      eCfuComplete();
      return E_CIL_MAP_NOT_FOUND;
   }


   /* Read through map file to find the appropriate entry */
   LineCounter = 1;

   while ( CfuStatus == SYS_NOMINAL )
   {
      /* Get a line from the file */
      CfuStatus = eCfuGetLine( NULL );

      /* If at least one argument found on the line */
      if ( eCfuParamCount() > 0 )
      {
         /* If Parse line into expected components */
         TxtNamePtr = NULL;
         TxtHostPtr = NULL;
         TxtPortPtr = NULL;

         if ( eCfuGetNumParam( I_CIL_PARAM_NAME, EntryName ) == SYS_NOMINAL )
         {
            TxtNamePtr = EntryName;
         }
         if ( eCfuGetNumParam( I_CIL_PARAM_HOST, EntryHost ) == SYS_NOMINAL )
         {
            TxtHostPtr = EntryHost;
         }
         if ( eCfuGetNumParam( I_CIL_PARAM_PORT, EntryPort ) == SYS_NOMINAL )
         {
            TxtPortPtr = EntryPort;
         }

         /* If valid entries read in correctly */
         if ((TxtNamePtr == NULL) || (strlen(TxtNamePtr) > E_CIL_IDLEN) ||
             (TxtHostPtr == NULL) || (TxtPortPtr == NULL))
         {
            iCilReport(
               "Map file format error on line %d of map file %s",
               LineCounter, MapFileNamePtr
            );
            eCfuComplete();
            *CilNamePtr = '\0';
            return (E_CIL_MAP_ERROR);
         }

         /* Increment the counter (which ID we are up to) */
         IdCounter++;

         /* Check here for match. */
         if (IdCounter == CilId)
         {
            strncpy(CilNamePtr, TxtNamePtr, CilNameLen);
            eCfuComplete();
            return SYS_NOMINAL;
         }

      }   /* End of if eCfuParamCount() > 0 */

      LineCounter++;

   }   /* End of while CfuStatus == SYS_NOMINAL */

   /* 
   ** If we get here, then we didn't find the required name.
   ** Mind you, we should never get here... as we've already
   ** checked the CIL ID variable range at the start of the
   ** function.
   */

   iCilReport("The specified name could not found in CIL map");
   eCfuComplete();
   return (E_CIL_ID_INVALID);


}  /* End of eCilName() */


/*
** Function Name:
**    eCilSend
**
** Type:
**    Status_t
**
** Purpose:
**    Sends a data packet to a specified destination.
**
** Description:
**    ...
**
** Arguments:
**    Int32_t TxId             (in)
**       ID code of the desination task. This should be a symbolic
**       constant, as defined in Cil.h, with a corresponging
**       entry in the CIL map file.
**    eCilMsg_t MsgPtr         (in)
**       Pointer to a CIL message structure, the contents of which
**       will be sent to the requested address.
**
** Authors:
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**
** History:
**    15-Jun-2000 djm Changed some error return codes.
**    26-Apr-2000 djm Changed error logging calls to iCilReport().
**    27-Mar-2000 djm Modified original by PLW.
** 
*/

Status_t eCilSend
(
   Int32_t TxId,
   eCilMsg_t *MsgPtr
)
{
   iCilDest_t *NextDestPtr;
   Bool_t Matched;
   struct sockaddr_in To;
   size_t ToLen = sizeof(To);
   int TxCount;
   Status_t Status;        /* Function return status */
   Int32_t SrcCode;        /* Address code of message source */
   Int32_t DstCode;        /* Address code of message destination */
   Int32_t ClaCode;        /* Temporary variable for message class */
   Int32_t SvcCode;        /* Temporary variable for message service */
   Uint32_t SeqCode;       /* Temporary variable for sequence number */
   Int32_t Tm1Code;        /* Temporary variable for timestamp (sec) */
   Int32_t Tm2Code;        /* Temporary variable for timestamp (nsec) */
   char *BufferPtr;        /* Temporary buffer to hold CIL message */
   size_t BufferLen;       /* Size of the temporary buffer */
   char *TempPtr;          /* Temporary pointer to temporary buffer(!) */


#if 0
   printf("\n\nTx HDRBLK = %x %x %x %x %x %x %x (size = %d)\n\n",
      MsgPtr->SourceId,
      MsgPtr->DestId,
      MsgPtr->Class,
      MsgPtr->Service,
      MsgPtr->SeqNum,
      MsgPtr->TimeStamp.t_sec,
      MsgPtr->TimeStamp.t_nsec,
      I_CIL_HDRBLK_SIZE
   );
#endif

   if (iCilDestListPtr == NULL)
   {
      /* setup has not run yet */
      return (E_CIL_NO_SETUP);
   }

   /*
   ** Sanity check the input parameters.
   */

   if(MsgPtr == NULL)
   {
      return E_CIL_NULL_MSG;
   }

   if((long)MsgPtr->DataLen < 0)
   {
      return E_CIL_NEG_DATALEN;
   }

   /* search for matching associated destination */
   for
   (
      NextDestPtr = iCilDestListPtr, Matched = FALSE;
      NextDestPtr != NULL;
      NextDestPtr = NextDestPtr->Next
   )
   {
      if (NextDestPtr->ID != TxId)
      {
         continue;
      }
      else
      {
         Matched = TRUE;
         break;
      }
   }

   /* If we get to the end of the for-loop, and no destination was */
   /*    found, then indicate that we have an error */
   if (Matched == FALSE)
   {
      iCilReport("Failure to find ID=%d (0x%x)in CIL map", TxId);
      Status = E_CIL_ID_INVALID;
      return Status;
   }

   /* Get the current time and write into CIL message header */
   Status = eTimGetTime( &( MsgPtr->TimeStamp ) );

   /* Allocate some memory for a temporary transmission buffer    */
   /*    the size of it is the data block, plus the two addresses */
   BufferLen = I_CIL_HDRBLK_SIZE + MsgPtr->DataLen;
   BufferPtr = TTL_MALLOC(BufferLen);
   if (BufferPtr == NULL)
   {
      iCilReport("No memory to formulate transmission buffer");
      return E_CIL_NO_MEMORY;
   }

   /* Put all header block fields into network byte order */
   SrcCode = htonl(MsgPtr->SourceId);
   DstCode = htonl(MsgPtr->DestId);
   ClaCode = htonl(MsgPtr->Class);
   SvcCode = htonl(MsgPtr->Service);
   SeqCode = htonl(MsgPtr->SeqNum);
   Tm1Code = htonl(MsgPtr->TimeStamp.t_sec);
   Tm2Code = htonl(MsgPtr->TimeStamp.t_nsec);

   /* Add header block fields to message buffer */
   TempPtr = BufferPtr;
   memcpy(TempPtr, &SrcCode, sizeof(SrcCode));
   TempPtr += sizeof(SrcCode);
   memcpy(TempPtr, &DstCode, sizeof(DstCode));
   TempPtr += sizeof(DstCode);
   memcpy(TempPtr, &ClaCode, sizeof(ClaCode));
   TempPtr += sizeof(ClaCode);
   memcpy(TempPtr, &SvcCode, sizeof(SvcCode));
   TempPtr += sizeof(SvcCode);
   memcpy(TempPtr, &SeqCode, sizeof(SeqCode));
   TempPtr += sizeof(SeqCode);
   memcpy(TempPtr, &Tm1Code, sizeof(Tm1Code));
   TempPtr += sizeof(Tm1Code);
   memcpy(TempPtr, &Tm2Code, sizeof(Tm2Code));
   TempPtr += sizeof(Tm2Code);
   if(MsgPtr->DataLen > 0)
   {
      memcpy(TempPtr, MsgPtr->DataPtr, MsgPtr->DataLen);
   }

   /* Formulate the socket address */
   To.sin_family = AF_INET;
   To.sin_port = NextDestPtr->Port;
   To.sin_addr.s_addr = NextDestPtr->IpAddress;

   /* Print out the packet contents if M_CIL_MSG_PRINT is defined */
   mCilPrintMsg("eCilSend", BufferPtr, BufferLen);


   /* Finally we actually send the buffer of data */
   TxCount = sendto(MySock.Socket, BufferPtr, BufferLen, 0,
                    (void *) &To, ToLen);

   /* Free up our temporary buffer */
   TTL_FREE(BufferPtr);

   /* Check to see that the transmission occurred correctly */
   if (TxCount < 0)
   {
      /* Return an error except in specific cases */
      switch( errno )
      {
         /* No error if destination host is down - fire and forget */
         case EHOSTDOWN :
            Status = SYS_NOMINAL;
            break;
         /* Return an error in all other cases */
         default :
            Status = E_CIL_SEND_ERR;
            iCilReport("Datagram Send Failure");
            break;
      }
   }
   else
   {
      Status = SYS_NOMINAL;
   }

   /* Terminate the function by returning the status of the "send" */
   return Status;

}  /* End of eCilSend() */




Status_t eCilRawSend
(
   Int32_t DestId,
   size_t DataLen,
   char *DataPtr
)
{
/*
** Function Name:
**    eCilRawSend
**
** Type:
**    Status_t
**
** Purpose:
**    Sends a raw character array to an address specified by a CIL ID.
**
** Description:
**    Normally, CIL messages are sent with a CIL message header 
**    containing information for the source, destination, class, 
**    etc. thus specifying details of the transaction. However,
**    there may be times when raw UDP packets need to be sent, and
**    in those cases, this function may be used.
**
**    It sends a UDP packet composed of the data defined in the
**    character array pointed to by DataPtr.
**
** Arguments:
**    Int32_t DestId         (in)
**       CIL ID code of the desination task. This should be a symbolic
**       constant, as defined in Cil.h, with a corresponging
**       entry in the CIL map file.
**    size_t DataLen         (in)
**       Length of the DataPtr array that is sent to DestId.
**    char *DataPtr          (in)
**       Pointer to a character array of data to be sent.
**
** Authors:
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**
** History:
**    02-Jun-2000 djm Adapted from eCilSend().
** 
*/

   /* Local variables */
   iCilDest_t *NextDestPtr;  /* Loop marker for searching list of CIL IDs */
   Bool_t Matched;           /* Flag for checking if we found the CIL entry */
   struct sockaddr_in To;
   size_t ToLen = sizeof(To);
   int TxCount;              /* Count of the number of bytes actually sent */
   Status_t Status;          /* Function return status */


   if (iCilDestListPtr == NULL)
   {
      /* setup has not run yet */
      return (E_CIL_NO_SETUP);
   }

   /*
   ** Sanity check the input parameters.
   */
   if(DataPtr == NULL)
   {
      return E_CIL_NULL_DATA;
   }


   /* search for matching associated destination */

   for
   (
      NextDestPtr = iCilDestListPtr, Matched = FALSE;
      NextDestPtr != NULL;
      NextDestPtr = NextDestPtr->Next
   )
   {
      if (NextDestPtr->ID != DestId)
      {
         continue;
      }
      else
      {
         Matched = TRUE;
         break;
      }
   }

   if (Matched == FALSE)
   {
      iCilReport("Failure to find ID=%d (0x%x)in CIL map", DestId);
      Status = E_CIL_ID_INVALID;
      return Status;
   }


   /* Formulate the socket address */
   To.sin_family = AF_INET;
   To.sin_port = NextDestPtr->Port;
   To.sin_addr.s_addr = NextDestPtr->IpAddress;

   /* Print out the packet contents if M_CIL_MSG_PRINT is defined */
   mCilPrintMsg("eCilSend", DataPtr, DataLen);


   /* Finally we actually send the buffer of data */
   TxCount = sendto(MySock.Socket, DataPtr, DataLen, 0,
                    (void *) &To, ToLen);


   if (TxCount < 0)
   {
      /* Return an error except in specific cases */
      switch( errno )
      {
         /* No error if destination host is down - fire and forget */
         case EHOSTDOWN :
            Status = SYS_NOMINAL;
            break;
         /* Return an error in all other cases */
         default :
            Status = E_CIL_SEND_ERR;
            iCilReport("Datagram Send Failure");
            break;
      }
   }
   else
   {
      Status = SYS_NOMINAL;
   }

   /* Terminate the function by returning the status of the "send" */
   return Status;

}  /* End of eCilRawSend() */


/*
** Function Name:
**    eCilReceive
**
** Type:
**    Status_t
**
** Purpose:
**    Receive a data packet to a specified destination.
**
** Description:
**    ...
**
** Arguments:
**    int Timeout              (in)
**        The maximum time to wait for a packet, in milliseconds.
**        If this time expires before any data is recieved control
**        is returned to the caller and the returned status will
**        be E_CIL_TIMEOUT.
**        If this value is zero no delay will occur.
**    Int32_t *RxIdPtr         (out)
**       ID code of the task that sent the returned packet.
**    eCilMsg_t MsgPtr         (out)
**       Pointer to a CIL message structure, the contents of which
**       will be sent to the requested address.
**
** Authors:
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**
** History:
**    15-Jun-2000 djm Changes to returned error codes.
**    14-Jun-2000 djm Changed typecast in recvfrom() call in VMS code.
**    26-Apr-2000 djm Changed error logging calls to iCilReport().
**    13-Apr-2000 djm Improved error reporting.
**    27-Mar-2000 djm Modified original by PLW.
**
*/

Status_t eCilReceive
(
   int Timeout,
   Int32_t *RxIdPtr,
   eCilMsg_t *MsgPtr
)
{
   struct sockaddr_in From;
#if defined __linux__
   unsigned int FromLen = sizeof(From);
#else
   int FromLen = sizeof(From);
#endif
   iCilDest_t *NextDestPtr;
   int RxCount;
   Int32_t SrcCode;        /* Address code of message source */
   Int32_t DstCode;        /* Address code of message destination */
   Int32_t ClaCode;        /* Temporary variable for message class */
   Int32_t SvcCode;        /* Temporary variable for message service */
   Uint32_t SeqCode;       /* Temporary variable for sequence number */
   Int32_t Tm1Code;        /* Temporary variable for timestamp (sec) */
   Int32_t Tm2Code;        /* Temporary variable for timestamp (nsec) */
   char *BufferPtr;        /* Temporary buffer to hold CIL message */
   size_t BufferLen;       /* Size of the temporary buffer */
   char *TempPtr;          /* Temporary pointer to temporary buffer(!) */


   /* Make sure a map file has been setup (i.e. eCilSetup was run) */
   if (iCilDestListPtr == NULL)
   {
      return E_CIL_NO_SETUP;
   }

   /*
   ** Sanity check the input parameters.
   */

   /* Check that we have a valid location to write the Rx ID */
   if(RxIdPtr == NULL)
   {
      return E_CIL_NULL_RXID;
   }

   /* Check for a negative timeout */ 
   if(Timeout < 0)
   {
      return E_CIL_BAD_TIMEOUT;
   }

   /* Check to see we don't have a NULL pointer to direct the data to */
   if(MsgPtr == NULL)
   {
      return E_CIL_NULL_MSG;
   }

   /* Ensure that the data body space is large enough */
   if(MsgPtr->DataLen == 0)
   {
      return E_CIL_NO_STORAGE;
   }


   /* Check/wait/timeout on the socket having some data to receive */
   if (iCilSocketHasData(MySock.Socket, Timeout) == FALSE)
   {
      MsgPtr->DataLen = 0;
      return E_CIL_TIMEOUT;
   }

   /*
   ** Allocate some memory for a temporary reception buffer the
   ** size of it is the data block size, plus the header size.
   */

   BufferLen = I_CIL_HDRBLK_SIZE + MsgPtr->DataLen;
   BufferPtr = TTL_MALLOC(BufferLen);
   if (BufferPtr == NULL)
   {
      iCilReport("No memory to formulate reception buffer");
      return E_CIL_NO_MEMORY;
   }

   /* Actually retrieve the data from the socket */
#ifdef __VMS
   RxCount = recvfrom(MySock.Socket, (char *) BufferPtr, BufferLen, 0,
                      (struct sockaddr *) &From, (unsigned int *)&FromLen);
#else
   RxCount = recvfrom(MySock.Socket, (void *) BufferPtr, BufferLen, 0,
                      (struct sockaddr *) &From, &FromLen);
#endif

   /* Check to make sure there were no socket errors */
   if (RxCount < 0)
   {
      iCilReport("Socket read failure");
      MsgPtr->DataLen = 0;
      TTL_FREE(BufferPtr);
      return E_CIL_RECV_ERR;
   }


   /* Print out the packet contents if M_CIL_MSG_PRINT is defined */
   mCilPrintMsg("eCilReceive", BufferPtr, RxCount);


   /* Make sure we have at least the source/destination addresses */
   if (RxCount < I_CIL_HDRBLK_SIZE)
   {
      iCilReport("Incomplete addresses in message header");
      MsgPtr->DataLen = 0;
      TTL_FREE(BufferPtr);
      return E_CIL_GEN_ERR;
   }


   /* Extract the contents of buffer and put into a CIL message structure */
/*
   memcpy(&SrcCode, BufferPtr, sizeof(SrcCode));
   memcpy(&DstCode, BufferPtr + 4, sizeof(DstCode));
*/

   /* Extract header block fields from message buffer */
   TempPtr = BufferPtr;
   memcpy(&SrcCode, TempPtr, sizeof(SrcCode));
   TempPtr += sizeof(SrcCode);
   memcpy(&DstCode, TempPtr, sizeof(DstCode));
   TempPtr += sizeof(DstCode);
   memcpy(&ClaCode, TempPtr, sizeof(ClaCode));
   TempPtr += sizeof(ClaCode);
   memcpy(&SvcCode, TempPtr, sizeof(SvcCode));
   TempPtr += sizeof(SvcCode);
   memcpy(&SeqCode, TempPtr, sizeof(SeqCode));
   TempPtr += sizeof(SeqCode);
   memcpy(&Tm1Code, TempPtr, sizeof(Tm1Code));
   TempPtr += sizeof(Tm1Code);
   memcpy(&Tm2Code, TempPtr, sizeof(Tm2Code));
   TempPtr += sizeof(Tm2Code);

   /* Put all header block fields into network byte order */
   MsgPtr->SourceId = ntohl(SrcCode);
   MsgPtr->DestId   = ntohl(DstCode);
   MsgPtr->Class    = ntohl(ClaCode);
   MsgPtr->Service  = ntohl(SvcCode);
   MsgPtr->SeqNum   = ntohl(SeqCode);
   MsgPtr->TimeStamp.t_sec  = ntohl(Tm1Code);
   MsgPtr->TimeStamp.t_nsec = ntohl(Tm2Code);
   MsgPtr->DataLen = BufferLen - I_CIL_HDRBLK_SIZE;
   if (MsgPtr->DataLen > 0)
   {
      memcpy(MsgPtr->DataPtr, TempPtr, MsgPtr->DataLen);
   }
   TTL_FREE(BufferPtr);
   TempPtr = BufferPtr;

#if 0
   printf("\n\nRx HDRBLK = %x %x %x %x %x %x %x\n\n",
      MsgPtr->SourceId,
      MsgPtr->DestId,
      MsgPtr->Class,
      MsgPtr->Service,
      MsgPtr->SeqNum,
      MsgPtr->TimeStamp.t_sec,
      MsgPtr->TimeStamp.t_nsec
   );
#endif


/* DEBUG - Diagnostic to trace addresses */
/*
   iCilReport(
      "Message source (0x%X %X %d)",
      *RxIdPtr, ntohs(From.sin_addr.s_addr), ntohs(From.sin_port)
   );
*/

   /* Search for a matching associated destination */
   for
   (
      NextDestPtr = iCilDestListPtr;
      NextDestPtr != NULL;
      NextDestPtr = NextDestPtr->Next
   )
   {
      if (NextDestPtr->ID == -1)  continue;
      if ((NextDestPtr->IpAddress == From.sin_addr.s_addr) &&
          (NextDestPtr->Port == From.sin_port))
      {
         MsgPtr->DataLen = RxCount - I_CIL_HDRBLK_SIZE;
         *RxIdPtr = NextDestPtr->ID;
         return SYS_NOMINAL;
      }
   }

   /* If we get this far, then we must have received a message from */
   /* an unexpected source - one that is not in our list - in which */
   /* case, we discard it and return an error to the caller.        */

   iCilReport(
      "Unknown message source (0x%X %X %d)",
      *RxIdPtr, ntohl(From.sin_addr.s_addr), ntohs(From.sin_port)
   );
   MsgPtr->DataLen = 0;
   return E_CIL_RXID_UNKNOWN;

}  /* End of eCilReceive() */



/* Internal Functions, declared in CilPrivate.h */


/*
** Function Name:
**    iCilCreateUdpSocket
**
** Type:
**    Status_t
**
** Purpose:
**    Create a local UDP socket and binds it to an IP address.
**
** Description:
**    This function takes a iCilSock structure and opens a socket
**    for it. It then binds the IP address and port number specified
**    for the relevant CIL entry to this socket. If the socket is
**    on the incorrect host, or the socket is already in use, an
**    appropriate error is returned.
**
** Arguments:
**    iCilSock_t *MySockPtr    (in/out)
**       Pointer to a socket structure containg the local
**       address details (IP address + port) which is also
**       used to store the descriptor of the created socket.
**
** Authors:
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**
** History:
**    21-Jul-2000 djm Fixed problem with binding addresses to the
**                    socket. Also added more error codes, explaining
**                    why any failures have occurred. Also changed the
**                    return type from Bool_t to Status_t.
**    26-Apr-2000 djm Changed error logging calls to iCilReport().
**    27-Mar-2000 djm Modified original by PLW.
**
*/

static Status_t iCilCreateUdpSocket
(
   iCilSock_t *MySockPtr
)
{
   struct sockaddr_in SockAddr;
   int Socket;
   int Status;
#if defined __VMS || defined __linux__
   unsigned int BufferSize; 
#else
   int BufferSize; 
#endif
   int Buffer;     /* Size of existing receive buffer. */

   /* Create an unbound socket */
   if ((Socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
      iCilReport("Failed to create UDP socket");
      return E_CIL_SOCKET_FAIL;
   }

   /* Bind the local address to the socket */
   memset(&SockAddr, '\0', sizeof(SockAddr));
   /* The next line is not required in the Linux version. */
/*
   SockAddr.sin_len = sizeof(MySockPtr->IpAddress);
*/
   SockAddr.sin_family = AF_INET;
   SockAddr.sin_port = MySockPtr->Port;

#if 1
   /* This code makes use of a specific IP address (per the Cil.map) */
   SockAddr.sin_addr.s_addr = MySockPtr->IpAddress;
#else
   /* This code uses the default IP address, and will run on any host */
   SockAddr.sin_addr.s_addr = INADDR_ANY;
#endif


   /* Try to bind this address to our socket */
   if (bind(Socket, (void *) &SockAddr, sizeof(SockAddr)) < 0)
   {
      /* We have a problem, so close the socket... */
      close(Socket);
      /* ... and report the appropriate error */
      switch(errno)
      {
         case EADDRINUSE :
            iCilReport(
               "The IP address (%8X:%d) for this CIL ID is already in use",
               ntohl(MySockPtr->IpAddress), ntohs(MySockPtr->Port)
            );
            return E_CIL_ADDRINUSE;
            break;
         case EADDRNOTAVAIL :
            iCilReport(
               "The IP address %8X:%d is not available to the current host",
               ntohl(MySockPtr->IpAddress), ntohs(MySockPtr->Port)
            );
            return E_CIL_ADDRNOTAVAIL;
            break;
         default:
            iCilReport(
               "Failed to bind address %8X:%d",
               ntohl(MySockPtr->IpAddress), ntohs(MySockPtr->Port)
            );
            return E_CIL_GEN_ERR;
            break;
      }
   }

   /*
   ** Set receive buffer size.
   */
   BufferSize = sizeof(BufferSize);
   Status = getsockopt( Socket, SOL_SOCKET, SO_RCVBUF, 
                        (void*) &Buffer, &BufferSize);
   if (Status != 0)
   {
      iCilReport("Unable to get receive buffer options. Details follow..");

      switch(errno)
      {
         case EBADF :
            iCilReport("Invalid descriptor.");
            break;
         case EDOM :
            iCilReport("Value out of range.");
            break;
         case EFAULT :
            iCilReport("Illegal pointer.");
            break;
         case ENOPROTOOPT :
            iCilReport("This options in unknwon at the level indicated.");
            break;
         case ENOBUFS :
            iCilReport("No buffer space available.");
            break;
         default:
            iCilReport("Unknown errno - %d",errno);
            break;
      }
   }
   else
   {
     iCilReport("Receive buffer default size is %d",Buffer);
     Buffer = I_CIL_RCVBUF_SIZE;
     iCilReport("Setting size of receive buffer to %d.", Buffer);


     Status = setsockopt( Socket, SOL_SOCKET, SO_RCVBUF, 
                        (void*) &Buffer, BufferSize);
     if (Status != 0)
     {
       iCilReport("Unable to set receive buffer options. Details follow..");

       switch(errno)
       {
         case EBADF :
            iCilReport("Invalid descriptor.");
            break;
         case EDOM :
            iCilReport("Value out of range.");
            break;
         case EFAULT :
            iCilReport("Illegal pointer.");
            break;
         case ENOPROTOOPT :
            iCilReport("This options in unknwon at the level indicated.");
            break;
         case ENOBUFS :
            iCilReport("No buffer space available.");
            break;
         default:
            iCilReport("Unknown errno - %d",errno);
            break;
       }
     }
   }
   Status = getsockopt( Socket, SOL_SOCKET, SO_RCVBUF, 
                        (void*) &Buffer, &BufferSize);
   if (Status != 0)
   {
      iCilReport("Unable to get receive buffer options. Details follow..");

      switch(errno)
      {
         case EBADF :
            iCilReport("Invalid descriptor.");
            break;
         case EDOM :
            iCilReport("Value out of range.");
            break;
         case EFAULT :
            iCilReport("Illegal pointer.");
            break;
         case ENOPROTOOPT :
            iCilReport("This options in unknwon at the level indicated.");
            break;
         case ENOBUFS :
            iCilReport("No buffer space available.");
            break;
         default:
            iCilReport("Unknown errno - %d",errno);
            break;
      }
   }
   else
   {
     iCilReport("Receive buffer new size is %d",Buffer);
   }

   /*
   ** Set send buffer size.
   */
   Status = getsockopt( Socket, SOL_SOCKET, SO_SNDBUF, 
                        (void*) &Buffer, &BufferSize);
   if (Status != 0)
   {
      iCilReport("Unable to get send buffer options. Details follow..");

      switch(errno)
      {
         case EBADF :
            iCilReport("Invalid descriptor.");
            break;
         case EDOM :
            iCilReport("Value out of range.");
            break;
         case EFAULT :
            iCilReport("Illegal pointer.");
            break;
         case ENOPROTOOPT :
            iCilReport("This options in unknown at the level indicated.");
            break;
         case ENOBUFS :
            iCilReport("No buffer space available.");
            break;
         default:
            iCilReport("Unknown errno - %d",errno);
            break;
      }
   }
   else
   {
     iCilReport("Send buffer default size is %d",Buffer);
     Buffer = I_CIL_SNDBUF_SIZE;
     iCilReport("Setting size of send buffer to %d.", Buffer);


     Status = setsockopt( Socket, SOL_SOCKET, SO_SNDBUF, 
                        (void*) &Buffer, BufferSize);
     if (Status != 0)
     {
       iCilReport("Unable to set send buffer options. Details follow..");

       switch(errno)
       {
         case EBADF :
            iCilReport("Invalid descriptor.");
            break;
         case EDOM :
            iCilReport("Value out of range.");
            break;
         case EFAULT :
            iCilReport("Illegal pointer.");
            break;
         case ENOPROTOOPT :
            iCilReport("This options in unknown at the level indicated.");
            break;
         case ENOBUFS :
            iCilReport("No buffer space available.");
            break;
         default:
            iCilReport("Unknown errno - %d",errno);
            break;
       }
     }
   }
   Status = getsockopt( Socket, SOL_SOCKET, SO_SNDBUF, 
                        (void*) &Buffer, &BufferSize);
   if (Status != 0)
   {
      iCilReport("Unable to get send buffer options. Details follow..");

      switch(errno)
      {
         case EBADF :
            iCilReport("Invalid descriptor.");
            break;
         case EDOM :
            iCilReport("Value out of range.");
            break;
         case EFAULT :
            iCilReport("Illegal pointer.");
            break;
         case ENOPROTOOPT :
            iCilReport("This options in unknwon at the level indicated.");
            break;
         case ENOBUFS :
            iCilReport("No buffer space available.");
            break;
         default:
            iCilReport("Unknown errno - %d",errno);
            break;
      }
   }
   else
   {
     iCilReport("Send buffer new size is %d",Buffer);
   }
   /* If we get to this point, the binding worked. */

   /* Note the socket number in the structure and return success */
   MySockPtr->Socket = Socket;
   return SYS_NOMINAL;

}  /* End of iCilCreateUdpSocket() */




/*
** Function Name:
**    iCilSocketHasData
**
** Type:
**    Bool_t
**
** Purpose:
**    Check whether socket has data to read.
**
** Description:
**    Checks whether a socket has data available. Returns 
**    TRUE if data is available, FALSE otherwise. Does not
**    read the data.
**
** Arguments:
**    int Socket Socket        (in)
**       Descriptor of the socket to check.
**    int Timeout              (in)
**       Specifies the maximum time to wait for data, in
**       milliseconds. A zero value specifes no delay.
**
** Authors:
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**
** History:
**    27-Mar-2000 djm Adapted original by PLW to coding standard.
**
*/

static Bool_t iCilSocketHasData
(
   int Socket,
   int Timeout
)
{
   fd_set Readfd;
   struct timeval Tval;
   Bool_t RetVal;

   FD_ZERO(&Readfd);
   FD_SET(Socket, &Readfd);

   /* convert millisecond timout */

   Tval.tv_sec = Timeout / 1000;
   Tval.tv_usec = (Timeout % 1000) * 1000;

   switch (select(Socket + 1, &Readfd, NULL, NULL, &Tval))
   {
      case -1:
         RetVal = FALSE;
         iCilReport("Select failed for socket");
         break;
      case 0:
         /* no data available */
         RetVal = FALSE;
         break;
      default:
         /* descriptor has data */
         RetVal = TRUE;
         break;
   }

   return (RetVal);

}  /* End of iCilSocketHasData() */






/*
** Function Name:
**    iCilClearUp
**
** Type:
**    void
**
** Purpose:
**    Clear up CIL data space.
**
** Description:
**    Clear down internal data structures to initial state.
**    Currently only called when iCilSetup fails.
**
** Arguments:
**    ...
**
** Authors:
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**
** History:
**    18-Aug-2000 djm Ensure that MyDestListPtr = NULL after clean-up
**    27-Mar-2000 djm Adapted original by PLW to coding standard.
**
*/

static void iCilClearUp(void)
{
   /* Local variables */
   iCilDest_t *NextDestPtr;  /* Pointer to CIL linked list elements */


   /* Clear the link list of CIL addresses/names/IDs/etc. */
   while (iCilDestListPtr != NULL)
   {
      NextDestPtr = iCilDestListPtr->Next;
      TTL_FREE(iCilDestListPtr);
      iCilDestListPtr = NextDestPtr;
   }

   /* Reset the socket handler variable to "none" */
   MySock.Socket = -1;

}  /* End of iCilClearUp() */


/*
** iCilLogErr
**
** Comon function used by Cil to encapsulate the error logging
** function. Presently only  a holding function because the
** logging mechanism has not yet been defined.
**
** NOTE - THIS FUNCTION IS NOW REDUNDANT AND SHOULD NOT BE USED!
**        PLEASE USE iCilReport() INSTEAD.
**
**
** Parameters
**
** ErrMsgPtr - Pointer to a text string describing the error.
*/

#if 0

static void iCilLogErr(char *ErrMsgPtr)
{
   /*
   ** Holding function until real logging functions defined
   ** Since this is a general function errno won't be valid
   ** in all cases & should be treated with suspicion.
   */

   char ErrString[133] = "cil: ";

   strcat(ErrString, ErrMsgPtr);
   fprintf(stderr, "%s\n", ErrString);

#ifdef TTL_DEBUG

   perror("Cil");

#endif

}  /* End of iCilLogErr() */

#endif



static void iCilReport(char *FormatPtr, ...)
{
/*
** Function Name:
**    iCilReport
**
** Type:
**    void
**
** Purpose:
**    Message reporting function for the CIL package.
**
** Description:
**    iCilReport provides an ecapsulation of the error and message
**    reporting functions for the Communications Interface Library 
**    (CIL). It contains any TTL defined logging calls or any other
**    method for reporting information as required. It operates as
**    if the argument list was that of the ASNI/ISO-C standard 
**    library call: printf(). It has been adapted directly from the
**    similar function iMcbReport().
**
** Arguments:
**    char *FormatPtr        (in)
**       Argument list as per "printf()".
**
** Authors:
**    djm: Derek J. McKay (TTL)
**
** History:
**    26-Apr-2000 djm Initial creation.
**
*/

#ifdef M_CIL_MSG_REPORT

   /* Local variables */
   va_list Args;             /* Variable length argument list */


   /* Initialise variable argument list */
   va_start(Args, FormatPtr);

   /* Format and convey report */
   fprintf(stderr, "CIL: ");
   vfprintf(stderr, FormatPtr, Args);
   fprintf(stderr, "\n");

   /* Clean up and terminate function */
   va_end(Args);

#endif

   return;


} /* End of iCilReport() */


void mCilPrintMsg
(
   char *FnStr,
   char *BufferPtr,
   size_t BufferLen
)
/*
** Function Name:
**    mCilPrintMsg
**
** Type:
**    void
**
** Purpose:
**    Print the contents of the data packet.
**
** Description:
**    ...
**
** Arguments:
**    int FnStr                (in)
**       Null terminated text string to print.
**    char *BufferPtr          (in)
**       Data packet sent/received.
**    size_t BufferLen         (in)
**       Number of characters in BufferPtr to print.
**
** Authors:
**    djm: Derek J. McKay (TTL)
**
** History:
**    13-Apr-2000 djm Initial creation
**
*/
{

#ifdef M_CIL_MSG_PRINT

   /* Local variables */
   int CharCtr;             /* Loop counter for characters in buffer */


   /* Print header */
   fprintf(stderr, "---- Start of %s output ---\n", FnStr);


   /* Print characters in buffer */
   for(CharCtr = 0; CharCtr < BufferLen; CharCtr++)
   {
      fprintf(stderr, "%2.2x ", BufferPtr[CharCtr]);
   }


   /* Print footer */
   fprintf(stderr, "\n---- End of %s output (%d bytes) ---\n", FnStr, BufferLen);

#endif


   /* Function complete */
   return;

}  /* End of mCilPrintMsg() */



Status_t eCilConvert32bitArray(
   size_t NumBytes,
   void *ArrayPtr
)
{
/*
** Function Name:
**    eCilConvert32bitArray
**
** Type:
**    Status_t
**
** Purpose:
**    Converts from network to hardware byte order an array of 32bit integers.
**
** Description:
**    Often, TTL messages that are sent via CIL are comprised of essentially 
**    long lists of 32-bit words. Each of these needs to be converted
**    to/from network byte-order prior to transmission across the network. 
**    This applies to the entire body of the message (the CIL send and 
**    receive calls do the header byte-order conversion).
**
**    This function converts an array of 32-bit integers from network to 
**    hardware byte order, or vice-versa. Note that it just uses one ntohl()
**    call to do this, as the htonl() function is symmetrical to it. The 
**    function makes no checks on the length of the message and it assumes
**    that the array is valid.
**
** Arguments:
**    size_t NumBytes        (in)
**       The number of 8-bit bytes in the array to be converted. Note that
**       this value should always be a multiple of sizeof(Uint32_t)
**       (= 32-bits).
**    void *ArrayPtr         (in/out)
**       A pointer to the array containing the 32-bit words.
**
** Authors:
**    djm: Derek J. McKay (TTL)
**
** History:
**    22-Jun-2000 djm Initial creation.
**
*/

   /* Local variables */
   int Byte;                 /* Loop counter for words in the array */
   Uint32_t *TmpPtr;         /* Temporary pointer to the array */

   /* Assign the address */
   TmpPtr = (Uint32_t *)ArrayPtr;

   /* Loop through the array, word at a time */
   for(Byte = 0; Byte < NumBytes; Byte += sizeof(Uint32_t))
   {
      /* Convert the byte order of the next 32-bits */
      *TmpPtr = ntohl(*TmpPtr);

      /* Increment the array to point at the next word */
      TmpPtr++;
   }

   return SYS_NOMINAL;

}  /* End of eCilConvert32bitArray() */


/*******************************************************************************
**
** Function Name:
**    eCilFlush
**
** Type:
**    Status_t
**       Returns the completion status of this function - SYS_NOMINAL, 
**       E_CIL_NO_SETUP, or E_CIL_FLUSH_ERROR.
**
** Purpose:
**    Function to flush the incoming CIL message stack.
**
** Description:
**    This function repeatedly calls the function to receive an incoming CIL
**    message with zero timeout, disposing of the incoming messages. This 
**    function completes when either there are no more incoming messages, or
**    the number of messages flushed reaches I_CIL_MAX_FLUSH. If this maximum
**    number of messages are flushed, this will be due incoming CIL messages
**    being received faster than they can be read, and a flush error is 
**    returned.
**
** Arguments:
**    None
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
*******************************************************************************/

Status_t eCilFlush( void )
{
   int      RxCount;                   /* Number of messages flushed */
   Status_t Status;                    /* Status of eCilReceive() */
   Int32_t  CilId;                     /* CIL ID of deliverer of message */
   char     RxBuffer[ I_CIL_TMP_SIZE ];/* Size of temporary message buffer */
   eCilMsg_t CilMsg;                   /* For incoming CIL message */

   /* Make sure a map file has been setup (i.e. eCilSetup was run) */
   if (iCilDestListPtr == NULL)
   {
      return E_CIL_NO_SETUP;
   }

   /* Initialise the receive count */
   RxCount = 0;

   /* Flush messages until either none left or the maximum reached */
   do
   {
      /* Setup the incoming message */
      CilMsg.DataPtr = RxBuffer;
      CilMsg.DataLen = sizeof( RxBuffer );

      /* Attempt to receive an incoming CIL message */
      Status = eCilReceive( 0, &CilId, &CilMsg );

      /* If the attempted receive timed out, no more incoming packets */
      if ( Status == E_CIL_TIMEOUT )
      {
         /* Return SYS_NOMINAL, function completed successfully */
         return SYS_NOMINAL;
      }
      /* Else increment the count of packets flushed (even if rx error) */
      else
      {
         /* Increment the receive count */
         RxCount++;
      }
   }
   while ( RxCount < I_CIL_MAX_FLUSH );

   /* If got to here, then return a flush error */
   return E_CIL_FLUSH_ERROR;

}  /* End of eCilFlush() */


/*******************************************************************************
**
** Function Name:
**    eCilId
**
** Type:
**    Int32_t
**       Returns the CIL ID in use by the calling process. If no CIL ID is in 
**       use then E_CIL_INVALID is returned.
**
** Purpose:
**    This function returns the CIL ID currently in use.
**
** Description:
**    This function simply returns a copy of the CIL ID in use by the calling 
**    process.
**
** Arguments:
**    None
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
*******************************************************************************/

Int32_t  eCilId( void )
{

   /* simply return the CIL ID in use */
   return mMyCilId;

}


Status_t eCilSendDynamic     ( Int32_t TxId, eCilMsg_t * MsgPtr )
{
   struct sockaddr_in To;
   size_t ToLen = sizeof(To);
   int TxCount;
   Status_t Status;        /* Function return status */
   Int32_t SrcCode;        /* Address code of message source */
   Int32_t DstCode;        /* Address code of message destination */
   Int32_t ClaCode;        /* Temporary variable for message class */
   Int32_t SvcCode;        /* Temporary variable for message service */
   Uint32_t SeqCode;       /* Temporary variable for sequence number */
   Int32_t Tm1Code;        /* Temporary variable for timestamp (sec) */
   Int32_t Tm2Code;        /* Temporary variable for timestamp (nsec) */
   char *BufferPtr;        /* Temporary buffer to hold CIL message */
   size_t BufferLen;       /* Size of the temporary buffer */
   char *TempPtr;          /* Temporary pointer to temporary buffer(!) */

   iCilDynamic_t * DynamicPtr;   /* To overlay a dynamic CIL identifier */

   /*
   ** This function is similar to 'eCilSend', except that the 'TxId' is a
   ** dynamic identifier containing the 2 least-significant elements (of 4) from
   ** the destination IP address, and the port. The full IP address is formed
   ** using the 2 most-significant elements of our own IP address (as they must
   ** match).
   */

   /*
   ** Sanity check the input parameters.
   */

   if( MsgPtr == NULL )
   {
      return E_CIL_NULL_MSG;
   }

   if( (long) MsgPtr->DataLen < 0 )
   {
      return E_CIL_NEG_DATALEN;
   }

   /* Get the current time and write into CIL message header */
   Status = eTimGetTime( &( MsgPtr->TimeStamp ) );

   /* Allocate some memory for a temporary transmission buffer    */
   /*    the size of it is the data block, plus the two addresses */
   BufferLen = I_CIL_HDRBLK_SIZE + MsgPtr->DataLen;
   BufferPtr = TTL_MALLOC( BufferLen );
   if ( BufferPtr == NULL )
   {
      iCilReport( "No memory to formulate transmission buffer" );
      return E_CIL_NO_MEMORY;
   }

   /* Put all header block fields into network byte order */
   SrcCode = htonl(MsgPtr->SourceId);
   DstCode = htonl(MsgPtr->DestId);
   ClaCode = htonl(MsgPtr->Class);
   SvcCode = htonl(MsgPtr->Service);
   SeqCode = htonl(MsgPtr->SeqNum);
   Tm1Code = htonl(MsgPtr->TimeStamp.t_sec);
   Tm2Code = htonl(MsgPtr->TimeStamp.t_nsec);

   /* Add header block fields to message buffer */
   TempPtr = BufferPtr;
   memcpy(TempPtr, &SrcCode, sizeof(SrcCode));
   TempPtr += sizeof(SrcCode);
   memcpy(TempPtr, &DstCode, sizeof(DstCode));
   TempPtr += sizeof(DstCode);
   memcpy(TempPtr, &ClaCode, sizeof(ClaCode));
   TempPtr += sizeof(ClaCode);
   memcpy(TempPtr, &SvcCode, sizeof(SvcCode));
   TempPtr += sizeof(SvcCode);
   memcpy(TempPtr, &SeqCode, sizeof(SeqCode));
   TempPtr += sizeof(SeqCode);
   memcpy(TempPtr, &Tm1Code, sizeof(Tm1Code));
   TempPtr += sizeof(Tm1Code);
   memcpy(TempPtr, &Tm2Code, sizeof(Tm2Code));
   TempPtr += sizeof(Tm2Code);
   if( MsgPtr->DataLen > 0 )
   {
      memcpy( TempPtr, MsgPtr->DataPtr, MsgPtr->DataLen );
   }

   /* Overlay the dynamic identifier structure */
   DynamicPtr = (iCilDynamic_t *) &TxId;

   /* Formulate the socket address */
   To.sin_family      = AF_INET;
   To.sin_port        = htons( DynamicPtr->Port );
   To.sin_addr.s_addr = htonl( ( mCilIpAddress & I_CIL_DYN_MS_MASK )
                                + DynamicPtr->Address );

   /* Print out the packet contents if M_CIL_MSG_PRINT is defined */
   mCilPrintMsg( "eCilSend", BufferPtr, BufferLen );

   /* Finally we actually send the buffer of data */
   TxCount = sendto( MySock.Socket, BufferPtr, BufferLen, 0,
                     (void *) &To, ToLen );

   /* Free up our temporary buffer */
   TTL_FREE( BufferPtr );

   /* Check to see that the transmission occurred correctly */
   if ( TxCount < 0 )
   {
      /* Return an error except in specific cases */
      switch( errno )
      {
         /* No error if destination host is down - fire and forget */
         case EHOSTDOWN :
            Status = SYS_NOMINAL;
            break;
         /* Return an error in all other cases */
         default :
            Status = E_CIL_SEND_ERR;
            iCilReport( "Datagram Send Failure" );
            break;
      }
   }
   else
   {
      Status = SYS_NOMINAL;
   }

   /* Terminate the function by returning the status of the "send" */
   return Status;

}


Status_t eCilReceiveDynamic  ( int Timeout, Int32_t *RxIdPtr, eCilMsg_t *MsgPtr )
{
   struct sockaddr_in From;
#if defined __linux__
   unsigned int FromLen = sizeof(From);
#else
   int FromLen = sizeof(From);
#endif
   int RxCount;
   Int32_t SrcCode;        /* Address code of message source */
   Int32_t DstCode;        /* Address code of message destination */
   Int32_t ClaCode;        /* Temporary variable for message class */
   Int32_t SvcCode;        /* Temporary variable for message service */
   Uint32_t SeqCode;       /* Temporary variable for sequence number */
   Int32_t Tm1Code;        /* Temporary variable for timestamp (sec) */
   Int32_t Tm2Code;        /* Temporary variable for timestamp (nsec) */
   char *BufferPtr;        /* Temporary buffer to hold CIL message */
   size_t BufferLen;       /* Size of the temporary buffer */
   char *TempPtr;          /* Temporary pointer to temporary buffer(!) */

   iCilDynamic_t * DynamicPtr;   /* To overlay a dynamic CIL identifier */

   /*
   ** This function is similar to 'eCilReceive', but the originator of the
   ** message need not be in the CIL map. However, the first 2 (of 4) elements
   ** of the originator's IP address must be the same as our own.
   */

   /*
   ** Sanity check the input parameters.
   */

   /* Check that we have a valid location to write the Rx ID */
   if(RxIdPtr == NULL)
   {
      return E_CIL_NULL_RXID;
   }

   /* Check for a negative timeout */
   if(Timeout < 0)
   {
      return E_CIL_BAD_TIMEOUT;
   }

   /* Check to see we don't have a NULL pointer to direct the data to */
   if(MsgPtr == NULL)
   {
      return E_CIL_NULL_MSG;
   }

   /* Ensure that the data body space is large enough */
   if(MsgPtr->DataLen == 0)
   {
      return E_CIL_NO_STORAGE;
   }

   /* Check/wait/timeout on the socket having some data to receive */
   if (iCilSocketHasData(MySock.Socket, Timeout) == FALSE)
   {
      MsgPtr->DataLen = 0;
      return E_CIL_TIMEOUT;
   }

   /*
   ** Allocate some memory for a temporary reception buffer the
   ** size of it is the data block size, plus the header size.
   */

   BufferLen = I_CIL_HDRBLK_SIZE + MsgPtr->DataLen;
   BufferPtr = TTL_MALLOC(BufferLen);
   if (BufferPtr == NULL)
   {
      iCilReport("No memory to formulate reception buffer");
      return E_CIL_NO_MEMORY;
   }

   /* Actually retrieve the data from the socket */
#ifdef __VMS
   RxCount = recvfrom(MySock.Socket, (char *) BufferPtr, BufferLen, 0,
                      (struct sockaddr *) &From, (unsigned int *)&FromLen);
#else
   RxCount = recvfrom(MySock.Socket, (void *) BufferPtr, BufferLen, 0,
                      (struct sockaddr *) &From, &FromLen);
#endif

   /* Check to make sure there were no socket errors */
   if (RxCount < 0)
   {
      iCilReport("Socket read failure");
      MsgPtr->DataLen = 0;
      TTL_FREE(BufferPtr);
      return E_CIL_RECV_ERR;
   }

   /* Print out the packet contents if M_CIL_MSG_PRINT is defined */
   mCilPrintMsg("eCilReceive", BufferPtr, RxCount);

   /* Make sure we have at least the source/destination addresses */
   if (RxCount < I_CIL_HDRBLK_SIZE)
   {
      iCilReport("Incomplete addresses in message header");
      MsgPtr->DataLen = 0;
      TTL_FREE(BufferPtr);
      return E_CIL_GEN_ERR;
   }

   /* Check that the upper 16 bits of the originator match our IP address */
   if ( ( ntohl( From.sin_addr.s_addr ) & I_CIL_DYN_MS_MASK ) !=
        ( mCilIpAddress & I_CIL_DYN_MS_MASK ) )
   {
      iCilReport( "Message received from invalid IP address" );
      MsgPtr->DataLen = 0;
      TTL_FREE( BufferPtr );
      return E_CIL_RX_ADDR_ERR;
   }

   /* Extract the contents of buffer and put into a CIL message structure */
/*
   memcpy(&SrcCode, BufferPtr, sizeof(SrcCode));
   memcpy(&DstCode, BufferPtr + 4, sizeof(DstCode));
*/

   /* Extract header block fields from message buffer */
   TempPtr = BufferPtr;
   memcpy(&SrcCode, TempPtr, sizeof(SrcCode));
   TempPtr += sizeof(SrcCode);
   memcpy(&DstCode, TempPtr, sizeof(DstCode));
   TempPtr += sizeof(DstCode);
   memcpy(&ClaCode, TempPtr, sizeof(ClaCode));
   TempPtr += sizeof(ClaCode);
   memcpy(&SvcCode, TempPtr, sizeof(SvcCode));
   TempPtr += sizeof(SvcCode);
   memcpy(&SeqCode, TempPtr, sizeof(SeqCode));
   TempPtr += sizeof(SeqCode);
   memcpy(&Tm1Code, TempPtr, sizeof(Tm1Code));
   TempPtr += sizeof(Tm1Code);
   memcpy(&Tm2Code, TempPtr, sizeof(Tm2Code));
   TempPtr += sizeof(Tm2Code);

   /* Put all header block fields into network byte order */
   MsgPtr->SourceId = ntohl(SrcCode);
   MsgPtr->DestId   = ntohl(DstCode);
   MsgPtr->Class    = ntohl(ClaCode);
   MsgPtr->Service  = ntohl(SvcCode);
   MsgPtr->SeqNum   = ntohl(SeqCode);
   MsgPtr->TimeStamp.t_sec  = ntohl(Tm1Code);
   MsgPtr->TimeStamp.t_nsec = ntohl(Tm2Code);
   MsgPtr->DataLen = BufferLen - I_CIL_HDRBLK_SIZE;
   if (MsgPtr->DataLen > 0)
   {
      memcpy(MsgPtr->DataPtr, TempPtr, MsgPtr->DataLen);
   }
   TTL_FREE(BufferPtr);
   TempPtr = BufferPtr;

   /* DEBUG - Diagnostic to trace addresses */
/*
   iCilReport(
         "Message source (0x%X %X %d)",
         *RxIdPtr, ntohs(From.sin_addr.s_addr), ntohs(From.sin_port)
      );
*/


   /*
   ** Construct up the dynamic identifier, using the least-significant bits of
   ** the IP address and the port number.
   */

   /* Overlay the dynamic identifier structure */
   DynamicPtr = (iCilDynamic_t *) RxIdPtr;

   /* Copy the elements of the dynamic identifier */
   DynamicPtr->Address = ( ntohl( From.sin_addr.s_addr ) ) & I_CIL_DYN_LS_MASK;
   DynamicPtr->Port    = ntohs( From.sin_port );

   /* Decrement the receive length by the header size */
   MsgPtr->DataLen = RxCount - I_CIL_HDRBLK_SIZE;

   return SYS_NOMINAL;

}


Int32_t  eCilGetIdDynamic    ( Int32_t DynamicId )
{
   Int32_t  CilId;
   iCilDest_t *NextDestPtr;
   Uint32_t IpAddress;
   Uint16_t Port;

   iCilDynamic_t * DynamicPtr;   /* To overlay a dynamic CIL identifier */

   /*
   ** This function takes a 'dynamic' CIL ID and attempts to match it against
   ** the 'normal' CIL IDs from the CIL map.
   **
   ** If a match isn't found, E_CIL_INVALID is returned.
   */

   /* Overlay the dynamic identifier structure */
   DynamicPtr = (iCilDynamic_t *) &DynamicId;

   /*
   ** Construct up the IP address and port from the dynamic ID, also converting
   ** from host to network byte ordering.
   */
   IpAddress = htonl( ( mCilIpAddress & I_CIL_DYN_MS_MASK )
                       + DynamicPtr->Address );
   Port = htons( DynamicPtr->Port );

   /* Initially assume a match hasn't been found */
   CilId = E_CIL_INVALID;

   /* Loop around CIL entries looking for a match */
   for ( NextDestPtr = iCilDestListPtr;
         NextDestPtr != NULL;
         NextDestPtr = NextDestPtr->Next )
   {
      /* Check for a match in IP address and port */
      if ( ( NextDestPtr->IpAddress == IpAddress ) &&
           ( NextDestPtr->Port == Port ) )
      {
         CilId = NextDestPtr->ID;
         break;
      }
      else
      {
         continue;
      }
   }

   return CilId;

}


/* EOF */
