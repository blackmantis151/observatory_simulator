/*
** Module Name:
**    Cfu.c
**
** Purpose:
**    Provides common functions for accessing configuration files.
**
** Description:
**    Contains functions to set up the use of Configuration File Utilities, 
**    and retrieve both complete lines and parameters from such a file, where 
**    the unwanted characters will be removed.
**
**    This code has been written to conform to "TTL Coding Standard" v0.6.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** Version:
**    $Id: Cfu.c,v 0.8 2004/05/06 13:18:14 mjf Exp $
**
** History:
**    $Log: Cfu.c,v $
**    Revision 0.8  2004/05/06 13:18:14  mjf
**    eCfuSetup made robust to a NULL pointer.
**
**    Revision 0.7  2001/01/18 09:50:27  mjf
**    Filename passed to setup function now 'const'.
**
**    Revision 0.6  2000/12/20 14:19:35  mjf
**    Addition of support to use up to 256 files at once. The reading of the
**    files is nested. Note that an unsuccessful 'setup' must still be
**    'complete'-ed.
**
**    Revision 0.5  2000/12/18 12:11:32  mjf
**    Fix to resetting of setup level when completing use of a config file.
**
**    Revision 0.4  2000/09/01 14:34:47  mjf
**    Fix to trim whitespace at start of line correctly in all cases.
**
**    Revision 0.3  2000/08/17 16:37:47  mjf
**    Use Status_t and SYS_NOMINAL to return status.
**
**    Revision 0.2  2000/08/09 14:14:33  mjf
**    Addition of functions to test exsistence of a parameter, and at a position,
**    and retrieve the number of parameters on a line.
**
**    Revision 0.1  2000/08/09 09:08:45  mjf
**    Initial revision.
**
**
*/


/*
** Compiler include files
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
** System include files
*/

#include "TtlSystem.h"
#include "Cfu.h"


/*
** Local include files
*/

#include "CfuPrivate.h"


/* 
** Global variables
*/

/* index to current file in list being used */

static int FileIndex = -1;

/* flag set M_CFU_SETUP_AWAIT (this must be zero) when setup has been called */

static mCfuSetup_t mCfuSetupFlag[ M_CFU_MAX_FILES ] = { M_CFU_SETUP_AWAIT };

/* pointer to file to be read from */

static FILE * mFilePtr[ M_CFU_MAX_FILES ];

/* buffer holding a line read in */

static char mLineBuffer[ E_CFU_STRING_LEN ];

/* current pointer to buffer held */

static char * mCurrentPtr = NULL;


/*
** Function Name:
**    eCfuSetup
**
** Type:
**    Status_t
**       Returns the status of the function on completion, either SYS_NOMINAL,
**       E_CFU_FILE_ERROR, or E_CFU_NO_COMPLETION.
**
** Purpose:
**    Sets up the configuration file utilities for use on a particular file.
**
** Description:
**    Performs initialisation for configuration file utility functions. Accepts  
**    the name of the file to be read from.
**
** Arguments:
**    char * Filename                  (in)
**       Pointer to string containing the name of the file to be read from.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** History:
**    17-Aug-2000 mjf Use Status_t & SYS_NOMINAL, return during function.
**    08-Aug-2000 mjf Initial creation.
**
*/

Status_t eCfuSetup( const char * Filename )
{
   Status_t Status;                    /* status of function to be returned */ 

   /* initially status is nominal */
   Status = SYS_NOMINAL;

   /* increment the file index, even if setup may subsequently fail  */
   FileIndex++;

   /* check if maximum number of files reached */
   if ( FileIndex >= M_CFU_MAX_FILES )
   {
      return E_CFU_FILES_OVERFLOW;
   }

   /* check that setup is awaited - that any previous use was completed */
   if ( mCfuSetupFlag[ FileIndex ] != M_CFU_SETUP_AWAIT )
   {
      return E_CFU_NO_COMPLETION;
   }

   /* check a NULL-pointer hasn't been supplied */
   if ( Filename == NULL )
   {
      Status = E_CFU_FILE_ERROR;
   }

   /* attempt to open the specified file for reading */
   mFilePtr[ FileIndex ] = fopen( Filename, M_CFU_STR_OPEN );

   /* check for unsuccessful opening */
   if ( mFilePtr[ FileIndex ] == NULL )
   {
      Status = E_CFU_FILE_ERROR;
      mCfuSetupFlag[ FileIndex ] = M_CFU_SETUP_NO_FILE;
   }
   /* else file opened successfully */
   else
   {
      Status = SYS_NOMINAL;
      mCfuSetupFlag[ FileIndex ] = M_CFU_SETUP_FILE_OPEN;
   }

   /* set held buffer to be zero-length string, and current pointer to it */
   mLineBuffer[ 0 ] = M_CFU_CHAR_NULL;
   mCurrentPtr = NULL;
   
   return Status;

}


