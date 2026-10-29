/*
** Header File Name:
**    CfuPrivate.h
**
** Purpose:
**    Private header file for Configuration File Utilities package.
**
** Description:
**    This code has been written to conform to "TTL Coding Standard" v0.6.
**
** Authors:
**    mjf : Martyn J. Ford (TTL)
**
** Version:
**    $Id: CfuPrivate.h,v 0.2 2000/12/20 14:19:35 mjf Exp $
**
** History:
**    $Log: CfuPrivate.h,v $
**    Revision 0.2  2000/12/20 14:19:35  mjf
**    Addition of support to use up to 256 files at once. The reading of the
**    files is nested. Note that an unsuccessful 'setup' must still be
**    'complete'-ed.
**
**    Revision 0.1  2000/08/09 09:08:45  mjf
**    Initial revision.
**
**
*/

#ifndef CFU_PRIVATE_H_DEFINED
#define CFU_PRIVATE_H_DEFINED


/* 
** Macro definitions
*/

/* maximum number of concurrent open files */

#define M_CFU_MAX_FILES      256

/* zero-terminator for string */

#define M_CFU_CHAR_NULL      '\0'

/* separator in configuration file */

#define M_CFU_CHAR_SEPARATOR ','

/* characters to truncate after and including in configuration file */

#define M_CFU_CHAR_CR        '\n'
#define M_CFU_CHAR_COMMENT   '#'
#define M_CFU_CHAR_SPACE     ' '
#define M_CFU_CHAR_TAB       '\t'

/* string used when opening the configuration file */

#define M_CFU_STR_OPEN       "r"


/*
** Type definitions
*/

/* enumerated type for setup stages */

typedef enum
{
   M_CFU_SETUP_AWAIT,                  /* must be 0 to auto-init static array */
   M_CFU_SETUP_FILE_OPEN,
   M_CFU_SETUP_NO_FILE
} mCfuSetup_t;


/* 
** Function prototypes
*/

static char * mCfuTrimText   ( char * TextPtr );


#endif

/*
** EOF
*/
