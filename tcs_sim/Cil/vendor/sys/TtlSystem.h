/*
**       Copyright (c) Telescope Technologies Limited, 2000-2003
**
** Header File Name:
**    TtlSystem.h
**
** Purpose:
**    TTL globals definitions file, all environments.
**
** Description:
**    Contains general purpose definitions, macros and types for
**    all TTL software. This file is platform independant but the
**    user needs to define the appropriate macros for their
**    environment if they are not already available.
**
** Authors:
**    plw: Paul L. Wright (TTL)
**    djm: Derek J. McKay (TTL)
**    mjf: Martyn J. Ford (TTL)
**    asp: Andrzej S. Piascik (TTL)
**    ljs: Lorna J. Slater (TTL)
**    rdl: Robert D. Lett (TTL)
**    jdm: Jon D. Mills (TTL)
**    mkv: Mark K. Vernon (TTL)
**    pxh: Peter Hall (TTL)
**
** Version:
**    $Id: TtlSystem.h,v 1.89 2007/09/20 10:05:23 mjf Exp $
**
** History:
**    $Log: TtlSystem.h,v $
**    Revision 1.89  2007/09/20 10:05:23  mjf
**    Addition of package ID for SMU.
**
**    Revision 1.88  2007/07/17 15:00:51  mjf
**    Create new process states enumeration purely for lookup table generation.
**
**    Revision 1.87  2004/06/29 16:18:32  mjf
**    Addition of Fummy Heartbeat Task (DHT).
**
**    Revision 1.86  2004/05/06 10:56:31  mjf
**    Addition of WFL, SPI and QTS packages.
**
**    Revision 1.85  2004/04/21 16:36:23  mjf
**    Addition of Yunnan.
**
**    Revision 1.84  2003/06/12 13:33:14  mjf
**    Addition of PPT (Photon Porting Tool) package ID.
**
**    Revision 1.83  2003/02/04 11:48:22  mjf
**    Addition of some package IDs.
**
**    Revision 1.82  2002/10/15 13:08:22  mjf
**    Addition of macros to retrieve package IDs from a status code,
**    service number and OID number.
**
**    Revision 1.81  2002/07/25 09:45:44  mjf
**    Addition of package IDs for MPU and MBT.
**
**    Revision 1.80  2002/05/28 14:05:18  man
**    Included package ID for Aci (Autoguider Camera Interface)
**    used to test the Agg and Ags.
**
**    Revision 1.79  2002/01/18 09:29:59  sxf
**    Added package ID for AGA (Autoguider Artificial images and the
**    code to generate them).
**
**    Revision 1.78  2002/01/07 11:12:21  mjf
**    Addition of dummy enumeration for individual telescope definitions.
**    This is only to allow the use of HTI name lookups using the HTI library.
**
**    Revision 1.77  2002/01/07 10:40:27  mjf
**    Addition of telescope definitions and telescope groups using a bit
**    field. Comment updated for STANDBY and SUSPEND states.
**
**    Revision 1.76  2001/12/11 14:52:17  rdl
**    Added package ID for Axis Motion Test tool (AMT).
**
**    Revision 1.75  2001/10/29 15:08:20  djm
**    Added package code for SSE.
**
**    Revision 1.74  2001/10/15 11:16:44  djm
**    Replaced IRF package ID with AFC (Autoguider Filter Control).
**
**    Revision 1.73  2001/10/09 10:43:05  mkb
**    Changed TTM (Truss Temperature Monitoring) package id to
**    MBS (ModBus Server) to reflect generic nature of package.
**
**    Revision 1.72  2001/10/03 07:14:50  mjf
**    Addition of MLP package ID to configuration-manage MicroLynx programs.
**
**    Revision 1.71  2001/09/25 11:23:25  mjf
**    Removal of TFL package ID (now part of TFP) - replaced with MBL for
**    prospective MODBUS interface library.
**
**    Revision 1.70  2001/09/24 11:08:06  mjf
**    Addition of 'SUSPEND' state into the process state enumeration.
**
**    Revision 1.69  2001/09/24 09:13:26  mjf
**    Addition of Truss Temperature Monitoring package ID.
**
**    Revision 1.68  2001/09/19 11:04:49  mjf
**    Addition of HTG package.
**
**    Revision 1.67  2001/09/19 10:36:17  mjf
**    Addition of package IDs for ASI, TFL and AML.
**
**    Revision 1.66  2001/09/14 10:31:07  mjf
**    Moving of node sub-system definitions to file Acn.h.
**
**    Revision 1.65  2001/08/31 11:45:09  mjf
**    Addition of package ID for TSF - Test Script Files.
**
**    Revision 1.64  2001/06/15 14:46:00  mjf
**    Addition of new package IDs HNF and PID. Comments for generic SDB datum
**    slots updated to include the units and enum names.
**
**    Revision 1.63  2001/05/18 13:53:58  mjf
**    Addition of package ID for TDL - TTL development library.
**
**    Revision 1.62  2001/05/16 09:28:52  djm
**    Added MML and IET package IDs.
**
**    Revision 1.61  2001/04/24 09:13:51  mjf
**    Correction to comments for process states.
**
**    Revision 1.60  2001/04/23 09:43:13  djm
**    Added package IDs for OHG, TMU and CBI.
**
**    Revision 1.59  2001/03/26 14:46:51  mjf
**    Addition of package IDs for GSC and PMS, and addition command-line
**    switch for the mirror interface.
**
**    Revision 1.58  2001/03/19 19:09:52  mjf
**    Addition of definitions of node names to be used as command-line
**    arguments.
**
**    Revision 1.57  2001/03/07 14:18:26  rdl
**    Amendend Package Ids for Auxiliary Mechanism Node
**    Communications In and Communications Out from ACI to OCI and
**    ACO to OCO respectively.
**
**    Revision 1.56  2001/03/05 16:45:03  pxh
**    ��Addition of Package Id. AMS.
**
**    Revision 1.55  2001/03/02 09:24:30  mjf
**    Addition of package ID for Computer Control Task (CCT). Also a SYS_EOL
**    (end-of-list) marker added for package IDs.
**
**    Revision 1.54  2001/03/01 09:07:46  mjf
**    Addition of new package IDs for future autoguider development.
**
**    Revision 1.53  2001/02/28 18:33:44  rdl
**    Added new package ids :
**    OID - OidServer for Azi, Alt & Cas Axis Control Nodes.
**    ACI - AMN Communications In Task.
**    ACO - AMN Communications Out Task.
**
**    Revision 1.52  2001/02/27 13:38:30  mjf
**    Definitions for SDB units TRUEFALSE, ONOFF, YESNO assigned to the
**    existing definitions for TRUE and FALSE.
**
**    Revision 1.51  2001/02/27 12:14:43  djm
**    Added AES & AEI package IDs.
**
**    Revision 1.50  2001/02/23 16:07:57  mjf
**    Addition of enumerations for SDB units TRUE/FALSE, ON/OFF, YES/NO.
**
**    Revision 1.49  2001/02/20 15:39:58  mjf
**    Addition of package IDs for QMS, STl, STD and STI.
**
**    Revision 1.48  2001/01/25 09:04:59  mjf
**    Addition of package IDs for TQB, DF1, ETC, PMM, TQW, OEM.
**
**    Revision 1.47  2001/01/02 12:08:37  mjf
**    Addition of package ID for Header2Id utility.
**
**    Revision 1.46  2000/12/11 18:53:50  mjf
**    In set command structure, make units of type 'Int32_t'.
**
**    Revision 1.45  2000/12/11 18:17:48  mjf
**    Addition of package IDs for AMN. Addition of generic datum ID for
**    application version. Moving of some generic service numbers from Mcp.h,
**    and addition of generic service numbers for SET and GET commands.
**    Structure type for format of SET command added.
**
**    Revision 1.44  2000/12/09 15:08:45  mjf
**    Addition of SPT and TST packages, and removal of TABs.
**
**    Revision 1.43  2000/12/06 18:09:06  rdl
**    Addition of Packageid for :
**    a). NCI - Node Communications In Task for ACN & AMN nodes.
**    b). NCO - Node Communications Out Task for ACN & AMN nodes.
**
**    Revision 1.42  2000/12/03 10:38:17  rdl
**    Addition of Packageid (HBA) for Heart Beat Agent used within
**    Axis Control Nodes and Auxiliary Mechanism Node.
**
**    Revision 1.41  2000/11/29 14:12:51  mkv
**    Added MIR for Mirror Cell Controller.
**
**    Revision 1.40  2000/11/08 09:42:22  mjf
**    Addition of Axis Stop Controller and Axis Movement Controller.
**
**    Revision 1.39  2000/11/07 14:20:23  mjf
**    Addition of MCP datums for generic SDB submissions. Previously in Mcp.h.
**
**    Revision 1.38  2000/11/07 14:15:15  mjf
**    Include <stdlib.h> to use malloc, free etc.
**
**    Revision 1.37  2000/11/07 14:13:02  mjf
**    Addition of package NMC for Node Motion Controller.
**
**    Revision 1.36  2000/10/31 14:50:53  jdm
**    NSC package identifier added for Node
**    State Controller.
**
**    Revision 1.35  2000/10/20 11:53:54  djm
**    Added WMS, SIF, INC and CAN package IDs. Removed EWT ID.
**
**    Revision 1.34  2000/10/19 14:02:02  mkv
**    AET PackageId added for Absolute Encoder Task.
**
**    Revision 1.33  2000/10/11 12:44:57  mkv
**    AGN PackageID added for Agent Node.
**
**    Revision 1.32  2000/10/10 07:57:56  mjf
**    Removal of package ID DAQ - replaced in list with CMT (Computer
**    Monitoring Task).
**
**    Revision 1.31  2000/10/02 12:34:44  jdm
**    VEN package ID added for Virtual Encoder package.
**
**    Revision 1.30  2000/09/27 09:15:25  jdm
**    Package IDs REP and CFL added. OID_START macro
**    added to assist allocation of OID numbers.
**
**    Revision 1.29  2000/09/26 15:20:43  djm
**    Added EPT package ID.
**
**    Revision 1.28  2000/09/26 11:10:12  rdl
**    Additional package identifiers added (AGT, TMF & TNF).
**
**    Revision 1.27  2000/09/20 13:30:17  jdm
**    DIF changed into MIF (Motor Inter-Face).
**
**    Revision 1.26  2000/09/20 10:36:29  ljs
**    Added ERT and EWT package IDs. Also, added package ID comments.
**
**    Revision 1.25  2000/09/19 07:53:20  jdm
**    DIF error code added for Digital Inter-Face server.
**
**    Revision 1.24  2000/09/19 07:33:19  mjf
**    Addition of copyright.
**
**    Revision 1.23  2000/09/11 15:57:41  asp
**    Added TTL_CALLOC & TTL_REALLOC wrappers
**
**    Revision 1.22  2000/09/07 15:14:16  mjf
**    Addition of package IDs for UIT, AIT and MIT to interface to UPS, APS,
**    and modem.
**
**    Revision 1.21  2000/08/25 13:09:21  mjf
**    Implementation of both status and state types as type Int32_t.
**    Also some general tidying up.
**
**    Revision 1.20  2000/08/22 17:57:19  djm
**    Added new package code (CFU).
**
**    Revision 1.19  2000/08/07 14:42:17  djm
**    Added package code for the SDB File Retrival (SFR) task.
**
**    Revision 1.18  2000/07/21 11:05:05  asp
**    TRUE & FALSE changed from enumerated type to int. typedef Bool_t added
**
**    Revision 1.17  2000/07/14 16:05:31  djm
**    Added status code offsets for some new packages.
**
**    Revision 1.16  2000/07/14 10:09:22  djm
**    Minor change to allow port to Linux/GNU-C.
**
**    Revision 1.15  2000/07/05 10:32:42  djm
**    Added SERVICE_START macro for determining unique service codes
**    within CIL messages.
**
**    Revision 1.14  2000/06/02 16:01:07  djm
**    Added LOG ID for error codes, etc..
**
**    Revision 1.13  2000/05/26 08:03:51  djm
**    Added TIM package to enumerated list.
**
**    Revision 1.12  2000/05/12 08:21:22  djm
**    Slight modification to allow compilation unadjusted under VMS.
**
**    Revision 1.11  2000/05/08 12:56:45  djm
**    Added eTtlTime_t type.
**
**    Revision 1.10  2000/05/05 10:32:55  djm
**    Added addition error code offsets.
**
**    Revision 1.9  2000/04/07 09:51:48  djm
**    Added SYS_INVALID_STATE into the ProcState_t enumerated type.
**
**    Revision 1.8  2000/04/05 16:41:04  djm
**    Fixed up some of the comments to conform to the coding standard.
**    Modified some of the TtlProcState enumerated list items.
**
**    07.05.99 First draft for QNX environment. PLW
**    Added package/status range assignment Feb 2000. PLW
*/