/*
** Function Name:
**    eCfuComplete
**
** Type:
**    Status_t
**       Returns the status of the function on completion, either SYS_NOMINAL,
**       E_CFU_NO_SETUP or E_CFU_FILE_ERROR.
**
** Purpose:
**    Used to signify reading from a configuration file is complete.
**
** Description:
**    Attempts to close the open file, and sets the setup status as appropriate 
**    to enable the CFU functions to be re-used.
**
** Arguments:
**    None
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** History:
**    17-Aug-2000 mjf Use Status_t & SYS_NOMINAL, return during function.
**    08-Aug-2000 mjf Initial creation.
**
*/

Status_t eCfuComplete( void )
{
   Status_t Status;                    /* status of function to be returned */ 

   /* initially status is nominal */
   Status = SYS_NOMINAL;

   /* if no setup has been performed, return 'no setup' */
   if ( mCfuSetupFlag[ FileIndex ] == M_CFU_SETUP_AWAIT )
   {
      return E_CFU_NO_SETUP;
   }      

   /* set the setup flag for re-use */
   mCfuSetupFlag[ FileIndex ] = M_CFU_SETUP_AWAIT;

   /* check that file pointer indicates file is open */
   if ( mFilePtr[ FileIndex ] != NULL )
   {
      /* close the file, check for success */
      if ( fclose( mFilePtr[ FileIndex ] ) == 0 )
      {
         Status = SYS_NOMINAL;
      }
      /* else file close failed, set status to be file error */
      else
      {
         Status = E_CFU_FILE_ERROR;
      }
   }
   /* else invalid file pointer, set status */
   else
   {
      Status = E_CFU_FILE_ERROR;
   }

   /* set held buffer to be zero-length string, and current pointer to it */
   mLineBuffer[ 0 ] = M_CFU_CHAR_NULL;
   mCurrentPtr = NULL;

   /* decrement the file index to use the pervious file (if any) */
   FileIndex--;

   return Status;

}


/*
** Function Name:
**    eCfuGetLine
**
** Type:
**    Status_t
**       Returns SYS_NOMINAL if there is no error, E_CFU_EOF if there is no line 
**       remaining in the file, E_CFU_FILE_ERROR if a problem occurred accessing 
**       the file, or E_CFU_NO_SETUP if setup has not been performed.
**
** Purpose:
**    Reads a line of text from a configuration file.
**
** Description:
**    Reads or moves onto the next valid line of text from a configuration file. 
**    This line is processed for whitespace and comment markers, and only a line 
**    containing valid characters is deemed successful. Can be used to move onto 
**    the next line of text when retrieving parameters from a file.
**
** Arguments:
**    char * Buffer                    (out)
**       Pointer to buffer for the destination string to be written into, or 
**       NULL. If a pointer is supplied, then the buffer must be of minimum size
**       E_CFU_STRING_LEN. If NULL is specified, then the function will be used 
**       to move onto the next line for the purpose of retrieving parameters.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** History:
**    17-Aug-2000 mjf Use Status_t & SYS_NOMINAL, return during function.
**    08-Aug-2000 mjf Initial creation.
**
*/

