/*******************************************************************************
** Module Name:
**    Tim.c
**
** Purpose:
**    TTL system time library code.
**
** Description:
**    ...
**
**    This code has been written to conform to "TTL Coding Standard" v1.0.
**
** Traceability to Software Design:
**    EPT design and implementation document v0.03 28-Sep-2000.
**
** Author(s):
**    Mark K. Bowman (mkb)
**    Derek J. McKay (djm)
**    Robert D. Lett (rdl)
**    Andrzej S. Piascik (asp)
**    Martyn J. Ford (mjf)
**
** Copyright (c) Telescope Technologies Limited (TTL), 2000
**
** Version:
**    $Id: Tim.c,v 0.21 2004/01/15 14:36:49 sxf Exp $
**
** History:
**    $Log: Tim.c,v $
**    Revision 0.21  2004/01/15 14:36:49  sxf
**    Updated for Neutrino.
**
**    Revision 0.20  2001/11/21 14:14:44  mjf
**    Addition of function to update the cached time using a supplied time,
**    for cases where the real-time clock is not the time source.
**
**    Revision 0.19  2001/11/20 12:37:12  mjf
**    Correction to names of static variables.
**
**    Revision 0.18  2001/11/20 11:31:21  mjf
**    Change of name of added functions.
**
**    Revision 0.17  2001/11/20 09:31:55  mjf
**    Implementation of functionality to cache the time to prevent the need
**    for time-consuming re-reads of the system clock.
**
**    Revision 0.16  2001/01/30 11:40:32  djm
**    Corrected incorrectly named variables.
**
**    Revision 0.15  2001/01/18 16:54:47  djm
**    Added eTimToDouble() function.
**
**    Revision 0.14  2000/11/14 14:55:30  djm
**    Adapted to conform to the coding standard.
**    Reworked handling of algorithms for negative durations.
**
**    Revision 0.13  2000/11/01 15:20:29  asp
**    Port code to Linux.
**
**    Revision 0.12  2000/07/05 14:53:21  djm
**    Fixed incorrect function return types.
**
**    Revision 0.11  2000/06/16 15:21:37  rdl
**    Additional tests for new time structure copying functions:
**    'eTimNearCopy' and 'eTimExtCopy'.
**
**    Revision 0.10  2000/06/15 14:46:15  rdl
**    Correction of typing error.
**
**    Revision 0.9  2000/06/15 14:40:11  rdl
**    1.  Addition of functions: 'eTimCopy' and 'eTimFarCopy'.
**    2.  Removal of include file: 'TimPrivate.h'.
**    3.  Addition of include file: 'TtlConstants.h'.
**    4.  Removal of potential 'golden numbers'.
**    5.  Addition of checks on function input parameters.
**    6.  Updated comments, adding detail where appropriate.
**
**    Revision 0.8  2000/06/07 10:06:10  djm
**    eTimGetTime() now sets the time to {0,0} in the case of a
**    failure in recovering the system time.
**
**    Revision 0.7  2000/06/02 13:11:00  djm
**    Added eTimDifference() and eTimSum() functions.
**
**    Revision 0.6  2000/05/31 08:43:51  djm
**    Added eTimCompare() function.
**
**    Revision 0.5  2000/05/30 14:24:11  djm
**    Minor change to comments. Fixed typo in symbolic constant use.
**
**    Revision 0.4  2000/05/30 12:41:19  djm
**    Changed VMS code to get it working again on that O/S.
**
**    Revision 0.3  2000/05/30 10:43:57  djm
**    Changes to comments, and code standardisation.
**
**    Revision 0.2  2000/05/26 08:11:04  djm
**    Adaptation to QNX from original VMS code.
**
**    Revision 0.1  2000/05/05 ??:??:??  mkb
**    Initial creation (under OpenVMS, as part of the TCS software).
**
*******************************************************************************/


/*
** Compiler include files
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>


/*
** System include files
*/

#include "TtlConstants.h"
#include "Tim.h"


/*
** Functions
*/

Status_t mTimGetSystemTime( eTtlTime_t * TimePtr );