#ifndef TTLSYSTEM_H_DEFINED
#define TTLSYSTEM_H_DEFINED

/*
** Include files
*/

/* To use malloc, free etc. */

#include <stdlib.h>


/*
** VMS predefines TRUE and FALSE in stdio.h. The definition in Bool_t 
** is consistent with the VMS defintion, but replaces it.
*/

#ifdef __VMS
#undef TRUE
#undef FALSE
#endif

typedef int Bool_t;

#ifndef FALSE
#define FALSE    (Bool_t)0
#endif

#ifndef TRUE
#define TRUE     (Bool_t)1
#endif


/*
** Standard type defintions
*/

#ifdef __QNX__

#ifdef __WATCOMC__

typedef signed char          Int8_t;
typedef unsigned char        Uint8_t;
typedef short int            Int16_t;
typedef unsigned short int   Uint16_t;
typedef int                  Int32_t;
typedef unsigned int         Uint32_t;

#endif /* __WATCOMC__ */

#endif /* __QNX__ */


#ifdef __VMS

typedef signed char          Int8_t;
typedef unsigned char        Uint8_t;
typedef short int            Int16_t;
typedef unsigned short int   Uint16_t;
typedef int                  Int32_t;
typedef unsigned int         Uint32_t;

#endif /* __VMS */


