/*
**       Copyright (c) Telescope Technologies Limited, 2000
**
** Header File Name:
**    Cil.h
**
** Purpose:
**    Provides user access to the CIL routines and data structures.
**
** Description:
**    This file must be included by all processes wishing to use the
**    CIL routines for interprocess comms. It provides an abstract interface
**    to the underlying communications method and definitions of the data
**    items and structures to be used when using the interface.
**
** Authors:
**    apb: Aidan Pilkington-Burrows (TTL)
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**
** Version:
**    $Id: Cil.h,v 1.44 2007/07/12 13:06:12 mjf Exp $
**
** History:
**    $Log: Cil.h,v $
**    Revision 1.44  2007/07/12 13:06:12  mjf
**    Re-format enumeration definition for lookup table generation.
**
**    Revision 1.43  2003/02/04 11:31:47  mjf
**    Addition of entries for Nasmyth X+, Nasmyth X- and Prime focal
**    stations, in terms of a de-rotator axis, a suite of auxiliary
**    mechanisms and an autoguider.
**
**    Revision 1.42  2001/11/30 15:30:59  sxf
**    Added AGP, AGG and AGI for autoguider.
**
**    Revision 1.41  2001/07/17 09:11:55  sxf
**    Added AGG for Autoguider guide packets
**
**    Revision 1.40  2001/03/26 18:03:06  mjf
**    Addition of CIL identifiers for primary mirror support.
**
**    Revision 1.39  2001/03/05 13:28:31  mjf
**    Change of CIL names for AMx to be OMx (optical).
**
**    Revision 1.38  2001/03/02 16:43:41  mjf
**    Change of CIL names for Auxiliary Mechanism Nodes from AM? to XM?, as
**    requested by RDL.
**
**    Revision 1.37  2001/03/02 09:35:23  mjf
**    Addition of Computer Monitoring Tasks.
**
**    Revision 1.36  2001/01/18 10:12:52  mjf
**    Change to system includes for VMS porting. CilSetup now accepts a
**    const filename.
**
**    Revision 1.35  2000/12/14 12:14:20  mjf
**    Addition of function to return whether a CIL setup has been performed.
**
**    Revision 1.34  2000/12/11 18:32:08  mjf
**    Addition of TST - Test Scripting Tool.
**
**    Revision 1.33  2000/12/09 14:55:10  mjf
**    Addition of function to return the CIL ID in use by the calling process.
**
**    Revision 1.32  2000/10/17 07:50:46  mjf
**    Addition of 'eCilFlush' function to flush the incoming message buffer.
**
**    Revision 1.31  2000/10/10 10:05:07  mjf
**    Addition of function 'eCilNameString' to return a string from memory
**    containing the CIL name for a specified CIL ID.
**
**    Revision 1.30  2000/10/10 08:11:03  mjf
**    Addition of 10 Computer Monitoring Tasks (CMT).
**
**    Revision 1.29  2000/09/19 07:33:19  mjf
**    Addition of Engineering Reporting Tasks, Autoguider, and major changes
**    to ACNs. Also extensive reordering.
**
**    Revision 1.28  2000/08/23 16:47:24  asp
**    New Cil entries added DA, SFR, AI, UI, MIT. Old IU entries renamed to EI
**
**    Revision 1.27  2000/08/11 13:16:28  mjf
**    Moving of _CIL_IDLEN macro (max length of CIL name) from private header
**    file into public header file.
**
**    Revision 1.26  2000/07/21 10:48:17  djm
**    Added two new error codes for set-up failure.
**
**    Revision 1.25  2000/07/14 16:06:05  djm
**    Added another CIL test ID for the IUCAA prototype TCS.
**
**    Revision 1.24  2000/07/05 09:49:48  djm
**    Added CIL ID for the OCS.
**
**    Revision 1.23  2000/06/22 17:05:49  djm
**    Added function prototype for byte-order conversion function.
**
**    Revision 1.22  2000/06/16 16:26:21  djm
**    Added new error code for negative data lengths.
**
**    Revision 1.21  2000/06/15 10:02:37  djm
**    Altered CIL error code list.
**
**    Revision 1.20  2000/06/02 16:00:18  djm
**    Added E_CIL_LOG CIL ID enumeration.
**    Added function prototype for eCilRawSend().
**
**    Revision 1.19  2000/05/31 15:03:40  djm
**    Fixed typographical error.
**
**    Revision 1.18  2000/05/31 14:21:32  djm
**    Major rearrangement of CIL addresses.
**
**    Revision 1.17  2000/05/24 16:47:22  djm
**    Added new message class to be distict from E_CIL_ACK_CLASS, but
**    indicate receipt of a command for a long-term action (for TCS).
**
**    Revision 1.16  2000/05/08 12:58:07  djm
**    Adapted to suit the new CIL header block by modiying the
**    eCilMsg_t structure and adding the eCilClass enumeration.
**
**    Revision 1.15  2000/04/26 08:47:11  djm
**    Added prototype for eCilName().
**
**    Revision 1.14  2000/04/20 09:41:17  djm
**    Added prototype for eCilLookup().
**
**    Revision 1.13  2000/04/18 17:40:39  djm
**    Added addition test units (now TU0-TU9).
**
**    Revision 1.12  2000/04/11 13:12:28  djm
**    Added CHB entry in identifier list. Also changed order.
**
**    Revision 1.11  2000/04/03 13:27:50  djm
**    Minor change to comments.
**
**    Revision 1.10  2000/04/03 10:35:34  djm
**    Converted CIL IDs from "int" to "Uint32_t".
**
**    Revision 1.9  2000/03/27 16:35:51  djm
**    Now contains enumerated telescope subsystem list.
**    Some formatting changes made to bring it in line with
**    the TTL coding standard.
**
**
**    27-Mar-2000 djm Added new header comment block.
**    27-Mar-2000 djm Reworked for new API.
**    22-Mar-2000 djm Added non-primative message structure and 
**                    enumerate list of telescope subsystem codes.
**    ??-Feb-2000 plw UDP only. For motion node development.
**                    All FLEET functionality etc stripped out.
**    ??-May-1999 plw Added 1st pass UDP functionality.
**                    Needs conditional compilation switches for 
**                    QNX, UNIX and VMS plus restructuring and
**                    cleaning up. This version based on ADP's 
**                    original QNX only version.
**
*/