/*
** Module-level variables.
*/

static Bool_t mTimCacheFlag = FALSE;
static eTtlTime_t mTimCachedTime;


/*******************************************************************************
** Function Name:
**    mTimReadSystemTime
**
** Purpose:
**    Reads the current system time from the real-time clock.
**
** Description:
**    This function reads the current system time and puts it into a time 
**    structure. The VMS code makes use of System V Release 4 code standard 
**    (gettimeofday()).
**
**    If the system time can not be determined the function will set the time 
**    structure to zero, and return an error code.
**
**    This function was previously named eTimGetTime().
**
**    NOTE : THIS FUNCTION CONTAINS SYSTEM DEPENDENT CALLS
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    eTtlTime_t *TimePtr      (in/out)
**       A pointer to a structure containing the time result.
**
** Authors:
**    Mark K. Bowman (mkb)
**    Derek J. McKay (djm)
**
*******************************************************************************/

Status_t mTimReadSystemTime
(
   eTtlTime_t  *TimePtr
)
{


   /*
   ** QNX SPECIFIC CODE
   */

/* QNX 4 specific code so exclude Neutrino */
#ifndef __QNXNTO__
#ifdef __QNX__
   struct timespec TimeSpec;

   /* Check if input parameters are valid */
   if (TimePtr == NULL)
   {
      return (E_TIM_PARAM_NULL_PTR) ;
   }

   /* Zero the result structure, in case of failure */
   TimePtr->t_sec = 0;
   TimePtr->t_nsec = 0;

   if(clock_gettime(CLOCK_REALTIME, &TimeSpec) == -1)
   {
      return E_TIM_GETTIME_FAIL;
   }
   TimePtr->t_sec = TimeSpec.tv_sec;
   TimePtr->t_nsec = TimeSpec.tv_nsec;

   return SYS_NOMINAL;
#endif
#endif



   /*
   ** VMS and LINUX SPECIFIC CODE (POSIX.4)
   */

#if ( defined __VMS || defined __GNUC__ )
   struct timeval TimeVal;

   /* Check if input parameters are valid */
   if (TimePtr == NULL)
   {
      return (E_TIM_PARAM_NULL_PTR) ;
   }

   /* Zero the result structure, in case of failure */
   TimePtr->t_sec = 0;
   TimePtr->t_nsec = 0;

   if(gettimeofday( (struct timeval *) &TimeVal, NULL ) == -1)
   {
      return E_TIM_GETTIME_FAIL;
   }

   /* Convert microsecond component of time to nanoseconds. */
   TimePtr->t_sec = TimeVal.tv_sec;
   TimePtr->t_nsec = ((int) (E_TTL_NANOSECS_PER_SEC / E_TTL_MICROSECS_PER_SEC))
                    * (TimeVal.tv_usec);

   return SYS_NOMINAL;
#endif /* __VMS || __GNUC__  */



   /*
   ** DEFAULT, FOR COMPILING ON NON-SUPPORTED PLATFORMS
   */

#if ( !defined __VMS && !defined __QNX__ && !defined __GNUC__ )
   return E_TIM_NOSYSTIME;
#endif /* !__GNUC__ && !__QNX__ && !__VMS */


}  /* End of mTimReadSystemTime() */


/*******************************************************************************
** Function Name:
**    eTimGetTimeOut
**
** Purpose:
**    Get a timeout value by adding TimeIncr to current time.
**
** Description:
**    This function gets the current system time and adds an
**    increment in order to generate a future time. Thus new
**    time is returned as the TimePtr argument. The naming of
**    this function has been chosen for compatibility with the
**    existing TCS code.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    int TimeIncr             (in)
**       The number of seconds to be added to the current system
**       time, as determined by this function.
**    eTtlTime_t *TimePtr      (in/out)
**       A pointer to a structure containing the future time result.
**
** Authors:
**    Mark K. Bowman (mkb)
**    Derek J. McKay (djm)
**
*******************************************************************************/