/* #ifdef LINUX */

#ifdef __GNUC__

typedef signed char          Int8_t;
typedef unsigned char        Uint8_t;
typedef short int            Int16_t;
typedef unsigned short int   Uint16_t;
typedef int                  Int32_t;
typedef unsigned int         Uint32_t;

#endif /* __GNUC__ */

/* #endif */ /* LINUX */


/*
** Package number assignments
*/

typedef enum eSysPackageId_e
{
   SYS = 0x0001,    /* Generic system messages */
   CIL = 0x0002,    /* Communications Interface Library */ 
   ACN = 0x0003,    /* Generic Axis Control Node Package */
   SHM = 0x0004,    /* Shared memory library */
   SCL = 0x0005,    /* Serial Communications Library */
   TFP = 0x0006,    /* Time and Frequency Process (IRIG-B) */
   HHE = 0x0007,    /* Heidenhain Encoder package */ 
   CMT = 0x0008,    /* Computer Monitoring Task */
   TCS = 0x0009,    /* Telescope Control System */
   MCB = 0x000a,    /* Master Command Broker */
   MCP = 0x000b,    /* Master Control Process */
   CHB = 0x000c,    /* Continuous Heartbeats */
   SDB = 0x000d,    /* Status Database */
   TIM = 0x000e,    /* Time handling functions */
   LOG = 0x000f,    /* Message reporting library (Syslog, Console, etc.) */
   CLU = 0x0010,    /* Command Line (parsing) Utilities */
   ECI = 0x0011,    /* Engineering Control Interface */
   DAT = 0x0012,    /* Data Analysis Tool */
   SFR = 0x0013,    /* SDB File Retrieval process */
   CFU = 0x0014,    /* Configuration File Utilities */
   UIT = 0x0015,    /* UPS Interface Task */
   AIT = 0x0016,    /* Addressable power switch Interface Task */
   MIT = 0x0017,    /* (Callout) Modem Interface Task */
   MIF = 0x0018,    /* Motor Inter-Face server */
   ERT = 0x0019,    /* Engineering interface Reporting Task */
   WMS = 0x001a,    /* Weather Monitoring System */
   AGT = 0x001b,    /* Agent-Server function Library */
   TMF = 0x001c,    /* TTL Wrapper functions for QNX messaging functions */
   TNF = 0x001d,    /* TTL Wrapper functions for QNX Process Name functions */
   EPT = 0x001e,    /* Enclosure PLC Task */
   REP = 0x001f,    /* Node SDB Reporter task */
   CFL = 0x0020,    /* Node Controller Fast Logger task */   
   VEN = 0x0021,    /* Virtual Encoder task */
   AGN = 0x0022,    /* Agent Node Task */
   AET = 0x0023,    /* Absolute Encoder Task */
   INC = 0x0024,    /* Inclinometer monitor */
   CAN = 0x0025,    /* CAN bus controller */
   SIF = 0x0026,    /* Primary Mirror Support Interface */
   NSC = 0x0027,    /* Node State Controller */
   NMC = 0x0028,    /* Node Motion Controller */
   ASC = 0x0029,    /* Axis Stop Controller */
   AMC = 0x002a,    /* Axis Motion Controller */
   MIR = 0x002b,    /* Mirror Cell Controller */
   HBA = 0x002c,    /* Heart Beat Agent for ACN & AMN Nodes */
   NCI = 0x002d,    /* Node Communications In Task */
   NCO = 0x002e,    /* Node Communications Out Task */
   SPT = 0x002f,    /* Services PLC Task */
   TST = 0x0030,    /* Test Script Parsing Tool */
   AGD = 0x0031,    /* Autoguider Deployment */
   AGF = 0x0032,    /* Autoguider Focus */
   SFD = 0x0033,    /* Science Fold Deployment */
   SFP = 0x0034,    /* Science Fold Position */
   SMF = 0x0035,    /* Secondary Mirror Focus */
   ACM = 0x0036,    /* ACADS Mechanisms */
   TMS = 0x0037,    /* Temperature Monitoring System */
   AFC = 0x0038,    /* Autoguider Filter Control */
   DSL = 0x0039,    /* Dark Slide */
   AMN = 0x003a,    /* Auxiliary Mechanism Node */
   CBS = 0x003b,    /* Can Bus Server (will supersede CAN package id later) */
   HTI = 0x003c,    /* Header To ID utility */
   TQB = 0x003d,    /* TTL QNX Build Files */
   DF1 = 0x003e,    /* Allen-Bradley DF1 Protocol Library */
   ETC = 0x003f,    /* Configuration Files */
   PMM = 0x0040,    /* Process Management Module */
   TQW = 0x0041,    /* TTL QNX Wrapper Library */
   OEM = 0x0042,    /* Externally generated drivers */
   QMS = 0x0043,    /* QNX Monitoring System (sysmon) */
   STL = 0x0044,    /* SDB 'testlist' utility */
   STD = 0x0045,    /* SDB 'testdump' utility */
   STI = 0x0046,    /* SDB 'testinject' utility */
   AES = 0x0047,    /* Absolute Encoder Server */
   AEI = 0x0048,    /* Absolute Encoder Interface */
   OID = 0x0049,    /* OID Server for Nodes */
   OCI = 0x004a,    /* Node Communications Task In  - AMN */
   OCO = 0x004b,    /* Node Communications Task Out - AMN */
   AGI = 0x004c,    /* Autoguider User Interface */
   AGC = 0x004d,    /* Autoguider Camera Driver Task */
   AGG = 0x004e,    /* Autoguider Guiding Task */
   AGS = 0x004f,    /* Autoguider System Task */
   CCT = 0x0050,    /* Computer Control Task */
   AMS = 0x0051,    /* Auxiliary Mechanism Node State Control Task */
   GSC = 0x0052,    /* Generic State Controller (OIDs for NSC, AMS, PMS) */
   PMS = 0x0053,    /* Primary Mirror State Controller */
   CBI = 0x0054,    /* CAN-bus interface */
   TMU = 0x0055,    /* Tape Modelling Utility */
   OHG = 0x0056,    /* Operational-Hours Generator */ 
   IET = 0x0057,    /* Interface to rotating Enclosure Task */
   MML = 0x0058,    /* Matrix Manipulation Library */
   TDL = 0x0059,    /* Telescope Development Library */
   HNF = 0x005a,    /* H-iNFinity servo controller */
   PID = 0x005b,    /* Proportional-Integral-Derivative servo controller */
   TSF = 0x005c,    /* Test Script Tool Files */
   ASI = 0x005d,    /* Node Agent/Server Interface */
   MBL = 0x005e,    /* MODBUS Interface Library */
   AML = 0x005f,    /* Axis Motion Controller Logger */
   HTG = 0x0060,    /* HTI Tables Generation */
   MBS = 0x0061,    /* ModBus Server */
   MLP = 0x0062,    /* MicroLynx Programs */
   SSE = 0x0063,    /* SDB String Encoding library */
   AMT = 0x0064,    /* Axis Motion Tool */
   AGA = 0x0065,    /* Autoguider Artificial Images (images and generation) */
   ACI = 0x0066,    /* utoguider Camera Interface (to test AGC and AGG) */
   MPU = 0x0067,    /* MicroLynx Programming Utility */
   MBT = 0x0068,    /* Multi-function Board Test utility */
   HCE = 0x0069,    /* Hipparcos Catalogue Extraction */
   WDT = 0x006a,    /* WatchDog Test utility */
   WMP = 0x006b,    /* Weather Monitoring Processor */
   HTR = 0x006c,    /* HTI Report utility */
   PTC = 0x006d,    /* AGC Photon Transfer Curve utility */
   PPT = 0x006e,    /* Photon Porting Tool */
   WFL = 0x006f,    /* Wrapper Function Library */
   SPI = 0x0070,    /* Services PLC Interface to T2-408 PLC */
   QTS = 0x0071,    /* QNX Tick Size utility */
   DHT = 0x0072,    /* Dummy Heartbeat Task */
   SMU = 0x0073,    /* SDB MySQL Utilities */
   EIP = 0x0074,    /* Wrapper and test utility for tuxplc based Ethernet IP library */
   BCI = 0x0075,    /* Boltwood Cloud Interface */
   MCH = 0x0076,    /* Mount Communications Handler */
   MTI = 0x0077,    /* Mount Time Interface */
   MTA = 0x0078,    /* Mount Telemetry Agent */
   MCA = 0x0079,    /* Motion Control of Axis */
   MCL = 0x007a,    /* Motion Control Logger */
   MIL = 0x007b,    /* Mount Interface Library */
   EIL = 0x007c,    /* Encoder Interface Library */
   DIL = 0x007d,    /* Drive Interface Library */
   WIB = 0x007e,    /* Watchdog Interlocks and Brakes Library */

   /* Start instances of hardware-specific implementations higher up */   
   S300      = 0x0101,  /* Danaher S300 Drives Interface */
   ACCES     = 0x0102,  /* ACCES Digital I/O Interface */
   IPT       = 0x0103,  /* Interface for Paddle and Telemetry */

   EILS300   = 0x0104,  /* Encoder Interface to Servostar 300 Drives */
   DILS300   = 0x0105,  /* Drive Interface to Servostar 300 Drives */
   WIBS300   = 0x0106,  /* Interlocks Interface for Servostar 300 Drives */

   EILKUB    = 0x0107,  /* Encoder Interface to Kubler 9080 */
   DILYAS    = 0x0108,  /* Drive Interface to Yaskawa V1000 */
   WIBTURCK  = 0x0109,  /* Interlocks Interface to TURCK DeviceNet */

   WIBSHMEM  = 0x010a,  /* Interlocks Interface via shared memory */

   EILHHE    = 0x010b,  /* Encoder Interface to Heidenhain IK220 */
   DILPDAQ   = 0x010c,  /* Drive Interface to PowerDAQ */
   WIBPDAQ   = 0x010d,  /* Interlocks Interface via PowerDAQ */
   MTIIRIGB  = 0x010e,  /* Timing Interface via IRIG-B card */

   SYS_EOL          /* End-of-list marker - insert additions before this */ 

} eSysPackageId_t;