Status_t eCfuGetLine( char * Buffer )
{
   Status_t Status;                    /* status of function to be returned */ 
   char   * BufferPtr;                 /* pointer to relevant part of line */
   char   * LinePtr;                   /* pointer to line from config file */

   /* initially status is nominal */
   Status = SYS_NOMINAL;

   /* check that relevant level of setup has been performed */
   if ( mCfuSetupFlag[ FileIndex ] != M_CFU_SETUP_FILE_OPEN )
   {
      return E_CFU_NO_SETUP;
   }

   /* loop until a satisfactory line read, or EOF */
   do
   {
      /* attempt to read a line from the file */
      LinePtr = fgets( mLineBuffer, E_CFU_STRING_LEN, mFilePtr[ FileIndex ] );

      /* if a line read OK */
      if ( LinePtr == mLineBuffer )
      {
         /* trim unwanted characters from the line */
         BufferPtr = mCfuTrimText( mLineBuffer );
 
         /* take copy of pointer to currently held line */
         mCurrentPtr = BufferPtr;

         /* if required, copy useful text into buffer for return */
         if ( Buffer != NULL )
         {
            strcpy( Buffer, BufferPtr );
         }

         /* set relevant status */
         Status = SYS_NOMINAL;
      }
      /* else not read OK, need to determine why */
      else
      {
         /* check for end-of-file */
         if ( feof( mFilePtr[ FileIndex ] ) != 0 )
         {
            Status = E_CFU_EOF;
         }
         /* else error is in file access */
         else
         {
            Status = E_CFU_FILE_ERROR;
         }
      }
   }
   /* note that check for status must be first in line */
   while ( ( Status == SYS_NOMINAL ) && ( strlen( BufferPtr ) == 0 ) );

   return Status;

}                               


/*
** Function Name:
**    eCfuGetParam
**
** Type:
**    Status_t
**       Returns SYS_NOMINAL if there is no error, E_CFU_NO_LINE if a line has 
**       not been read in for parsing, E_CFU_NO_SETUP if setup has not been 
**       performed, or E_CFU_NO_PARAM if there is no further parameter on the 
**       current line.
**
** Purpose:
**    Retrieves a parameter from line in a configuration file.
**
** Description:
**    Attempts to get a parameter from a line in a configuration file. The 
**    parameter is processed for whitespace and comment markers, and only a
**    parameter containing valid characters is deemed successful. The parameters 
**    in a configuration file are separated by commas within a line.
**
** Arguments:
**    char * Buffer                    (out)
**       Pointer to buffer for the destination string to be written into. The  
**       supplied buffer must be of minimum size E_CFU_STRING_LEN.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** History:
**    17-Aug-2000 mjf Use Status_t & SYS_NOMINAL, return during function.
**    08-Aug-2000 mjf Initial creation.
**
*/

Status_t eCfuGetParam( char * Buffer )
{
   Status_t Status;                    /* status of function to be returned */ 
   int      ParamLen;                  /* length of extracted param string */
   char   * EndParamPtr;               /* pointer to end of parameter */

   /* initially status is nominal */
   Status = SYS_NOMINAL;

   /* check that relevant level of setup has been performed */
   if ( mCfuSetupFlag[ FileIndex ] != M_CFU_SETUP_FILE_OPEN )
   {
      return E_CFU_NO_SETUP;
   }

   /* check that a buffer is currently in hand */
   if ( mCurrentPtr != NULL )
   {
      /* check that a further parameter may be on this line */
      if ( *mCurrentPtr != M_CFU_CHAR_NULL )
      {
         /* attempt to find next separator */
         EndParamPtr = strchr( mCurrentPtr, M_CFU_CHAR_SEPARATOR );

         /* if next separator not found, then end pointer is end-of-line */
         if ( EndParamPtr == NULL )
         {
            EndParamPtr = mCurrentPtr + strlen( mCurrentPtr );
         }

         /* determine length of parameter to extract */
         ParamLen = EndParamPtr - mCurrentPtr;

         /* extract from current pointer to end of parameter */
         strncpy( Buffer, mCurrentPtr, ParamLen );

         /* check string is zero-terminated */
         Buffer[ ParamLen ] = M_CFU_CHAR_NULL;

         /* trim unwanted characters from the end */
         mCfuTrimText( Buffer );

         /* for next time point to character beyond param end */
         mCurrentPtr = EndParamPtr + 1;
         /* if text left on this line, trim any unwanted from the start */
         if ( *mCurrentPtr != M_CFU_CHAR_NULL )
         {
            mCurrentPtr = mCfuTrimText( mCurrentPtr );
         }
      }
      /* else nothing more left on this line */
      else
      {
         Status = E_CFU_NO_PARAM;
      }
   }
   /* else no buffer to work with */
   else
   {
      Status = E_CFU_NO_LINE;
   }

   return Status;

}                               