Status_t eTimGetTimeOut
(
   int  TimeIncr,
   eTtlTime_t  *TimePtr
)
{
   /* Local variable(s) */
   Status_t Status;          /* Return value from functions */


   /* Check if input parameters are valid */
   if (TimePtr == NULL)
   {
      return (E_TIM_PARAM_NULL_PTR) ;
   }

   /* Get the current system time */
   Status = eTimGetTime(TimePtr);
   if(Status != SYS_NOMINAL)
   {
      return Status;
   }

   /* Add the time increment to the determined time */
   TimePtr->t_sec += TimeIncr;

   /* Return success status */
   return SYS_NOMINAL;

}  /* End of eTimGetTimeOut() */






/*******************************************************************************
** Function Name:
**    eTimZeroTime
**
** Purpose:
**    Sets a time structure to zero.
**
** Description:
**    This function abstracts the setting of all internal fields of a
**    TTLtime structure to zero.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    eTtlTime_t *TimePtr      (in/out)
**       A pointer to a structure containing the time result.
**
** Authors:
**    Mark K. Bowman (mkb)
**    Derek J. McKay (djm)
**
*******************************************************************************/

Status_t eTimZeroTime
(
   eTtlTime_t  *TimePtr
)
{
   /* No local variables */


   /* Set the fields to zero */
   TimePtr->t_sec  = 0;
   TimePtr->t_nsec = 0;


   /* Return success */
   return (SYS_NOMINAL) ;

}  /* End of eTimZeroTime() */


/*******************************************************************************
** Function Name:
**    eTimGetTime
**
** Purpose:
**    Returns the current system time.
**
** Description:
**    This function is used to return the current system time. If a call to 
**    'eTimCacheTime' has been made, then time caching will be enabled. If a 
**    call to 'eTimUncacheTime' has been made, then time caching will be 
**    disabled. Time caching will be disabled by default. If time caching is 
**    disabled, then a call will be made to the function 'mTimGetSystemTime' 
**    which will read from the real-time clock. If time caching is enabled, 
**    then this function will return the time than was last re-freshed by a 
**    call to the function 'eTimCacheTime'.
**
**    If the system time can not be determined the function will set the time 
**    structure to zero, and return an error code.
**
**    The previous contents of this function are now in 'mTimReadSystemTime'.
**
**    NOTE : THIS FUNCTION CALLS FUNCTION 'mTimGetSystemTime', WHICH CONTAINS
**           SYSTEM CALLS.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    eTtlTime_t *TimePtr      (in/out)
**       A pointer to a structure containing the time result.
**
** Authors:
**    Martyn J. Ford (mjf)
**
*******************************************************************************/

Status_t eTimGetTime
(
   eTtlTime_t  *TimePtr
)
{
   Status_t Status;                    /* Completion status of this function */

   /*
   ** If time caching is enabled.
   */
   if ( mTimCacheFlag == TRUE )
   {
      /*
      ** Take a copy of the last read time for return.
      */
      memcpy( TimePtr, &mTimCachedTime, sizeof( *TimePtr ) );

      /*
      ** Set status to indicate successful completion.
      */
      Status = SYS_NOMINAL;
   }

   /*
   ** If time caching is disabled.
   */
   if ( mTimCacheFlag == FALSE )
   {
      /*
      ** Read the time from the system clock.
      */
      Status = mTimReadSystemTime( TimePtr );
   }

   return Status;

}  /* End of eTimGetTime() */


/*******************************************************************************
** Function Name:
**    eTimCacheTime
**
** Purpose:
**    Refreshes the buffered copy of the current system time.
**
** Description:
**    This function forces a re-read of the current system time from the 
**    real-time clock, and makes a local copy of this time. Once this function 
**    has been called, subsequent calls to the function 'eTimGetTime' will 
**    return this copy of the system time, until the next call of 'eTimRefresh'.
**    This is for use on operating systems where the reading of the system time 
**    from the real-time clock is time-consuming. 
**
**    If the system time can not be determined the function will set the time 
**    structure to zero, and return an error code.
**
**    NOTE : THIS FUNCTION CALLS FUNCTION 'mTimGetSystemTime', WHICH CONTAINS
**           SYSTEM CALLS.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    eTtlTime_t *TimePtr      (in/out)
**       A pointer to a structure containing the time result.
**
** Authors:
**    Martyn J. Ford (mjf)
**
*******************************************************************************/