/*
** Status return range assignment macro.
** All status returns are enumerations/integers which are 4 byte values.
** These are assigned as follows:
** Most significant 16 bits package id.
** Least significant 16 bits return value.
*/

#define STATUS_START(x) ((x) << 16)


/*
** Services range assignment macro.
** All services are enumerations/integers which are 4 byte values.
** These are assigned as follows:
** Most significant 16 bits - package ID.
** Least significant 16 bits - service within the package.
*/

#define SERVICE_START(x) ((x) << 16)


/*
** OID number assignment macro.
** All OIDs are enumerations/integers which are 4 byte values.
** These are assigned as follows:
** Most significant 16 bits - package ID.
** Least significant 16 bits - OID number within the package.
*/

#define OID_START(x) ((x) << 16)


/*
** Package ID retrieval macros.
** This suite of macros retrieves the package ID from identifiers 
** constructed using the STATUS_START, SERVICE_START and OID_START
** macros above. These macros are named accordingly with '_PACKAGE'
** replacing '_START' in each case.
*/

#define STATUS_PACKAGE(x)    ((x) >> 16)
#define SERVICE_PACKAGE(x)   ((x) >> 16)
#define OID_PACKAGE(x)       ((x) >> 16)



/* 
** System definitions used for exchanging data between packages, and for 
** submission into the SDB with units E_SDB_TRUEFALSE_UNITS, E_SDB_ONOFF_UNITS
** and E_SDB_YESNO_UNITS.
*/