/*
** Function Name:
**    eCfuGetNumParam
**
** Type:
**    Status_t
**       Returns SYS_NOMINAL if there is no error, E_CFU_NO_LINE if a line has 
**       not been read in for parsing, E_CFU_NO_SETUP if setup has not been 
**       performed, or E_CFU_NO_PARAM if there is no suitable parameter on the 
**       current line.
**
** Purpose:
**    Retrieves a numbered parameter from line in a configuration file.
**
** Description:
**    Attempts to get a numbered parameter from a line in a configuration file.
**    The number of the parameter is specified, indexed from 1. The parameter is
**    processed for whitespace and comment markers, and only a parameter 
**    containing valid characters is deemed successful. The parameters in a 
**    configuration file are separated by commas within a line.
**
** Arguments:
**    int ParamNumber                  (in)
**       Number of the required parameter, indexed from 1.
**    char * Buffer                    (out)
**       Pointer to buffer for the destination string to be written into. The  
**       supplied buffer must be of minimum size E_CFU_STRING_LEN.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** History:
**    17-Aug-2000 mjf Use Status_t & SYS_NOMINAL, return during function.
**    08-Aug-2000 mjf Initial creation.
**
*/

Status_t eCfuGetNumParam( int ParamNumber, char * Buffer )
{
   Status_t Status;                    /* status of function to be returned */ 
   int      ParamCount;                /* counter to the retrieval of params */

   /* initially status is nominal */
   Status = SYS_NOMINAL;

   /* check that relevant level of setup has been performed */
   if ( mCfuSetupFlag[ FileIndex ] != M_CFU_SETUP_FILE_OPEN )
   {
      return E_CFU_NO_SETUP;
   }

   /* check that a buffer is currently in hand */
   if ( mCurrentPtr != NULL )
   {
      /* reset buffer pointer to start of line - trim unwanted characters */
      mCurrentPtr = mCfuTrimText( mLineBuffer );

      /* retrieve parameters until the required one was the last one */
      for ( ParamCount = 1; ParamCount <= ParamNumber; ParamCount++ )
      {
         Status = eCfuGetParam( Buffer );
      }
   }
   /* else no buffer to work with */
   else
   {
      Status = E_CFU_NO_LINE;
   }

   return Status;

}


/*
** Function Name:
**    eCfuTestParam
**
** Type:
**    Status_t
**       Returns E_CFU_PARAM_MATCHED if the tested parameter was found, 
**       E_CFU_PARAM_ABSENT if the tested parameter was not found, E_CFU_NO_LINE 
**       if a line has not been read in for parsing, or E_CFU_NO_SETUP if setup 
**       has not been performed.
**
** Purpose:
**    Tests for a supplied parameter within a line in a configuration file.
**
** Description:
**    Tests all the parameters on a line in a configuration file to attempt to 
**    find a match against a supplied parameter. If a match is found, the number
**    of the parameter within the line is returned, else zero is returned.
**
** Arguments:
**    char * Param                     (in)
**       Pointer to string containing parameter to match against.
**    int * Position                   (out)
**       Position number of parameter in line, or zero if a match not found.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** History:
**    17-Aug-2000 mjf Use Status_t & SYS_NOMINAL, return during function.
**    09-Aug-2000 mjf Initial creation.
**
*/

Status_t eCfuTestParam( char * Param, int * Position )
{
   Status_t Status;                    /* status of function to be returned */ 
   Status_t ParamStatus;               /* status of function to get a param */ 
   int      ParamCount;                /* index to parameters in line */
   Bool_t   ParamMatched;              /* flag set TRUE when argument matched */
   char   * CurrentPtrCopy;            /* copy of current pointer to restore */
                                       /* buffer for parameter retrieved */
   char     ParamBuffer[ E_CFU_STRING_LEN ];

   /* initially status is nominal */
   Status = SYS_NOMINAL;

   /* check that relevant level of setup has been performed */
   if ( mCfuSetupFlag[ FileIndex ] != M_CFU_SETUP_FILE_OPEN )
   {
      return E_CFU_NO_SETUP;
   }

   /* check that a buffer is currently in hand */
   if ( mCurrentPtr != NULL )
   {
      /* take copy of current pointer to restore at end of function */
      CurrentPtrCopy = mCurrentPtr;

      /* reset buffer pointer to start of line - trim unwanted characters */
      mCurrentPtr = mCfuTrimText( mLineBuffer );

      /* set initial values before start of loop */
      ParamCount = 0;
      *Position = 0;
      ParamMatched = FALSE;

      /* retrieve parameters looking for match or the end of params */
      do
      {
         /* increment counter in line with parameter number retrieved */
         ParamCount++;
         /* attempt to retrieve a parameter */
         ParamStatus = eCfuGetParam( ParamBuffer );
         /* if parameter retrieved OK */
         if ( ParamStatus == SYS_NOMINAL )
         {
            /* compare against required param */
            if ( strcmp( Param, ParamBuffer ) == 0 )
            {
               ParamMatched = TRUE;
            }
         }
         /* else if first attempt error gets passed back */
         else if ( ParamCount == 1 )
         {
            Status = ParamStatus;
         }
      }
      while( ( ParamStatus == SYS_NOMINAL ) && ( ParamMatched == FALSE ) );

      /* if retrieval at least one parameter was successful */
      if ( Status == SYS_NOMINAL )
      {
         /* if argument was found, then assign parameter number for return */
         if ( ParamMatched == TRUE )
         {
            *Position = ParamCount;
            Status = E_CFU_PARAM_MATCHED;
         }
         /* else parameter not matched, set status */
         else
         {
            Status = E_CFU_PARAM_ABSENT;
         }
      }

      /* restore current pointer to position at start of this function */
      mCurrentPtr = CurrentPtrCopy;
   }
   /* else no buffer to work with */
   else
   {
      Status = E_CFU_NO_LINE;
   }

   return Status;

}


