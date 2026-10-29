/*
** Header File Name:
**    Cfu.h
**
** Purpose:
**    Header file for Configuration File Utilities package.
**
** Description:
**    Contains the macros, type definitions and function prototypes needed to 
**    use the Configuration File Utilities (CFU) package.
**
**    This code has been written to conform to "TTL Coding Standard" v0.6.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** Version:
**    $Id: Cfu.h,v 0.6 2002/03/07 17:35:12 mjf Exp $
**
** History:
**    $Log: Cfu.h,v $
**    Revision 0.6  2002/03/07 17:35:12  mjf
**    Increase of maximum line length to 256 characters.
**
**    Revision 0.5  2001/01/18 09:50:27  mjf
**    Filename passed to setup function now 'const'.
**
**    Revision 0.4  2000/12/20 14:19:35  mjf
**    Addition of support to use up to 256 files at once. The reading of the
**    files is nested. Note that an unsuccessful 'setup' must still be
**    'complete'-ed.
**
**    Revision 0.3  2000/08/17 16:36:42  mjf
**    Functions now return Status_t, and SYS_NOMINAL.
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

#ifndef CFU_H_DEFINED
#define CFU_H_DEFINED


/* 
** Macro definitions
*/

/* maximum length of string used to hold a line or resulting text */

#define E_CFU_STRING_LEN     512


/*
** Type definitions
*/

/* enumerated type for return values from Cfu functions */

typedef enum {
   E_CFU_NO_ERR = STATUS_START( CFU ), /* no error - unused */
   E_CFU_PARAM_MATCHED,                /* requested parameter found in line */
   E_CFU_PARAM_ABSENT,                 /* requested parameter not found */
   E_CFU_NO_PARAM,                     /* no further parameter on the line */
   E_CFU_NO_LINE,                      /* no line has been read in for params */
   E_CFU_EOF,                          /* end of file reached */       
   E_CFU_FILE_ERROR,                   /* error accessing configuration file */
   E_CFU_NO_COMPLETION,                /* completion of previous not done */
   E_CFU_NO_SETUP,                     /* setup has not been performed */
   E_CFU_FILES_OVERFLOW                /* attempt to open too many files */
} eCfuStatus_t;


/* 
** Function prototypes
*/

Status_t eCfuSetup           ( const char * Filename );
Status_t eCfuComplete        ( void );
Status_t eCfuGetLine         ( char * Buffer );
Status_t eCfuGetParam        ( char * Buffer );
Status_t eCfuGetNumParam     ( int ParamNumber, char * Buffer );
Status_t eCfuTestParam       ( char * Param, int * Position );
Status_t eCfuTestNumParam    ( char * Param, int Position );
int      eCfuParamCount      ( void );


#endif

/*
** EOF
*/