#define E_SYS_TRUE           TRUE
#define E_SYS_FALSE          FALSE

#define E_SYS_ON             TRUE
#define E_SYS_OFF            FALSE
                             
#define E_SYS_YES            TRUE
#define E_SYS_NO             FALSE


/*
** Definitions of telescopes. These definitions may be used to enable some 
** software processes to operate differently depending on the applicable
** telescope. The telescope definitions are implemented using a bit-field, 
** and groups of telescopes may be contsructed by bitwise OR-ing the 
** individual telescopes.
** Note : to cater for the event of more than 32 telescopes, it is intended 
** that at this point the MSB will be used to indicate an additional word is 
** used to define the telescope. At this point it will probably be necessary
** to create some functions to manipulate the words.
*/

/* Individual telescopes. */

#define E_SYS_LIVERPOOL      ( 1 <<  0 )    /* Liverpool Telescope.   */ 
#define E_SYS_IUCAA          ( 1 <<  1 )    /* IUCAA Telescope.       */
#define E_SYS_FAULKES_I      ( 1 <<  2 )    /* Faulkes I Telescope.   */
#define E_SYS_FAULKES_II     ( 1 <<  3 )    /* Faulkes II Telescope.  */
#define E_SYS_YUNNAN         ( 1 <<  4 )    /* Yunnan Telescope.      */