/*
** Function Name:
**    eCfuTestNumParam
**
** Type:
**    Status_t
**       Returns E_CFU_PARAM_MATCHED if the tested parameter was found, 
**       E_CFU_PARAM_ABSENT if the tested parameter was not found, E_CFU_NO_LINE 
**       if a line has not been read in for parsing, or E_CFU_NO_SETUP if setup 
**       has not been performed.
**
** Purpose:
**    Tests for a supplied parameter in a specific position within a line in a 
**    configuration file.
**
** Description:
**    Tests a specific parameter on a line in a configuration file to attempt to 
**    find a match against a supplied parameter. The specific parameter is 
**    specified by the parameter number, and the supplied parameter must be 
**    found in the specified position to achieve a match
**
** Arguments:
**    char * Param                     (in)
**       Pointer to string containing parameter to match against.
**    int Position                     (in)
**       Position number of parameter in line to attempt to match against.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** History:
**    17-Aug-2000 mjf Use Status_t & SYS_NOMINAL, return during function.
**    09-Aug-2000 mjf Initial creation.
**
*/

Status_t eCfuTestNumParam( char * Param, int Position )
{
   Status_t Status;                    /* status of function to be returned */ 
   Status_t ParamStatus;               /* status of function to get a param */ 
   char   * CurrentPtrCopy;            /* copy of current pointer to restore */
                                       /* buffer for parameter retrieved */
   char     ParamBuffer[ E_CFU_STRING_LEN ];

   /* initially status is nominal */
   Status = SYS_NOMINAL;

   /* check that relevant level of setup has been performed */
   if ( mCfuSetupFlag[ FileIndex ] != M_CFU_SETUP_FILE_OPEN )
   {
      return E_CFU_NO_SETUP;
   }

   /* check that a buffer is currently in hand */
   if ( mCurrentPtr != NULL )
   {
      /* take copy of current pointer to restore at end of function */
      CurrentPtrCopy = mCurrentPtr;

      /* reset buffer pointer to start of line - trim unwanted characters */
      mCurrentPtr = mCfuTrimText( mLineBuffer );

      /* retrieve parameter at required position */
      ParamStatus = eCfuGetNumParam( Position, ParamBuffer );

      /* if parameter retrieved correctly */
      if ( ParamStatus == SYS_NOMINAL )
      {
         /* compare retrieved against required param, set return status */
         if ( strcmp( Param, ParamBuffer ) == 0 )
         {
            Status = E_CFU_PARAM_MATCHED;
         }
         else
         {
            Status = E_CFU_PARAM_ABSENT;
         }
      }
      /* else error gets passed back */
      else
      {
         Status = ParamStatus;
      }

      /* restore current pointer to position at start of this function */
      mCurrentPtr = CurrentPtrCopy;
   }
   /* else no buffer to work with */
   else
   {
      Status = E_CFU_NO_LINE;
   }

   return Status;

}