Status_t eTimCacheTime  ( eTtlTime_t *TimePtr )
{
   Status_t Status;                    /* Completion status of this function */


   /*
   ** Read the time from the system clock.
   */
   Status = mTimReadSystemTime( TimePtr );

   /*
   ** If the time was obtained successfully.
   */
   if ( Status == SYS_NOMINAL )
   {
      /*
      ** Set the 'cache flag' to TRUE, to indicate we've taken a local copy of
      ** the time.
      */
      mTimCacheFlag = TRUE;

      /*
      ** Take a copy of the system time for the cache.
      */
      memcpy( &mTimCachedTime, TimePtr, sizeof( mTimCachedTime ) );
   }

   return Status;

}  /* End of eTimCacheTime() */


/*******************************************************************************
** Function Name:
**    eTimUncacheTime
**
** Purpose:
**    Forces a re-read of the system time from the real-time clock, and turns 
**    off caching of the time reading.
**
** Description:
**    This function forces a re-read of the current system time from the 
**    real-time clock, and ensures that all subsequent calls to the function 
**    'eTimGetTime' also re-read the time from the real-time clock.
**
**    If the system time can not be determined the function will set the time 
**    structure to zero, and return an error code.
**
**    NOTE : THIS FUNCTION CALLS FUNCTION 'mTimGetSystemTime', WHICH CONTAINS
**           SYSTEM CALLS.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    eTtlTime_t *TimePtr      (in/out)
**       A pointer to a structure containing the time result.
**
** Authors:
**    Martyn J. Ford (mjf)
**
*******************************************************************************/

Status_t eTimUncacheTime( eTtlTime_t *TimePtr )
{
   Status_t Status;                    /* Completion status of this function */


   /*
   ** Read the time from the system clock.
   */
   Status = mTimReadSystemTime( TimePtr );

   /*
   ** Set the 'cache flag' to FALSE, to indicate the time should be re-read in
   ** future.
   */
   mTimCacheFlag = FALSE;

   return Status;

}  /* End of eTimUncacheTime() */


/*******************************************************************************
** Function Name:
**    eTimUpdateCache
**
** Purpose:
**    Forces the internal cached time to be updated with the supplied time.
**
** Description:
**    This function forces the internally-cached time to be updated with the 
**    supplied time, for returning by all subsequent calls to the function 
**    'eTimGetTime'. This will be used in situations where the time is not to 
**    be determined by the real-time clock, but by another source (eg IRIG-B).
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    eTtlTime_t UpdateTime    (in)
**       A structure containing the new time to cache.
**
** Authors:
**    Martyn J. Ford (mjf)
**
*******************************************************************************/

Status_t eTimUpdateCache( eTtlTime_t UpdateTime )
{

   /*
   ** Set the flag TRUE, indicating that time cache is active.
   */
   mTimCacheFlag = TRUE;

   /*
   ** Take a copy of the supplied time, into the cache.
   */
   memcpy( &mTimCachedTime, &UpdateTime, sizeof( mTimCachedTime ) );

   return SYS_NOMINAL;

}


/*******************************************************************************
** Function Name:
**    eTimSleep
**
** Purpose:
**    Sleep for supplied number of seconds.
**
** Description:
**    This function will wait for a specified number of seconds
**    before returning.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    int Duration             (in)
**       The time, in seconds, for the function to pause, before
**       returning.
**
** Authors:
**    Mark K. Bowman (mkb)
**    Derek J. McKay (djm)
**
*******************************************************************************/

Status_t eTimSleep
(
   int Duration
)
{
   /* Local variables */
   int SleepStatus;

   /* Check if the specified sleep period is positive */
   if ( Duration > 0 )
   {
      /* Sleep for the specified duration (or until an interrupt is received) */
      SleepStatus = (int) sleep((unsigned)Duration);
      if ( SleepStatus != 0 )
      {
         return E_TIM_INTERRUPTED;
      }
      return SYS_NOMINAL;
   }
   else
   {
      return E_TIM_INVALID_ARGUMENT;
   }

}  /* End of eTimSleep() */