/* Continuation bit for additional words. */

#define E_SYS_CONTINUATION   ( 1 << 31 ) 

/* Groups of telescopes. */

#define E_SYS_NONE           ( 0 )          /* Identifier for no telescopes.  */ 
#define E_SYS_FAULKES        ( E_SYS_FAULKES_I | E_SYS_FAULKES_II )
#define E_SYS_GENERIC        ( E_SYS_LIVERPOOL | E_SYS_FAULKES )
#define E_SYS_ALL            0xFFFFFFFF     /* Identifier for all telescopes. */ 

/*
** Enumeration of telescopes - this is only to allow telescope labels to be 
** looked up using the HTI library. It is necessary for all the individual 
** telescopes (but not groups) to be in this enumerated list.
** Note : these definitions use the package ID MCP - this is only a dummy to
** enable compilation. During the HTI table generation this will be stripped
** off anyway.
*/

typedef enum eSysTelescope_e 
{
   E_MCP_LIVERPOOL  = E_SYS_LIVERPOOL,
   E_MCP_IUCAA      = E_SYS_IUCAA,
   E_MCP_FAULKES_I  = E_SYS_FAULKES_I,
   E_MCP_FAULKES_II = E_SYS_FAULKES_II,
   E_MCP_YUNNAN     = E_SYS_YUNNAN,     

   E_MCP_NONE       = E_SYS_NONE
} eSysTelescope_t;


