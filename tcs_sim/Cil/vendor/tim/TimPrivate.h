/*
** Header File Name:
**    TimPrivate.h
**
** Purpose:
**    Private header file for TTL System Time (TIM) functions.
**
** Description:
**    ...
**
** Authors:
**    mkb: Mark K. Bowman
**    djm: Derek J. McKay
**
** Version:
**    $Id: TimPrivate.h,v 0.3 2000/06/02 13:10:29 djm Exp $
**
** History:
**    $Log: TimPrivate.h,v $
**    Revision 0.3  2000/06/02 13:10:29  djm
**    Added nanosecond to second conversion.
**
**    Revision 0.2  2000/05/26 08:17:15  djm
**    Adaptation from original VMS code.
**
**
**    Revision 0.1  2000/05/05 ??:??:??  mkb
**    Initial creation (under OpenVMS, as part of the TCS software).
**
**
*/


#ifndef TIMPRIVATE_H_DEFINED
#define TIMPRIVATE_H_DEFINED



/* Required include files */

#include "TtlSystem.h"
#include "Tim.h"


/* Symbolic constants */

#define I_TIM_NSEC_PER_SEC 1000000000


#endif


/* EOF */