#ifndef CIL_H_DEFINED
#define CIL_H_DEFINED

#include <stddef.h>
#include "TtlSystem.h"



/*
** List of address codes for the telescope subsystems. Note that the
** enumeration MUST start at zero and have the last element as 
** E_CIL_EOL in order to work (c.f. eCilSetup() in Cil.h).
*/

typedef enum eCilNames_e
{
   E_CIL_BOL = 0,  /* Beginning of list (DO NOT USE FOR A TASK ID) */

   /*  MCS - MCP   */

   E_CIL_MCP,      /* Master Control Process */
   E_CIL_CHB,      /* Continuous Heartbeat (part of MCP package) */
   E_CIL_UI1,      /* Un-interruptable power-supply Interface task 1 */
   E_CIL_UI2,      /* Un-interruptable power-supply Interface task 2 */
   E_CIL_AI1,      /* Addressable power-switch Interface task 1 */
   E_CIL_AI2,      /* Addressable power-switch Interface task 2 */
   E_CIL_AI3,      /* Addressable power-switch Interface task 3 */
   E_CIL_MIT,      /* Modem Interface Task */

   /* MCS - rest   */

   E_CIL_MCB,      /* Master Command Broker */
   E_CIL_SDB,      /* Status Database */
   E_CIL_SFR,      /* Status database File Recovery task */
   E_CIL_SPT,      /* Services PLC Task */
   E_CIL_EPT,      /* Enclosure PLC Task */
   E_CIL_EPS,      /* Enclosure PLC Simulator */
   E_CIL_WMS,      /* Weather Monitoring System */
   E_CIL_AVS,      /* Audio-Visual System */

   /* TCS          */

   E_CIL_TCS,      /* Telescope Control System */
   E_CIL_RCS,      /* Robotic Control System */
   E_CIL_OCS,      /* Observatory Control System */
   E_CIL_AGS,      /* Autoguider system */
   E_CIL_AGP,      /* Autoguider guide packets */
   E_CIL_AGG,      /* Autoguider guide process */
   E_CIL_AGI,      /* Autoguider graphical interface */

   /* ECI - note that the ERx block must follow directly */

   E_CIL_EI0,      /* Engineering control Interface (session 0)             */
   E_CIL_EI1,      /* Engineering control Interface (session 1)             */
   E_CIL_EI2,      /* Engineering control Interface (session 2)             */
   E_CIL_EI3,      /* Engineering control Interface (session 3)             */
   E_CIL_EI4,      /* Engineering control Interface (session 4)             */

   /* ERT - note that these must directly follow the EIx block */

   E_CIL_ER0,      /* Engineering Reporting Task (session 0)                */
   E_CIL_ER1,      /* Engineering Reporting Task (session 1)                */
   E_CIL_ER2,      /* Engineering Reporting Task (session 2)                */
   E_CIL_ER3,      /* Engineering Reporting Task (session 3)                */
   E_CIL_ER4,      /* Engineering Reporting Task (session 4)                */

   /* Axis nodes   */

   E_CIL_AZC,      /* Azimuth ACN Comms-In                                  */
   E_CIL_AZN,      /* Azimuth ACN Control Node                              */
   E_CIL_AZS,      /* Azimuth ACN Comms-Out                                 */
   E_CIL_AZR,      /* Azimuth ACN SDB Reporting                             */
   E_CIL_AZT,      /* Azimuth ACN Test (formerly Simulator AZS)             */
   E_CIL_AZL,      /* Azimuth ACN Logger                                    */

   E_CIL_ELC,      /* Elevation ACN Comms-In                                */
   E_CIL_ELN,      /* Elevation ACN Control Node                            */
   E_CIL_ELS,      /* Elevation ACN Comms-Out                               */
   E_CIL_ELR,      /* Elevation ACN SDB Reporting                           */
   E_CIL_ELT,      /* Elevation ACN Test (formerly Simulator ELS)           */
   E_CIL_ELL,      /* Elevation ACN Logger                                  */

   E_CIL_CSC,      /* Cassegrain ACN Comms-In                               */
   E_CIL_CSN,      /* Cassegrain ACN Control Node                           */
   E_CIL_CSS,      /* Cassegrain ACN Comms-Out                              */
   E_CIL_CSR,      /* Cassegrain ACN SDB Reporting                          */
   E_CIL_CST,      /* Cassegrain ACN Test (formerly Simulator CSS)          */
   E_CIL_CSL,      /* Cassegrain ACN Logger                                 */

   E_CIL_OMC,      /* Auxiliary (Optical) Mechanism Comms-In                */
   E_CIL_OMN,      /* Auxiliary (Optical) Mechanism Control Node            */
   E_CIL_OMS,      /* Auxiliary (Optical) Mechanism Comms-Out               */
   E_CIL_OMR,      /* Auxiliary (Optical) Mechanism SDB Reporting           */
   E_CIL_OMT,      /* Auxiliary (Optical) Mechanism Test (formerly AMS)     */
   E_CIL_OML,      /* Auxiliary (Optical) Mechanism Logger                  */

   E_CIL_MSC,      /* Primary Mirror Support Comms-In                       */
   E_CIL_MSN,      /* Primary Mirror Support Control Node                   */
   E_CIL_MSS,      /* Primary Mirror Support Comms-Out                      */
   E_CIL_MSR,      /* Primary Mirror Support SDB Reporting                  */
   E_CIL_MST,      /* Primary Mirror Support Test                           */
   E_CIL_MSL,      /* Primary Mirror Support Logger                         */

   E_CIL_NDC,      /* Nasmyth X+ (drive side) ACN Comms-In                  */
   E_CIL_NDN,      /* Nasmyth X+ (drive side) ACN Control Node              */
   E_CIL_NDS,      /* Nasmyth X+ (drive side) ACN Comms-Out                 */
   E_CIL_NDR,      /* Nasmyth X+ (drive side) ACN SDB Reporting             */
   E_CIL_NDT,      /* Nasmyth X+ (drive side) ACN Test                      */
   E_CIL_NDL,      /* Nasmyth X+ (drive side) ACN Logger                    */

   E_CIL_NWC,      /* Nasmyth X- (wrap side) ACN Comms-In                   */
   E_CIL_NWN,      /* Nasmyth X- (wrap side) ACN Control Node               */
   E_CIL_NWS,      /* Nasmyth X- (wrap side) ACN Comms-Out                  */
   E_CIL_NWR,      /* Nasmyth X- (wrap side) ACN SDB Reporting              */
   E_CIL_NWT,      /* Nasmyth X- (wrap side) ACN Test                       */
   E_CIL_NWL,      /* Nasmyth X- (wrap side) ACN Logger                     */

   E_CIL_RLC,      /* Generic Roll Axis Comms-In                            */
   E_CIL_RLN,      /* Generic Roll Axis Control Node                        */
   E_CIL_RLS,      /* Generic Roll Axis Comms-Out                           */
   E_CIL_RLR,      /* Generic Roll Axis SDB Reporting                       */
   E_CIL_RLT,      /* Generic Roll Axis Test                                */
   E_CIL_RLL,      /* Generic Roll Axis Logger                              */

   E_CIL_PTC,      /* Generic Pitch Axis Comms-In                           */
   E_CIL_PTN,      /* Generic Pitch Axis Control Node                       */
   E_CIL_PTS,      /* Generic Pitch Axis Comms-Out                          */
   E_CIL_PTR,      /* Generic Pitch Axis SDB Reporting                      */
   E_CIL_PTT,      /* Generic Pitch Axis Test                               */
   E_CIL_PTL,      /* Generic Pitch Axis Logger                             */

   E_CIL_DOC,      /* Dome Azimuth Comms-In                                 */
   E_CIL_DON,      /* Dome Azimuth Control Node                             */
   E_CIL_DOS,      /* Dome Azimuth Comms-Out                                */
   E_CIL_DOR,      /* Dome Azimuth SDB Reporting                            */
   E_CIL_DOT,      /* Dome Azimuth Test                                     */
   E_CIL_DOL,      /* Dome Azimuth Logger                                   */

   E_CIL_WOC,      /* Nasmyth X- Auxiliary Mechanisms Comms-In              */
   E_CIL_WON,      /* Nasmyth X- Auxiliary Mechanisms Control Node          */
   E_CIL_WOS,      /* Nasmyth X- Auxiliary Mechanisms Comms-Out             */
   E_CIL_WOR,      /* Nasmyth X- Auxiliary Mechanisms SDB Reporting         */
   E_CIL_WOT,      /* Nasmyth X- Auxiliary Mechanisms Test                  */
   E_CIL_WOL,      /* Nasmyth X- Auxiliary Mechanisms Logger                */

   /* Nasmyth X+ (drive side) autoguider */

   E_CIL_DGS,      /* Nasmyth X+ Autoguider system */
   E_CIL_DGP,      /* Nasmyth X+ Autoguider guide packets */
   E_CIL_DGG,      /* Nasmyth X+ Autoguider guide process */
   E_CIL_DGI,      /* Nasmyth X+ Autoguider graphical interface */

   /* Nasmyth X- (wrap side) autoguider */

   E_CIL_WGS,      /* Nasmyth X- Autoguider system */
   E_CIL_WGP,      /* Nasmyth X- Autoguider guide packets */
   E_CIL_WGG,      /* Nasmyth X- Autoguider guide process */
   E_CIL_WGI,      /* Nasmyth X- Autoguider graphical interface */

   /* Prime Focus autoguider */

   E_CIL_PGS,      /* Prime Focus Autoguider system */
   E_CIL_PGP,      /* Prime Focus Autoguider guide packets */
   E_CIL_PGG,      /* Prime Focus Autoguider guide process */
   E_CIL_PGI,      /* Prime Focus Autoguider graphical interface */

   /* Test units   */

   E_CIL_TU0,      /* Test Unit 0 (for testing/debugging only) */
   E_CIL_TU1,      /* Test Unit 1 (for testing/debugging only) */
   E_CIL_TU2,      /* Test Unit 2 (for testing/debugging only) */
   E_CIL_TU3,      /* Test Unit 3 (for testing/debugging only) */
   E_CIL_TU4,      /* Test Unit 4 (for testing/debugging only) */
   E_CIL_TU5,      /* Test Unit 5 (for testing/debugging only) */
   E_CIL_TU6,      /* Test Unit 6 (for testing/debugging only) */
   E_CIL_TU7,      /* Test Unit 7 (for testing/debugging only) */
   E_CIL_TU8,      /* Test Unit 8 (for testing/debugging only) */
   E_CIL_TU9,      /* Test Unit 9 (for testing/debugging only) */
   E_CIL_TES,      /* Test Echo Server (for testing only) */
   E_CIL_TMB,      /* Test Message Broker (for testing only) */
   E_CIL_IPT,      /* IUCAA Prototype TCS */
   E_CIL_TST,      /* Test Scripting Tool (for testing only) */

   /* Misc         */

   E_CIL_LOG,      /* System logging (syslogd) */

   /* DAT          */

   E_CIL_DA0,       /* Data Analysis Tool (session 0) */
   E_CIL_DA1,       /* Data Analysis Tool (session 1) */
   E_CIL_DA2,       /* Data Analysis Tool (session 2) */
   E_CIL_DA3,       /* Data Analysis Tool (session 3) */
   E_CIL_DA4,       /* Data Analysis Tool (session 4) */
   E_CIL_DA5,       /* Data Analysis Tool (session 5) */
   E_CIL_DA6,       /* Data Analysis Tool (session 6) */
   E_CIL_DA7,       /* Data Analysis Tool (session 7) */
   E_CIL_DA8,       /* Data Analysis Tool (session 8) */
   E_CIL_DA9,       /* Data Analysis Tool (session 9) */

   /* CMT          */

   E_CIL_CM0,       /* Computer Monitoring Task (session 0) */
   E_CIL_CM1,       /* Computer Monitoring Task (session 1) */
   E_CIL_CM2,       /* Computer Monitoring Task (session 2) */
   E_CIL_CM3,       /* Computer Monitoring Task (session 3) */
   E_CIL_CM4,       /* Computer Monitoring Task (session 4) */
   E_CIL_CM5,       /* Computer Monitoring Task (session 5) */
   E_CIL_CM6,       /* Computer Monitoring Task (session 6) */
   E_CIL_CM7,       /* Computer Monitoring Task (session 7) */
   E_CIL_CM8,       /* Computer Monitoring Task (session 8) */
   E_CIL_CM9,       /* Computer Monitoring Task (session 9) */

   /* CCT          */

   E_CIL_CC0,       /* Computer Control Task (session 0) */
   E_CIL_CC1,       /* Computer Control Task (session 1) */
   E_CIL_CC2,       /* Computer Control Task (session 2) */
   E_CIL_CC3,       /* Computer Control Task (session 3) */
   E_CIL_CC4,       /* Computer Control Task (session 4) */
   E_CIL_CC5,       /* Computer Control Task (session 5) */
   E_CIL_CC6,       /* Computer Control Task (session 6) */
   E_CIL_CC7,       /* Computer Control Task (session 7) */
   E_CIL_CC8,       /* Computer Control Task (session 8) */
   E_CIL_CC9,       /* Computer Control Task (session 9) */

   E_CIL_EOL       /* End of list marker (DO NOT USE FOR A TASK ID) */
} eCilNames_t;


