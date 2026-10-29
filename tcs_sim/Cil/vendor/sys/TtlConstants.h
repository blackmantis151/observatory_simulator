/*
**       Copyright (c) Telescope Technologies Limited, 2000
**
** Header File Name:
**    TtlConstants.h
**
** Purpose:
**    Public header file for converting between various unit types.
**
** Description:
**    This header file contains a number of symbolic constants that
**    may be used to convert between different unit types. The
**    constants are all defined as floating point constants, even if
**    they may be expressed as integer quantities. Constants that are
**    used to convert between different unit types are expressed as
**    Unit1 per Unit2 (UNIT1_PER_UNIT2) in order to ensure that the
**    constant correctly self-describes itself. The reciprocal
**    conversion is done using 1.0 divided by the predefined
**    conversion. This is best done for when the first conversion is
**    an integer (even though it is expressed as a double).
**
**    Abbreviations used in the constants:
**       DEG    = Degree
**       MAS    = Milliarcsecond
**       AS     = Arcsecond
**       TURN   = Turn(s) (1 turn = 1 revolution = 360 degrees)
**       VEL    = Velocity (e.g. VEL_LIGHT = velocity of light)
**
**    This code has been written to conform to the "TTL Coding
**    Standard"  v0.4 29-Sep-1998.
**
** Authors:
**    djm: Derek J. McKay
**
** Version:
**    $Id: TtlConstants.h,v 0.12 2005/06/20 13:43:35 mjf Exp $
**
** History:
**    $Log: TtlConstants.h,v $
**    Revision 0.12  2005/06/20 13:43:35  mjf
**    Tidied to aid porting to Linux - no functional changes.
**
**    Revision 0.11  2001/10/23 13:12:36  djm
**    Added new constants.
**
**    Revision 0.10  2001/07/17 10:21:51  djm
**    Added additional angular unit conversions.
**
**    Revision 0.9  2001/01/02 12:08:37  mjf
**    Addition of scaling factors for microns.
**
**    Revision 0.8  2000/12/09 15:05:15  mjf
**    Correction to definition of E_TTL_TURNS_PER_MAS.
**
**    Revision 0.7  2000/11/14 11:44:21  djm
**    Added more time constants.
**
**    Revision 0.6  2000/09/26 13:45:20  mjf
**    Correction to E_TT_MILLISECS_PER_RAD - was incorrect by a factor of
**    PI. Also this group of scaling factors converted to floating point.
**
**    Revision 0.5  2000/09/19 07:33:19  mjf
**    Addition of scaling factors and times (milliseconds).
**
**    Revision 0.4  2000/06/12 10:24:07  rdl
**    Addition of Integer Rounding Up constants.
**
**    Revision 0.3  2000/06/12 10:08:42  rdl
**    Addition of Time Scale Factor constants.
**
**    Revision 0.2  2000/03/07 17:20:31  djm
**    Changed source of PI definition.
**    Slight alteration to format.
**    Added velocity of light (in m/s).
**
**    Revision 0.1  2000/03/07 16:44:09  djm
**    Initial creation.
**
**
*/


#ifndef TTLCONSTANTS_H_DEFINED
#define TTLCONSTANTS_H_DEFINED



/* Universal constants */

#define E_TTL_PI              3.141592653589793238
#define E_TTL_TWOPI           (E_TTL_PI * 2.0)


/* SI unit scale factors */


#define E_TTL_UNIT_PER_GIGA          1000000000.0
#define E_TTL_UNIT_PER_MEGA             1000000.0
#define E_TTL_UNIT_PER_KILO                1000.0


#define E_TTL_CENTI_PER_UNIT                100.0
#define E_TTL_MILLI_PER_UNIT               1000.0
#define E_TTL_MICRO_PER_UNIT            1000000.0
#define E_TTL_NANO_PER_UNIT          1000000000.0


/* Fractional unit scale factors */

#define E_TTL_PERCENT_PER_UNIT           ( E_TTL_CENTI_PER_UNIT )
#define E_TTL_MILLIPERCENT_PER_UNIT      ( E_TTL_MILLI_PER_UNIT * E_TTL_PERCENT_PER_UNIT )


/* Stated angular scale factors */

#define E_TTL_AS_PER_AMIN     60.0
#define E_TTL_AMIN_PER_DEG    60.0
#define E_TTL_DEG_PER_TURN    360.0


/* Angular quantity conversions */

#define E_TTL_TURNS_PER_DEG   (1.0 / E_TTL_DEG_PER_TURN)

#define E_TTL_MAS_PER_AS      (E_TTL_MILLI_PER_UNIT)
#define E_TTL_AS_PER_MAS      (1.0 / E_TTL_MAS_PER_AS)

#define E_TTL_AS_PER_DEG      (E_TTL_AS_PER_AMIN * E_TTL_AMIN_PER_DEG)
#define E_TTL_DEG_PER_AS      (1.0 / E_TTL_AS_PER_DEG)