/*******************************************************************************
** Function Name:
**    eTimToString
**
** Purpose:
**    Put ASCII representation of the time into a character array.
**
** Description:
**    This function takes a TTL time structure and puts an ASCII
**    representation of it into a character array supplied by the
**    caller. The size of the character array is given by BufferLen.
**    The format of the output string is "DD/MM/YY HH:MM:SS.CCC",
**    and therefore BufferLen should be greater than 21.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    const eTtlTime_t *TimePtr        (in)
**       Pointer to a TTL time structure containing the time to
**       be converted.
**    size_t BufferLen                 (in)
**       Value containing the size of the character array.
**    char *BufferPtr                  (in/out)
**       Pointer to the character array into which to put the ASCII
**       representation.
**
** Authors:
**    Derek J. McKay (djm)
**
*******************************************************************************/

Status_t eTimToString
(
   const eTtlTime_t  *TimePtr,
   size_t  BufferLen,
   char  *BufferPtr
)
{
   /* No local variables */


   /* Check if input parameters are valid */
   if (TimePtr == NULL)
   {
      return (E_TIM_PARAM_NULL_PTR) ;
   }

   /* Check if input parameters are valid */
   if (BufferPtr == NULL)
   {
      return (E_TIM_PARAM_NULL_PTR) ;
   }

   /* Check the message length */
   if(BufferLen < E_TIM_BUFFER_LENGTH)
   {
      return E_TIM_BUFFERTOOSMALL;
   }

   /* Format the basic message:
   ** %d - day of the month as a decimal number.
   ** %m - month as a decimal number.
   ** %y - year as a decimal number, without century.
   ** %T - 24-hour clock time in the format HH:MM:SS (POSIX).
   */
   strftime
   (
      BufferPtr, BufferLen,
      "%d/%m/%y %T", localtime((time_t *)&(TimePtr->t_sec))
   );

   /*
   ** Append the nanoseconds -
   ** displaying microseconds as three decimal figures
   */
   sprintf
   (
      BufferPtr, "%s.%3.3d",
      BufferPtr, (TimePtr->t_nsec/((int) E_TTL_MICROSECS_PER_SEC))
   );

   /* Return success */
   return SYS_NOMINAL;

}  /* End of eTimToString() */



/*******************************************************************************
** Function Name:
**    eTimCompare
**
** Purpose:
**    Compares two TTL time structures.
**
** Description:
**    This function compares two TTL time structures. It will
**    return -1 if TimeA is earlier than TimeB. It will return
**    0 if TimeA is the same as TimeB and it will return 1 if
**    TimeA is later than TimeB. This function will also work
**    for durations (i.e. TTL time structures used to represent
**    delta-times).
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    const eTtlTime_t *TimeAPtr       (in)
**       Pointer to a TTL time structure.
**    const eTtlTime_t *TimeBPtr       (in)
**       Pointer to a TTL time structure.
**
** Authors:
**    Derek J. McKay (djm)
**
*******************************************************************************/

int eTimCompare
(
   const eTtlTime_t  *TimeAPtr,
   const eTtlTime_t  *TimeBPtr
)
{
   /* No local variables */


   /* Start by comparing the seconds component */
   if(TimeAPtr->t_sec > TimeBPtr->t_sec)
   {
      return E_TIM_TIMEA_GT_TIMEB;
   }
   else if(TimeAPtr->t_sec < TimeBPtr->t_sec)
   {
      return E_TIM_TIMEA_LT_TIMEB;
   }
   else
   {
      /* If the seconds components are equal, then compare the nanoseconds */
      if(TimeAPtr->t_nsec > TimeBPtr->t_nsec)
      {
         return E_TIM_TIMEA_GT_TIMEB;
      }
      else if(TimeAPtr->t_nsec < TimeBPtr->t_nsec)
      {
         return E_TIM_TIMEA_LT_TIMEB;
      }
      else
      {
         return E_TIM_TIMEA_EQ_TIMEB;
      }
   }

   /* We should never get this far. */

}  /* End of eTimCompare() */