/*
** Returned Status Values
*/

enum eCilStatus {
   E_CIL_GEN_ERR = STATUS_START(CIL), /* Miscellaneous error */
   E_CIL_NO_SETUP,      /* eCilSetup has not been called. Function can't work */
   E_CIL_ALREADY_SETUP, /* eCilSetup() has already been called */
   E_CIL_MAP_NOT_FOUND, /* Unable to find the specified CIL Mapfile */
   E_CIL_NO_MAP_GIVEN,  /* No map filename was provided */
   E_CIL_MAP_ERROR,     /* An error occured while reading the map file */
   E_CIL_MAP_TOO_SMALL, /* Insufficient entries in map file for comms */
   E_CIL_UNKNOWN_NAME,  /* Specified name not found in map */
   E_CIL_NO_NAME,       /* No CIL name was specified */
   E_CIL_SEND_ERR,      /* Failed to send UDP packet */
   E_CIL_RECV_ERR,      /* Failed to receive UDP packet */
   E_CIL_TIMEOUT,       /* No UDP packet received in the specified time */
   E_CIL_ID_INVALID,    /* Specified CIL ID is not recognised */
   E_CIL_RXID_UNKNOWN,  /* Message received from unrecognised source */
   E_CIL_NEW_DEST_FAIL, /* Can't create a CIL destination ID in linked list */
   E_CIL_SOCKET_FAIL,   /* Unable to create a local socket */
   E_CIL_PARAM_ERR,     /* Misc problem with function argument */
   E_CIL_NULL_MSG,      /* NULL pointer to a message was passed to a function */
   E_CIL_NULL_RXID,     /* NULL pointer to a RxID was passed to a function */
   E_CIL_NULL_DATA,     /* NULL pointer to UDP data */
   E_CIL_NO_MEMORY,     /* Couldn't allocate some dynamic memory */
   E_CIL_NO_STORAGE,    /* No storage space has been left to receive message */
   E_CIL_BAD_TIMEOUT,   /* Incorrectly specified timeout (e.g. negative) */
   E_CIL_NEG_DATALEN,   /* The size of a data buffer is specified as negative */
   E_CIL_ADDRINUSE,     /* IP/port address already in use */
   E_CIL_ADDRNOTAVAIL,  /* The specified IP address is not available for use */
   E_CIL_FLUSH_ERROR,   /* Receive message saturation during a flush */
   E_CIL_MAP_DENIED,    /* NOT USED */
   E_CIL_ID_IN_USE,     /* NOT USED */
   E_CIL_NO_DATA,       /* NOT USED */
   E_CIL_RXID_INVALID,  /* NOT USED */
   E_CIL_TXID_UNKNOWN,  /* NOT USED */
   E_CIL_RX_ADDR_ERR    /* Message received from invalid address */

};

