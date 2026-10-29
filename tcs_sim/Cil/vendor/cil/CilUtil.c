/*******************************************************************************
** Module Name:
**    CilUtil.c
**
** Purpose:
**    Provides access to CIL-related functions for processes not actually using
**    CIL communications.
**
** Description:
**    This module contains CIL (Communication Interface Library) utility 
**    functions for use by processes who are not using CIL for communications.
**    The utility functions provide information about the use of CIL.
**
**    This code has been written to conform to "TTL Coding Standard" v1.0.
**
** Traceability to Software Design:
**    -
**
** Author(s):
**    Martyn J. Ford (mjf)
**
** Copyright (c) Telescope Technologies Limited (TTL), 2001
**
** Version:
**    $Id: CilUtil.c,v 0.3 2007/03/12 15:07:30 sxf Exp $
**
** History:
**    $Log: CilUtil.c,v $
**    Revision 0.3  2007/03/12 15:07:30  sxf
**    Minor modifications to build under linux.
**
**    Revision 0.2  2005/06/20 13:25:56  mjf
**    Tidied up to aid porting to Linux - no functional changes.
**
**    Revision 0.1  2001/03/02 15:51:50  mjf
**    Initial revision.
**
**
*******************************************************************************/


/* Compiler headers */

#include <stdio.h>           /* UNIX 'Standard I/O' Definitions   */
#include <string.h>

/* System headers */

#include "TtlSystem.h"
#include "Cfu.h"


/* Local Headers */

#include "Cil.h"
#include "CilPrivate.h"



/*
** Global data
*/

/* Shared with other CIL source files */

iCilDest_t *iCilDestListPtr = NULL;    /* Pointer to linked list */

/* Static list of CIL names created during 'eCilSetup' returned by 'eCilName' */

char iCilName[ E_CIL_EOL + 1 ][ E_CIL_IDLEN ];


/*******************************************************************************
**
** Function Name:
**    eCilNameString
**
** Type:
**    char * 
**       Returns a pointer to a string containing the CIL name of the specified
**       CIL ID.
**
** Purpose:
**    Function to return the name of a specified CIL ID - can be used to make 
**    log messages more informative.
**
** Description:
**    This function accepts a CIL identifier as a parameter, and returns a 
**    pointer to a string containing the associated CIL name. If the specified 
**    identifier is the beginning or end of list a suitable string is used to 
**    denote this, and also if the CIL identifier is out of range. Note that 
**    function obtains the name from an array in memory which has been 
**    initialised during 'eCilSetup', and does not access the disk.
**
** Arguments:
**    Int32_t CilId                    (in)
**       The identity of the CIL to return the name for.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
*******************************************************************************/

char *   eCilNameString( Int32_t CilId )
{
   static Bool_t FirstTime = TRUE;     /* Flag denoting initial function call */

   int Count;                /* Counter, starts at 0 (c.f. Cil.h) */
   char EntryName[ I_CIL_LINELEN + 1 ]; /* String for CIL name from file */
   char *TxtNamePtr;         /* Pointer to ID in text from CIL entry */
   Status_t CfuStatus;       /* Status returned by CFU functions */

   /* If this function is being called for the first time */
   if ( FirstTime == TRUE )
   {
      /* Set the FirstTime flag to be FALSE */
      FirstTime = FALSE;

      /* If CIL setup has not been performed, read names from the CIL map */
      if ( eCilAlreadySetup() == FALSE )
      {
         /* Open the default CIL map for reading */
         CfuStatus = eCfuSetup( I_CIL_DFLT_CIL_MAP );
         if ( CfuStatus != SYS_NOMINAL )
         {
            eCfuComplete();
            return E_CIL_STR_INVALID;
         }

         /* Set up basic data items */
         Count = E_CIL_BOL;

         /* Clear static array of CIL names */
         memset( iCilName, '\0', sizeof( iCilName ) );

         /* Read thru map file setting up destination table and local port */
         while ( CfuStatus == SYS_NOMINAL )
         {
            /* Get a line from the file */
            CfuStatus = eCfuGetLine( NULL );

            /* If at least one argument found on the line */
            if ( eCfuParamCount() > 0 )
            {
               /* Parse line into expected components */
               TxtNamePtr = NULL;

               if ( eCfuGetNumParam( I_CIL_PARAM_NAME, EntryName )
                       == SYS_NOMINAL )
               {
                  TxtNamePtr = EntryName;
               }
      
               /* If valid entry read in correctly */
               if ( ( TxtNamePtr == NULL ) 
                     || ( strlen( TxtNamePtr ) > E_CIL_IDLEN ) )
               {
                  return E_CIL_STR_INVALID;
               }
      
               /* Increment number of processes found */
               Count++;

               /* Copy CIL name into static array */
               strcpy( iCilName[ Count ], TxtNamePtr );
               /* Ensure string is zero-terminated */
               iCilName[ Count ][ E_CIL_IDLEN - 1 ] = '\0';

            }   /* End of if eCfuParamCount() > 0 */

         }   /* End of while CfuStatus == SYS_NOMINAL */

         /* Close the map file */
         eCfuComplete();
      }
   }

   /* Check that CIL ID is within range */
   if ( ( CilId > E_CIL_BOL ) && ( CilId < E_CIL_EOL ) )
   {
      /* Return pointer to CIL name from static array */
      return iCilName[ CilId ];
   }

   /* Check for beginning of list */
   if ( CilId == E_CIL_BOL )
   {
      /* Return string denoting BOL */
      return E_CIL_STR_BOL;
   }

   /* Check for end of list */
   if ( CilId == E_CIL_EOL )
   {
      /* Return string denoting EOL */
      return E_CIL_STR_EOL;
   }

   /* Else CIL ID must be invalid, return relevant string */
   return E_CIL_STR_INVALID;

} 


/*******************************************************************************
**
** Function Name:
**    eCilAlreadySetup
**
** Type:
**    Bool_t
**       Returns TRUE if a CIL setup has already been performed, else it 
**       returns FALSE.
**
** Purpose:
**    This function returns whether or not a CIL setup has been performed.
**
** Description:
**    This function simply checks for an already-performed CIL setup and 
**    returns TRUE if there has, else returns FALSE.
**
** Arguments:
**    None
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
*******************************************************************************/

Bool_t   eCilAlreadySetup( void )
{

   /* If list pointer is NULL, then CIL setup has not yet been done */
   if ( iCilDestListPtr == NULL )
   {
      return FALSE;
   }

   /* If we get here, then CIL setup must have been done */
   return TRUE;

}


/* EOF */