/*******************************************************************************
** Function Name:
**    eTimDifference
**
** Purpose:
**    Determines the difference between two TTL time structures.
**
** Description:
**    This function computes the difference between two time values.
**    The pointer to the output time structure may be the same as
**    the arguments; the function takes a copy of both input 
**    arguments before modifying the output.
**
**    The difference is positive if TimeB is later than TimeA
**    The difference is negative if TimeB is earlier than TimeA
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    const eTtlTime_t *TimeAPtr       (in)
**       Pointer to a TTL time structure.
**    const eTtlTime_t *TimeBPtr       (in)
**       Pointer to a TTL time structure.
**    eTtlTime_t *TimeDiffPtr          (out)
**       Popinter to a TTL time structure containing the result.
**
** Authors:
**    Mike Curtis (myc)
**    Derek J. McKay (djm)
**
*******************************************************************************/

Status_t eTimDifference
(
   const eTtlTime_t  *TimeAPtr,
   const eTtlTime_t  *TimeBPtr,
   eTtlTime_t  *DiffTimePtr
)
{
   /* Local variables */
   eTtlTime_t T1;           /* Copy of first time in argument list */
   eTtlTime_t T2;           /* Copy of second time in the argument list */


   /* Make a copy of the time variables for ease of use */
   memcpy(&T1, TimeAPtr, sizeof(eTtlTime_t));
   memcpy(&T2, TimeBPtr, sizeof(eTtlTime_t));


   DiffTimePtr->t_sec  = T2.t_sec  - T1.t_sec;
   DiffTimePtr->t_nsec = T2.t_nsec - T1.t_nsec;

   if( DiffTimePtr->t_nsec < 0 )
   {
      DiffTimePtr->t_sec-- ;
      DiffTimePtr->t_nsec += E_TTL_NANOSECS_PER_SEC; 
   }
   else if( DiffTimePtr->t_nsec >= E_TTL_NANOSECS_PER_SEC )
   {
      DiffTimePtr->t_sec++ ;
      DiffTimePtr->t_nsec -= E_TTL_NANOSECS_PER_SEC; 
   }


   /* Return success */
   return SYS_NOMINAL;

}  /* End of eTimDifference() */





/*******************************************************************************
** Function Name:
**    eTimSum
**
** Purpose:
**    Adds a duration to a time to determine a new time.
**
** Description:
**    This function computes a new time value from a current time
**    and a time difference value. The pointer to the output time
**    structure may be the same as the arguments; the function takes
**    a copy of both input arguments before modifying the output.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    const eTtlTime_t *Time           (in)
**       Pointer to a TTL time structure.
**    const eTtlTime_t *TimeDiff       (in)
**       Pointer to a TTL time structure containing a time duration.
**    eTtlTime_t *NewTimePtr           (out)
**       Pointer to a TTL time structure containing the result.
**
** Authors:
**    Mike Curtis (myc)
**    Derek J. McKay (djm)
**
*******************************************************************************/

Status_t eTimSum
(
   const eTtlTime_t  *TimePtr,
   const eTtlTime_t  *TimeDiffPtr,
   eTtlTime_t  *NewTimePtr
)

{
   /* Local variables */
   eTtlTime_t T1;           /* Copy of first time in argument list */
   eTtlTime_t T2;           /* Copy of second time in the argument list */



   /* Make a copy of the time variables for ease of use */
   memcpy(&T1, TimePtr, sizeof(eTtlTime_t));
   memcpy(&T2, TimeDiffPtr, sizeof(eTtlTime_t));

   /* Add the time components */
   NewTimePtr->t_sec  = T1.t_sec  + T2.t_sec;
   NewTimePtr->t_nsec = T1.t_nsec + T2.t_nsec;

   /* Check that the nanosecond component is in the correct range */
   if(NewTimePtr->t_nsec >= (int) E_TTL_NANOSECS_PER_SEC)
   {
      NewTimePtr->t_nsec -= (int) E_TTL_NANOSECS_PER_SEC;
      NewTimePtr->t_sec++;
   }
   else if(NewTimePtr->t_nsec < 0)
   {
      NewTimePtr->t_nsec += (int) E_TTL_NANOSECS_PER_SEC;
      NewTimePtr->t_sec--;
   }


   /* Terminate the function, returning success */
   return SYS_NOMINAL;

}  /* End of eTimSum() */