/*
** Function Name:
**    eCfuParamCount
**
** Type:
**    int
**       Returns the number of parameters present on the current line. If there 
**       are no parameters, or even no current line, then zero is returned.
**
** Purpose:
**    Returns the number of parameters on a given line.
**
** Description:
**    Determines the number of parameters present on the current line, or 
**    returns zero if there are no parameters or no current line. This function 
**    can be used to determine if a set of parameters are formed correctly.
**
** Arguments:
**    None
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** History:
**    17-Aug-2000 mjf Use Status_t & SYS_NOMINAL, return during function.
**    09-Aug-2000 mjf Initial creation.
**
*/

int eCfuParamCount( void )
{
   Status_t Status;                    /* status of function to be returned */ 
   Status_t ParamStatus;               /* status of function to get a param */ 
   int      ParamCount;                /* number of parameters found on line */
   char   * CurrentPtrCopy;            /* copy of current pointer to restore */
                                       /* buffer for parameter retrieved */
   char     ParamBuffer[ E_CFU_STRING_LEN ];

   /* initially status is nominal */
   Status = SYS_NOMINAL;

   /* check that relevant level of setup has been performed */
   if ( mCfuSetupFlag[ FileIndex ] != M_CFU_SETUP_FILE_OPEN )
   {
      return 0;
   }

   /* initially no parameters found on line */
   ParamCount = 0;

   /* check that a buffer is currently in hand */
   if ( mCurrentPtr != NULL )
   {
      /* take copy of current pointer to restore at end of function */
      CurrentPtrCopy = mCurrentPtr;

      /* reset buffer pointer to start of line - trim unwanted characters */
      mCurrentPtr = mCfuTrimText( mLineBuffer );

      /* retrieve parameters until end of params */
      do
      {
         /* attempt to retrieve a parameter */
         ParamStatus = eCfuGetParam( ParamBuffer );
         /* if parameter retrieved OK */
         if ( ParamStatus == SYS_NOMINAL )
         {
            /* increment count */
            ParamCount++;
         }
      }
      while( ParamStatus == SYS_NOMINAL );

      /* restore current pointer to position at start of this function */
      mCurrentPtr = CurrentPtrCopy;
   }

   /* return the number of parameters */
   return ParamCount;

}


/*
** Function Name:
**    mCfuTrimText
**
** Type:
**    char *
**       Returns a pointer to the relevant section of the buffer containing the 
**       trimmed text.
**
** Purpose:
**    Used to remove unwanted characters from a textual string.
**
** Description:
**    Removes unwanted characters (carriage-return, comment '#', space and tab)
**    from a string, and returns a pointer to the trimmed section of the buffer.
**
** Arguments:
**    char * TextPtr                   (in)
**       Pointer to text string to have the unwanted characters removed from it.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** History:
**    17-Aug-2000 mjf Use Status_t & SYS_NOMINAL, return during function.
**    08-Aug-2000 mjf Initial creation.
**
*/

static char * mCfuTrimText( char * TextPtr )
{
   char   * CharPtr;                   /* character pointer to parse string */
   char   * UsefulBufferPtr;           /* pointer to useful part of buffer */

   /* initially point to start of buffer */
   UsefulBufferPtr = TextPtr;

   /* attempt to truncate from and including a carriage-return */
   CharPtr = strchr( UsefulBufferPtr, M_CFU_CHAR_CR );
   if ( CharPtr != NULL )
   {
      *CharPtr = M_CFU_CHAR_NULL;
   }

   /* attempt to truncate from and including a comment-starter */
   CharPtr = strchr( UsefulBufferPtr, M_CFU_CHAR_COMMENT );
   if ( CharPtr != NULL )
   {
      *CharPtr = M_CFU_CHAR_NULL;
   }

   /* strip unwanted characters (space and TAB) from the start */
   while ( ( *UsefulBufferPtr == M_CFU_CHAR_SPACE ) 
            || ( *UsefulBufferPtr == M_CFU_CHAR_TAB ) )
   {
      UsefulBufferPtr++;
   }

   /* point to last character in string */
   CharPtr = UsefulBufferPtr + strlen( UsefulBufferPtr ) - 1;

   /* strip unwanted characters (space and TAB) from the end */
   while ( ( *CharPtr == M_CFU_CHAR_SPACE ) 
            || ( *CharPtr == M_CFU_CHAR_TAB ) )
   {
      *CharPtr = M_CFU_CHAR_NULL;
      CharPtr--;
   }

   /* return pointer to the buffer containing the trimmed text */
   return UsefulBufferPtr;

}                               


/*
** EOF
*/