/*
** Generic status enumeration. Used for test programs, etc. that do not
** have their own package IDs.
*/

enum eSysStatus_e {
   SYS_NOMINAL = STATUS_START(SYS), /* Expected successful completion status */
   SYS_WARNING,                     /* Not an error, but a potential cause */
   SYS_ERROR_NON_FATAL,             /* Operation failure that it recoverable */
   SYS_ERROR_FATAL                  /* Non-recoverable error */
};

typedef Int32_t Status_t;


/*
** Process state definitions.
** The various processes of the telescope system maintain a concept of
** their own "state". This is used internally, but is also reported to
** other parts of the system. In order to provide a common form to
** specify these states, the following enumerated list is provided.
** Because these states are part of the interprocess communications,
** the list should only have additional states added to the end of it,
** and not inserted in the middle.
** Note also that these are process states and not other states (e.g.
** mechanisms). They are to be used to determine if a process is
** operating, rather than how a system is functioning, for the
** purposes of MCP control of software recovery.
** The SYS_OFF_STATE and SYS_TIMEOUT_STATE are not set by the process, 
** but may be set by the MCP. These are inferred states. 
** The SYS_INVALID_STATE is never explicitly set. It is to prevent an 
** uninitialised state variable from having a valid state on creation.
** All states must be explicitly set, and this guards against unset cases.
*/

enum ProcState_e {
   SYS_INVALID_STATE = 0,       /* No state has been set (uninitialised). */
   SYS_OKAY_STATE,     /* The process is operating normally.              */
   SYS_INIT_STATE,     /* The process is performing its initial start-up  */
                       /* or awaiting initialisation commands to take the */
                       /* process to SYS_OKAY_STATE (if appropriate).     */
   SYS_STANDBY_STATE,  /* This state is used for a process or the system  */
                       /* when software is running, but not yet ready for */
                       /* full operation. To move from this state to OKAY */
                       /* will require some action to be taken. For       */
                       /* example, axis software will attain this state   */
                       /* until the axis is homed.                        */
   SYS_WARN_STATE,     /* A problem has occurred, but no automatic        */
                       /* intervention is required from the MCP. The      */
                       /* telescope may still be used in this state, but  */
                       /* there is a possibility that operational         */
                       /* performance may be degraded.                    */
   SYS_FAILED_STATE,   /* A problem has occurred with the process that    */
                       /* requires intervention by the MCP.               */
   SYS_SAFE_STATE,     /* The process has ceased normal operation (maybe  */
                       /* only temporarily) and is either about to        */
                       /* terminate itself or be able to be terminated by */
                       /* the MCP/system without risk of hardware damage  */
                       /* or serious data loss.                           */
   SYS_OFF_STATE,      /* The process is not running. THIS IS AN INFERRED */
                       /* STATE which is set by the MCP only.             */
   SYS_TIMEOUT_STATE,  /* The process is not responding to the MCP heart- */
                       /* beat messages. THIS IS AN INFERRED STATE which  */
                       /* is set by the MCP only.                         */
   SYS_SUSPEND_STATE   /* This is the usual state of a process or the     */         
                       /* system when it is awaiting the clearing of an   */
                       /* external condition which prevents normal        */
                       /* operation. The process is performing monitoring */
                       /* and reporting duties, but it will not accept    */
                       /* operating instructions (e.g. motion commands).  */
};

typedef Int32_t ProcState_t;


/*
** Process states enumeration to conform to naming convention, purely for
** generation of lookup tables - do not use in source code.
*/

typedef enum eSysProcState_e
{
   E_SYS_INVALID_STATE = SYS_INVALID_STATE, /* No state has been set (uninitialised)          */
   E_SYS_OKAY_STATE    = SYS_OKAY_STATE,    /* The process is operating normally              */
   E_SYS_INIT_STATE    = SYS_INIT_STATE,    /* The process is performing its initial start-up */
   E_SYS_STANDBY_STATE = SYS_STANDBY_STATE, /* Running but not yet ready for full operation   */
   E_SYS_WARN_STATE    = SYS_WARN_STATE,    /* Impaired engineering, operations may continue  */
   E_SYS_FAILED_STATE  = SYS_FAILED_STATE,  /* Problem has occurred requiring intervention    */
   E_SYS_SAFE_STATE    = SYS_SAFE_STATE,    /* Process is prepared for termination            */
   E_SYS_OFF_STATE     = SYS_OFF_STATE,     /* Process is not running (inferred state)        */
   E_SYS_TIMEOUT_STATE = SYS_TIMEOUT_STATE, /* Process is not responding (inferred state)     */
   E_SYS_SUSPEND_STATE = SYS_SUSPEND_STATE  /* Condition exists preventing operations         */
} eSysProcState_t;