/*
** Enumeration of message class
*/

enum eCilClass {
  E_CIL_INV_CLASS = 0,   /* Invalid class */
  E_CIL_CMD_CLASS,       /* Command */
  E_CIL_RSP_CLASS,       /* Response */
  E_CIL_ACK_CLASS,       /* Acknowledgement */
  E_CIL_ACT_CLASS,       /* Command received and is being acted upon */
  E_CIL_COM_CLASS,       /* Action has been completed */
                         /*    (follows E_CIL_ACT_CLASS) */
  E_CIL_ERR_CLASS        /* Error */
};


/*
** Strings returned by 'eCilName'
*/

#define E_CIL_STR_INVALID  "???"  /* Out of range CIL identifier */
#define E_CIL_STR_NOSETUP  "---"  /* CilSetup has not been performed */
#define E_CIL_STR_BOL      ">>>"  /* Beginning of CIL list */
#define E_CIL_STR_EOL      "<<<"  /* End of CIL list */


/*
** Definitions for error checking.
*/

#define E_CIL_INVALID      E_CIL_BOL


/*
** Predefined limits
*/

#define E_CIL_IDLEN        16 /* Maximum length of a CIL ID string */


/*
** Structure for packaging up CIL messages
*/

typedef struct eCilMsg_s
{
   Int32_t    SourceId;  /* Address code for source */
   Int32_t    DestId;    /* Address code for the destination */
   Int32_t    Class;     /* Qualifier on component of service transaction */
   Int32_t    Service;   /* Defines transaction service being used */
   Uint32_t   SeqNum;    /* Sequence number used by client */
   eTtlTime_t TimeStamp; /* Absolute time associated with the CIL message */
   void       *DataPtr;  /* Pointer to memory containing (space for) data */
   size_t     DataLen;   /* Number of bytes allocated for the data field */
} eCilMsg_t;