/*******************************************************************************
** Function Name:
**    eTimCopy
**
** Purpose:
**    The purpose of this function is to copy (set) the elements
**    of a timestamp structure to the same values as those contained
**    in a time structure that is passed as a parameter to the
**    function.
**
** Description:
**    This function stores a timestamp in the appropriate time
**    stamp storage variable, performing an element by element copy.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    eTtlTime_t  TStamp               (in)
**       A 'near' time structure containing the time that is to
**       be used to set the required time to.
**    eTtlTime_t  *TS_Location         (out)
**       The 'near' pointer to a time structure that is to be 
**       set to the same time as that contained in the source
**       time structure.
**
** Authors:
**    Robert D. Lett (rdl)
**
*******************************************************************************/

Status_t eTimCopy
(
   eTtlTime_t  TStamp,
   eTtlTime_t  *TS_Location
)
{

   /* Check if input parameters are valid */
   if (TS_Location == NULL)
   {
      return (E_TIM_PARAM_NULL_PTR);
   }

   /* Actually do the copy */
   TS_Location->t_sec  = TStamp.t_sec ;
   TS_Location->t_nsec = TStamp.t_nsec ;

   /* Terminate function, returning completion status */
   return (SYS_NOMINAL);

}  /* End of eTimCopy() */





/*******************************************************************************
** Function Name:
**    eTimFarCopy
**
** Purpose:
**    The purpose of this function is to copy (set) the elements
**    of a timestamp structure to the same values as those contained
**    in a time structure that is passed as a parameter to the
**    function.
**
** Description:
**    This function stores a timestamp in the appropriate time
**    stamp storage variable, performing an element by element copy.
**
**    NB: THIS FUNCTION IS ONLY PROVIDED IN THE QNX IMPLEMENTATION
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    eTtlTime_t  TStamp               (in)
**       A 'near' time structure containing the time that is to
**       be used to set the required time to.
**    __far eTtlTime_t  *TS_Location   (out)
**       A 'far' pointer to a time structure that is to be set
**       to the same time as that contained in the source time
**       structure.
**
** Authors:
**    Robert D. Lett (rdl)
**
*******************************************************************************/

/* QNX4 specific so exclude Neutrino */
#ifndef __QNXNTO__
#ifdef __QNX__
Status_t  eTimFarCopy
(
   eTtlTime_t  TStamp,
   eTtlTime_t __far *TS_Location
)
{
   /* No local variables */


   /* Check if input parameters are valid */
   if (TS_Location == NULL)
   {
      return (E_TIM_PARAM_NULL_PTR) ;
   }


   /* Actually do the copy */
   TS_Location->t_sec  = TStamp.t_sec ;
   TS_Location->t_nsec = TStamp.t_nsec ;


   /* Terminate the function, returning success */
   return (SYS_NOMINAL) ;

}  /* End of eTimFarCopy() */
#endif /* __QNX__ */
#endif




/*******************************************************************************
** Function Name:
**    eTimNearCopy
**
** Purpose:
**    The purpose of this function is to copy (set) the elements
**    of a timestamp structure to the same values as those contained
**    in a time structure that is passed as a parameter to the
**    function.
**
** Description:
**    This function stores a timestamp in the appropriate time
**    stamp storage variable, performing an element by element copy.
**
**    NB: THIS FUNCTION IS ONLY PROVIDED IN THE QNX IMPLEMENTATION
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    __far eTtlTime_t  TStamp         (in)
**       A 'far' time structure containing the time that is to
**       be used to set the required time to.
**    eTtlTime_t  *TS_Location         (out)
**       A 'near' pointer to a time structure that is to be set
**       to the same time as that contained in the source time
**       structure.
**
** Authors:
**    Robert D. Lett (rdl)
**
*******************************************************************************/