/*
** Global data submitted to the SDB on behalf of each application
*/

typedef enum eMcpDataId_e
{

   D_MCP_DATAID_BOL = 0,     /* Beginning of data list                        */

   D_MCP_PROC_STATE = 1,     /* Process state                                 */
                             /*  units : PROCSTATE,  enum : eSysProcState_e   */
   D_MCP_AUTH_STATE,         /* Requested/granted authorisation state         */
                             /*  units : AUTH_STATE, enum : eMcpAuthState_e   */
   D_MCP_SYS_REQUEST,        /* System request for MCP (eMcpSysRequest_e)     */
                             /*  units : SYSREQ,     enum : eMcpSysRequest_e  */
   D_MCP_APP_VERSION,        /* Application version                           */
                             /*  units : MVERSION,   enum : -                 */

   D_MCP_FIRST_USER_DATUM,   /* First datum slot available to user            */

   D_MCP_DATAID_EOL          /* End of list marker                            */

} eMcpDataId_t;


/*
** Timestamp defintion
*/

typedef struct {
   Int32_t t_sec;      /* Elapsed time in seconds */
   Int32_t t_nsec;     /* Elapsed nanoseconds in present second */
} eTtlTime_t;


/*
** Generic service numbers 
*/

#define E_MCP_HEARTBEAT      255       /* heartbeat message */
#define E_MCP_SHUTDOWN       254       /* shutdown message */
#define E_MCP_SAFESTATE      253       /* safe-state message */
#define E_MCP_ACTIVATE       252       /* activate message from safe-state */
#define E_MCP_SET_1          251       /* single set command */
#define E_MCP_SET_N          250       /* multiple set command */
#define E_MCP_GET_1          249       /* single get command */
#define E_MCP_GET_N          248       /* multiple get command */


/*
** Format of generic 'set' command
*/

typedef struct
{
   Int32_t  DataId;                    /* data table element or OID */
   Int32_t  Status;                    /* package or system status code */
   Int32_t  NumBytes;                  /* number of bytes required by param */
   Int32_t  ParamHigh;                 /* high word of param (maybe unused) */
   Int32_t  ParamLow;                  /* low word of param */
   eTtlTime_t TimeStamp;               /* achieve-by time, or 0 for immediate */
   Int32_t  Units;                     /* SDB units of the value to set */
} eTtlSet_t;

/*
** Format of generic SET_N/GET_N command.
*/

/* Parameter description */

typedef Int32_t eSysDataId_t;

typedef Int32_t eSysValue_t;

typedef Int32_t eSysUnit_t;

typedef struct
{
   eSysDataId_t   Datum;
   eSysValue_t    Value;
   eTtlTime_t     TimeStamp;
   eSysUnit_t     Units;
} eSysParam_t;

#define E_SYS_MAX_SETN_PARAM 4

/* External command description */

typedef struct
{
   Status_t       Status;              /* Overall status/error code */
   Int32_t        ParamValidity;       /* Bit field indicating Param validity */
                                       /* Parameters, may be in a defined order */
   eSysParam_t    Param[ E_SYS_MAX_SETN_PARAM ];
} eSysSetNCommand_t;


/*
** Default path definitions.
*/

#define E_SYS_DFLT_BASE_PATH "/home/tcs/Documents/TCS Network Interface Documents"

#define E_SYS_DFLT_CFG_PATH  E_SYS_DFLT_BASE_PATH ""

#define E_SYS_DFLT_BIN_PATH  E_SYS_DFLT_BASE_PATH "/bin"
#define E_SYS_DFLT_UTIL_PATH E_SYS_DFLT_BASE_PATH "/util"

#define E_SYS_DFLT_DATA_PATH E_SYS_DFLT_BASE_PATH "/data"
#define E_SYS_DFLT_TMP_PATH  E_SYS_DFLT_BASE_PATH "/tmp"


/*
** Debugging and checking macros.
*/

#define TTL_ASSERT(x) assert(x)

#define TTL_MALLOC(x) malloc(x)

#define TTL_CALLOC(x, y) calloc(x, y)

#define TTL_REALLOC(x, y) realloc(x, y)

#ifdef DEBUG
   #define TTL_FREE(x) if((x) != NULL) free(x); else fprintf(stderr,"Attempt to free null pointer")
#else
   #define TTL_FREE(x) if((x) != NULL) free(x)
#endif

#ifdef TTL_DEBUG
   #define DEBUGCODE(Level, Statements) {if (Level >= TTL_DEBUG) { Statements }}
#else
   #define DEBUGCODE(Level, Statement)
#endif


#endif /* TTLSYSTEM_H_DEFINED */


/* End of file */