/*
** Declarations for externally available Functions.
*/

Status_t eCilSetup(const char *MapFileNamePtr, Int32_t MyId);
Status_t eCilLookup(const char *MapFileNamePtr, const char *CilNamePtr,
                    Int32_t *IdPtr);
Status_t eCilName(const char *MapFileNamePtr, Int32_t CilId, 
                  size_t CilNameLen, char *CilNamePtr);
Status_t eCilSend(Int32_t TxId, eCilMsg_t *MsgPtr);
Status_t eCilRawSend(Int32_t DestId, size_t DataLen, char *DataPtr);
Status_t eCilReceive(int TImeout, Int32_t *RxIdPtr, eCilMsg_t *MsgPtr);
Status_t eCilConvert32bitArray(size_t NumBytes, void *Array);
Status_t eCilFlush( void );
char *   eCilNameString( Int32_t CilId );
Int32_t  eCilId( void );
Bool_t   eCilAlreadySetup( void );

Status_t eCilSendDynamic     ( Int32_t TxId, eCilMsg_t * MsgPtr );
Status_t eCilReceiveDynamic  ( int Timeout, Int32_t *RxIdPtr, eCilMsg_t *MsgPtr );
Int32_t  eCilGetIdDynamic    ( Int32_t DynamicId );


#endif /* CIL_H_DEFINED */


/* EOF */
