/*******************************************************************************
** Module Name:
**    Tim.h
**
** Purpose:
**    Public header file for the TTL System Time (TIM) package.
**
** Description:
**    This code has been written to conform to "TTL Coding Standard" v1.0.
**
** Traceability to Software Design:
**    EPT design and implementation document v0.03 28-Sep-2000.
**
** Author(s):
**    Mark K. Bowman (mkb)
**    Derek J. McKay (djm)
**    Martyn J. Ford (mjf)
**
** Copyright (c) Telescope Technologies Limited (TTL), 2000
**
** Version:
**    $Id: Tim.h,v 0.19 2004/01/15 14:36:49 sxf Exp $
**
** History:
**    $Log: Tim.h,v $
**    Revision 0.19  2004/01/15 14:36:49  sxf
**    Updated for Neutrino.
**
**    Revision 0.18  2001/11/21 14:14:44  mjf
**    Addition of function to update the cached time using a supplied time,
**    for cases where the real-time clock is not the time source.
**
**    Revision 0.17  2001/11/20 11:31:21  mjf
**    Change of name of added functions.
**
**    Revision 0.16  2001/11/20 09:31:55  mjf
**    Implementation of functionality to cache the time to prevent the need
**    for time-consuming re-reads of the system clock.
**
**    Revision 0.15  2001/01/30 11:40:59  djm
**    Corrected incorrectly named variables.
**
**    Revision 0.14  2001/01/18 16:55:03  djm
**    Added prototype for eTimToDouble() function.
**
**    Revision 0.13  2001/01/09 09:01:21  mjf
**    Correction to 'const' argument in eTimToString.
**
**    Revision 0.12  2000/11/16 09:53:35  djm
**    Converted to coding standard. Also fixed some function prototypes.
**
**    Revision 0.11  2000/11/01 15:20:49  asp
**    Port code to Linux
**
**    Revision 0.10  2000/10/31 10:37:58  asp
**    Made __far versions of copy functions specific to QNX
**
**    Revision 0.9  2000/07/05 14:53:46  djm
**    Fixed incorrect function prototypes.
**
**    Revision 0.8  2000/06/16 15:23:56  rdl
**    Added time structure copying functions: 'eTimNearCopy' and 'eTimExtCopy'.
**
**    Revision 0.7  2000/06/15 16:15:04  rdl
**    Additions of functions: 'eTimCopy' and 'eTimFarCopy'.
**    Creation of additional 'eTimStatus_t' values.
**    Creation of #defines to remove potential 'golden numbers'.
**
**    Revision 0.6  2000/06/02 13:09:51  djm
**    Added function prototypes for eTimDifference() and eTimSum().
**
**    Revision 0.5  2000/05/31 08:44:10  djm
**    Added prototype for new eTimCompare() function.
**
**    Revision 0.4  2000/05/30 14:22:58  djm
**    Fixed typo and removed an unused error code.
**
**    Revision 0.3  2000/05/30 10:45:18  djm
**    Change to function prototype for eTimToString().
**
**    Revision 0.2  2000/05/26 08:16:01  djm
**    Adaptation from original VMS code.
**
**    Revision 0.1  2000/05/05 ??:??:??  mkb
**    Initial creation (under OpenVMS, as part of the TCS software).
**
**
*/


#ifndef TIM_H_DEFINED
#define TIM_H_DEFINED


/* Required include files */

#include "TtlSystem.h"       /* For Status_t definition */

/* Symbolic Constants */

#define E_TIM_BUFFER_LENGTH      22
#define E_TIM_TIMEA_LT_TIMEB     -1
#define E_TIM_TIMEA_EQ_TIMEB      0
#define E_TIM_TIMEA_GT_TIMEB      1

/* Enumerate list of status/error values */

typedef enum
{
   E_TIM_GEN_ERR = STATUS_START(TIM),  /* Miscellaneous error */
   E_TIM_GETTIME_FAIL,       /* Failure to read system time */
   E_TIM_NOSYSTIME,          /* No system time recovery supported */
   E_TIM_INTERRUPTED,        /* A timer function was interrupted prematurely */
   E_TIM_INVALID_ARGUMENT,   /* Argument supplied to function invalid */
   E_TIM_BUFFERTOOSMALL,     /* Insufficient size for output buffer */
   E_TIM_PARAM_NULL_PTR,     /* function parameter is NULL pointer  */

   E_TIM_EOERR_LIST          /* End error list marker (DO NOT USE FOR STATUS) */
} eTimStatus_t;


/* Function prototypes */

extern Status_t eTimGetTime    ( eTtlTime_t *TimePtr );
extern Status_t eTimCacheTime  ( eTtlTime_t *TimePtr );
extern Status_t eTimUncacheTime( eTtlTime_t *TimePtr );
extern Status_t eTimUpdateCache( eTtlTime_t UpdateTime );
Status_t eTimZeroTime          ( eTtlTime_t  *TimePtr );
extern Status_t eTimGetTimeOut ( int TimeIncr, eTtlTime_t *TimePtr );
extern Status_t eTimSleep      ( int Duration );
extern Status_t eTimToString   ( const eTtlTime_t *Time,
                                 const size_t BufferLen,
                                 char *BufferPtr);
extern Status_t eTimToDouble   ( const eTtlTime_t *Time,
                                 double *ValuePtr);
extern int      eTimCompare    ( const eTtlTime_t *TimeAPtr,
                                 const eTtlTime_t *TimeBPtr);
extern Status_t eTimDifference ( const eTtlTime_t *TimeAPtr,
                                 const eTtlTime_t *TimeBPtr,
                                 eTtlTime_t *TimeDiffPtr );

extern Status_t eTimSum  ( const eTtlTime_t *TimePtr, 
                           const eTtlTime_t *TimeDiffPtr, 
                           eTtlTime_t *NewTimePtr );
		
extern Status_t eTimCopy ( eTtlTime_t TStamp,
                           eTtlTime_t *TS_Location );

/*
** Only applicable for QNX but not Neutrino
*/
#ifndef __QNXNTO__
#ifdef __QNX__
extern Status_t eTimFarCopy  ( eTtlTime_t        TStamp,
                               eTtlTime_t __far *TS_Location);
extern Status_t eTimNearCopy ( eTtlTime_t __far TStamp,
                               eTtlTime_t       *TS_Location);
extern Status_t eTimExtCopy  ( eTtlTime_t __far TStamp,
                               eTtlTime_t __far *TS_Location);
#endif /* __QNX__ */
#endif /* __QNXNTO__ */

#endif


/*******************************************************************************
** End of File Name: Tim.h
*******************************************************************************/