#define E_TTL_MAS_PER_TURN    (E_TTL_MAS_PER_AS \
                               * E_TTL_AS_PER_DEG \
                               * E_TTL_DEG_PER_TURN)
#define E_TTL_TURNS_PER_MAS   (1.0 / E_TTL_MAS_PER_TURN)

#define E_TTL_MAS_PER_DEG     (E_TTL_MAS_PER_AS * E_TTL_AS_PER_DEG)
#define E_TTL_DEG_PER_MAS     (1.0 / E_TTL_MAS_PER_DEG)

#define E_TTL_DEG_PER_RAD     (E_TTL_DEG_PER_TURN / E_TTL_TWOPI)
#define E_TTL_RAD_PER_DEG     (1.0 / E_TTL_DEG_PER_RAD)

#define E_TTL_MAS_PER_RAD     (E_TTL_MAS_PER_DEG * E_TTL_DEG_PER_RAD)
#define E_TTL_RAD_PER_MAS     (1.0 / E_TTL_MAS_PER_RAD)

#define E_TTL_AS_PER_RAD      (E_TTL_AS_PER_DEG * E_TTL_DEG_PER_RAD)
#define E_TTL_RAD_PER_AS      (1.0 / E_TTL_AS_PER_RAD)


/* Physical constants (in SI units) */

#define E_TTL_VEL_LIGHT       299792458.0


/* Stated time scale factors */

#define E_TTL_SECS_PER_MIN                  60.0
#define E_TTL_MINS_PER_HOUR                 60.0
#define E_TTL_HOURS_PER_DAY                 24.0

/* Derived time scale factors */

#define E_TTL_MILLISECS_PER_SEC     ( E_TTL_MILLI_PER_UNIT )
#define E_TTL_MICROSECS_PER_SEC     ( E_TTL_MICRO_PER_UNIT )
#define E_TTL_NANOSECS_PER_SEC      ( E_TTL_NANO_PER_UNIT )
#define E_TTL_NANOSECS_PER_MILLISEC ( E_TTL_NANOSECS_PER_SEC / E_TTL_MILLISECS_PER_SEC )

#define E_TTL_SECS_PER_HOUR  ( E_TTL_SECS_PER_MIN  * E_TTL_MINS_PER_HOUR )
#define E_TTL_SECS_PER_DAY   ( E_TTL_SECS_PER_HOUR * E_TTL_HOURS_PER_DAY )


/* Common times in milli-seconds (integers) */

#define E_TTL_MILLISEC_PER_TWENTIETH_SEC ( (int) E_TTL_MILLISECS_PER_SEC / 20 )
#define E_TTL_MILLISEC_PER_QUARTER_SEC   ( (int) E_TTL_MILLISECS_PER_SEC /  4 )
#define E_TTL_MILLISEC_PER_HALF_SEC      ( (int) E_TTL_MILLISECS_PER_SEC /  2 )
#define E_TTL_MILLISEC_PER_ONE_SEC       ( (int) E_TTL_MILLISECS_PER_SEC      )
#define E_TTL_MILLISEC_PER_TWO_SEC       ( (int) E_TTL_MILLISECS_PER_SEC *  2 )
#define E_TTL_MILLISEC_PER_FIVE_SEC      ( (int) E_TTL_MILLISECS_PER_SEC *  5 )
#define E_TTL_MILLISEC_PER_TEN_SEC       ( (int) E_TTL_MILLISECS_PER_SEC * 10 )
#define E_TTL_MILLISEC_PER_HALF_MINUTE   ( (int) E_TTL_MILLISECS_PER_SEC * 30 )
#define E_TTL_MILLISEC_PER_MINUTE        ( (int) E_TTL_MILLISECS_PER_SEC * 60 )


/* Scaling factors */

#define E_TTL_MILLISECS_PER_DAY       (   E_TTL_MILLI_PER_UNIT \
                                        * E_TTL_SECS_PER_MIN   \
                                        * E_TTL_MINS_PER_HOUR  \
                                        * E_TTL_HOURS_PER_DAY )
                                          

#define E_TTL_MILLIMETRES_PER_METRE         (E_TTL_MILLI_PER_UNIT)
#define E_TTL_MILLIMETRES_PER_KILOMETRE     (E_TTL_MILLI_PER_UNIT \
                                              * E_TTL_UNIT_PER_KILO)
#define E_TTL_METRES_PER_KILOMETRE          (E_TTL_UNIT_PER_KILO)
#define E_TTL_MILLISECS_PER_RAD             (E_TTL_MILLI_PER_UNIT \
                                             * E_TTL_SECS_PER_DAY \
                                             / E_TTL_TWOPI)

#define E_TTL_MICRONS_PER_METRE             (E_TTL_MICRO_PER_UNIT)
#define E_TTL_MICRONS_PER_MILLIMETRES       (E_TTL_MICRO_PER_UNIT \
                                             / E_TTL_MILLI_PER_UNIT)



/* Astronomical constants */




/* Miscellaneous conversion constant */

#define E_TTL_INT_ROUND_UP                   0.5

#endif


/* EOF */