/* QNX4 specific so exclude Neutrino */
#ifndef __QNXNTO__
#ifdef __QNX__
Status_t  eTimNearCopy
(
   eTtlTime_t  __far TStamp,
   eTtlTime_t  *TS_Location
)
{
   /* No local variables */


   /* Check if input parameters are valid */
   if (TS_Location == NULL)
   {
      return (E_TIM_PARAM_NULL_PTR) ;
   }

   /* Actually do the copy */
   TS_Location->t_sec  = TStamp.t_sec ;
   TS_Location->t_nsec = TStamp.t_nsec ;

   /* Terminate the function, returning sucess */
   return (SYS_NOMINAL) ;

}  /* End of eTimNearCopy() */
#endif /* __QNX__ */
#endif



/*******************************************************************************
** Function Name:
**    eTimExtCopy
**
** Purpose:
**    The purpose of this function is to copy (set) the elements
**    of a timestamp structure to the same values as those contained
**    in a time structure that is passed as a parameter to the
**    function.
**
** Description:
**    This function stores a timestamp in the appropriate time
**    stamp storage variable, performing an element by element copy.
**
**    NB: THIS FUNCTION IS ONLY PROVIDED IN THE QNX IMPLEMENTATION
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    __far eTtlTime_t  TStamp         (in)
**       A 'far' time structure containing the time that is to
**       be used to set the required time to.
**    __far eTtlTime_t  *TS_Location   (out)
**       A 'far' pointer to a time structure that is to be set
**       to the same time as that contained in the source time
**       structure.
**
** Authors:
**    Robert D. Lett (rdl)
**
*******************************************************************************/

/* QNX4 specific so exclude Neutrino */
#ifndef __QNXNTO__
#ifdef __QNX__
Status_t  eTimExtCopy
(
   eTtlTime_t __far TStamp,
   eTtlTime_t __far *TS_Location
)
{
   /* No local variables */


   /* Check if input parameters are valid */
   if (TS_Location == NULL)
   {
      return (E_TIM_PARAM_NULL_PTR) ;
   }

   /* Actually do the copy */
   TS_Location->t_sec  = TStamp.t_sec ;
   TS_Location->t_nsec = TStamp.t_nsec ;

   /* Terminate the function returning success */
   return (SYS_NOMINAL) ;

}  /* End of eTimExtCopy() */
#endif /* __QNX__ */
#endif


/*******************************************************************************
** Function Name:
**    eTimToDouble
**
** Purpose:
**    Convert a time structure to a double precision number
**
** Description:
**    The output is the seconds field plus the normalised nanoseconds 
**    field. This will be the number of seconds since epoch as a floating
**    point number.
**
**    NOTE: Due to the large size of the seconds field, and the level of
**    precision of the nanoseconds field, the accuracy of the double
**    precision number that is calculated is only accurate to the limitations
**    of the internal representation of the double. Under QNX, for dates circa
**    2001 AD, this is of the order of a few microseconds.
**
** Return Type:
**    Status_t
**       Returns the status of this function on completion.
**
** Arguments:
**    const eTtlTime_t *TimePtr        (in)
**       Pointer to a TTL time structure containing the time to
**       be converted.
**    double *ValuePtr                 (out)
**       Pointer to memory where the output of the conversion shall be
**       placed.
**
** Authors:
**    Derek J. McKay (djm)
**
*******************************************************************************/

Status_t eTimToDouble
(
   const eTtlTime_t *TimePtr,
   double *ValuePtr
)
{
   /* No local variables */

   /* Make the conversion */
   *ValuePtr = (double)(TimePtr->t_sec);
   *ValuePtr += (double)(TimePtr->t_nsec) / E_TTL_NANOSECS_PER_SEC;

   /* Return success */
   return SYS_NOMINAL;

}  /* End of eTimToDouble() */



/*******************************************************************************
** End of File Name: Tim.c
*******************************************************************************/
