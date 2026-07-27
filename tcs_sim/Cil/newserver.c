/*
** TESTSERV
**
** A simple program to test Cil functions. It establishes a CIL setup,
** then receives messages and returns them to the sender. It keeps count
** of where messages come from and displays the last received packet.
**
**
** Core CIL functionality is preserved:
**
**    eCilLookup()
**    eCilSetup()
**    eCilReceive()
**    eCilSend()
**    eCilReceiveDynamic()
**    eCilSendDynamic()
**
** Command-line parsing is handled locally in this file so the program
** can be built on Linux without the unavailable CLU/LOG/WFL layers.
**
**
** Original: 23-Mar-2000
*/


#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "TtlSystem.h"
#include "Cil.h"


/* Application details - normally found in XxxPrivate.h */

#define I_XXX_PROGRAM_NAME       "testserver"
#define I_XXX_PROGRAM_ABOUT      "Test CIL message server"
#define I_XXX_RELEASE_DATE       "Local no-CLU build"
#define I_XXX_YEAR               "2026"
#define I_XXX_MAJOR_VERSION      3
#define I_XXX_MINOR_VERSION      0


/*
** Default command-line values.
**
** For local Linux testing, a relative map path is more useful than the
** original TTL deployment paths. Override this with:
**
**    -map /full/path/to/Cil.map
*/

#define M_XXX_DFLT_CIL_MAP       "../Cil.map"
#define M_XXX_DFLT_CIL           "TES"


/* Symbolic constants */

#define MAXSZ                    5
#define COL                      13
#define DFLT_REFRESH_RATE        1
#define DFLT_TIMEOUT             5000

#define M_XXX_MAX_PATH_LEN       256
#define M_XXX_MAX_CIL_NAME_LEN   32
#define M_XXX_DATA_LEN           1024


/* Global variables */

static Bool_t  mTesEcho     = TRUE;
static Int32_t mRefreshRate = DFLT_REFRESH_RATE;
static Int32_t mTimeout     = DFLT_TIMEOUT;
static Bool_t  mDynamic     = FALSE;

static char    mCilMap[ M_XXX_MAX_PATH_LEN ]      = M_XXX_DFLT_CIL_MAP;
static char    mCilName[ M_XXX_MAX_CIL_NAME_LEN ] = M_XXX_DFLT_CIL;


/* Function prototypes */

static void     CrsHome( void );
static void     CrsGoto( int x, int y );
static void     CrsClrToEol( void );
static void     CrsClrScreen( void );
static Status_t mTesParseArgs( int argc, char *argv[] );
static void     mTesUsage( const char *ExecNamePtr, const char *MessagePtr );
static Bool_t   mArgMatches( const char *ArgPtr, const char *NamePtr );
static Status_t mCopyString( char *DestPtr, size_t DestLen,
                             const char *SrcPtr, const char *LabelPtr );
static void     mPrintStatus( const char *TextPtr, Status_t Status );


int main
(
   int argc,
   char *argv[]
)
{
   Status_t MyStatus;
   Status_t Status;
   Int32_t  Id;
   Int32_t  RxId;
   Int32_t  DynamicRxId;
   eCilMsg_t Msg;
   char     Data[ M_XXX_DATA_LEN ];
   int      Count1 = 0;
   int      Count2 = 0;
   int      CountTcs = 0;
   int      Count3 = 0;
   Int32_t  TotalCount = 0;
   Int32_t  SecondTally = 0;
   Int32_t  LastSecondTally = 0;
   Int32_t  SwapVar;
   int      n;
   int      NumTimeouts = 0;
   int      Count[ E_CIL_EOL ];
   time_t   CurrentTime = (time_t) 0;
   time_t   PrevTime = (time_t) 0;


   /* Parse the command line arguments. */
   Status = mTesParseArgs( argc, argv );
   if ( Status != SYS_NOMINAL )
   {
      exit( EXIT_FAILURE );
   }


   /*
   ** Determine the local CIL ID from the selected map file and name.
   */

   Status = eCilLookup( mCilMap, mCilName, &Id );
   if ( Status != SYS_NOMINAL )
   {
      printf
      (
         "Unable to look up local CIL name \"%s\" in map file \"%s\"\n",
         mCilName,
         mCilMap
      );
      mPrintStatus( "eCilLookup", Status );
      exit( EXIT_FAILURE );
   }


   /*
   ** Perform CIL setup. This binds the UDP socket for the selected local
   ** CIL entry.
   */

   Status = eCilSetup( mCilMap, Id );
   if ( Status != SYS_NOMINAL )
   {
      printf
      (
         "Unable to perform CIL setup for local CIL name \"%s\" "
         "(ID=%d) using map file \"%s\"\n",
         mCilName,
         Id,
         mCilMap
      );
      mPrintStatus( "eCilSetup", Status );
      exit( EXIT_FAILURE );
   }


   /* Reset per-source counts. */
   for ( Id = E_CIL_BOL; Id < E_CIL_EOL; Id++ )
   {
      Count[ Id ] = 0;
   }


   /*
   ** SET-UP SCREEN LAYOUT
   */

   CrsClrScreen();
   CrsGoto( 1, 1 );
   printf( "____CIL_TEST_SERVER (%s)____\n", mCilName );
   printf( " Timeouts:\n" );
   printf( " Last sec:\n" );
   printf( "Rx'd from:\n" );
   printf( "       ID:\n" );
   printf( " Num Rx'd:\n" );
   printf( " SourceId:\n" );
   printf( "   DestId:\n" );
   printf( "    Class:\n" );
   printf( "  Service:\n" );
   printf( "   SeqNum:\n" );
   printf( "   TS sec:\n" );
   printf( "  TS nsec:\n" );
   printf( "  DataLen:\n" );
   printf( "     Data:\n" );


   /*
   ** GO INTO THE MAIN LOOP
   */

   for ( ;; )
   {
      Msg.DataPtr = Data;
      Msg.DataLen = sizeof( Data );

      if ( mDynamic == TRUE )
      {
         MyStatus = eCilReceiveDynamic( mTimeout, &DynamicRxId, &Msg );
         if ( MyStatus == SYS_NOMINAL )
         {
            RxId = eCilGetIdDynamic( DynamicRxId );
         }
      }
      else
      {
         MyStatus = eCilReceive( mTimeout, &RxId, &Msg );
      }

      if ( MyStatus == SYS_NOMINAL )
      {
         /* Increment some counters based on where the message source. */
         switch ( Msg.SourceId )
         {
            case E_CIL_TU1 :
               Count1++;
               break;

            case E_CIL_TU2 :
               Count2++;
               break;

            case E_CIL_TU3 :
               CountTcs++;
               break;

            default :
               Count3++;
               break;
         }

         if ( ( RxId >= E_CIL_BOL ) && ( RxId < E_CIL_EOL ) )
         {
            Count[ RxId ]++;
         }

         TotalCount++;
         SecondTally++;

         /* Determine whether to update number of packets in last second. */
         CurrentTime = time( NULL );
         if ( PrevTime != CurrentTime )
         {
            PrevTime = CurrentTime;
            LastSecondTally = SecondTally;
            SecondTally = 0;
         }

         /* If whole number of refresh rate received, update the screen. */
         if ( ( TotalCount % mRefreshRate ) == 0 )
         {
            CrsGoto( COL,  2 );  printf( "(%d)", NumTimeouts );
            CrsGoto( COL,  3 );  printf( "%5d", LastSecondTally );
            CrsGoto( COL,  4 );  printf( "%s", eCilNameString( RxId ) );
            CrsGoto( COL,  5 );  printf( "%d ", RxId );
            CrsGoto( COL,  6 );  printf( "%d", Count[ RxId ] );
            CrsGoto( COL,  7 );  printf( "0x%8.8x = %s", Msg.SourceId,
                                         eCilNameString( Msg.SourceId ) );
            CrsGoto( COL,  8 );  printf( "0x%8.8x = %s", Msg.DestId,
                                         eCilNameString( Msg.DestId ) );
            CrsGoto( COL,  9 );  printf( "0x%8.8x", Msg.Class );
            CrsGoto( COL, 10 );  printf( "0x%8.8x", Msg.Service );
            CrsGoto( COL, 11 );  printf( "0x%8.8x", Msg.SeqNum );
            CrsGoto( COL, 12 );  printf( "0x%8.8x", Msg.TimeStamp.t_sec );
            CrsGoto( COL, 13 );  printf( "0x%8.8x", Msg.TimeStamp.t_nsec );
            CrsGoto( COL, 14 );  printf( "0x%8.8x = %d",
                                         (unsigned int) Msg.DataLen,
                                         (int) Msg.DataLen );
            CrsClrToEol();
            CrsGoto( COL, 15 );

            for ( n = 0; n < (int) Msg.DataLen; n++ )
            {
               unsigned char Ch = (unsigned char) Data[ n ];

               if ( Ch == '\n' )
               {
                  printf( "\n            " );
               }
               else if ( Ch == '\r' )
               {
                  printf( "\\r" );
               }
               else if ( Ch == '\t' )
               {
                  printf( "   " );
               }
               else if ( isprint( Ch ) )
               {
                  putchar( Ch );
               }
               else
               {
                  putchar( '.' );
               }
            }
            printf( " End" );
            CrsClrToEol();
         }


         /* Swap source and destination addresses. */
         SwapVar = Msg.SourceId;
         Msg.SourceId = Msg.DestId;
         Msg.DestId = SwapVar;


         /* Send message back again if we are echoing. */
         if ( mTesEcho == TRUE )
         {
            if ( mDynamic == TRUE )
            {
               MyStatus = eCilSendDynamic( DynamicRxId, &Msg );
            }
            else
            {
               MyStatus = eCilSend( RxId, &Msg );
            }

            if ( MyStatus != SYS_NOMINAL )
            {
               printf( "Send failure\n" );
               mPrintStatus( "eCilSend", MyStatus );
               return 3;
            }
         }
      }
      else if ( MyStatus == E_CIL_TIMEOUT )
      {
         NumTimeouts++;
         CrsGoto( COL,  2 );
         printf( "%d", NumTimeouts );
         CrsClrToEol();
         CrsGoto( COL, 15 );
      }
      else
      {
         printf( "Receive failure\n" );
         mPrintStatus( "eCilReceive", MyStatus );
         return 4;
      }
   }
}


static void CrsHome( void )
{
   static char EscSequence[] = " [?6h";

   EscSequence[ 0 ] = 27;
   puts( EscSequence );
}


static void CrsGoto
(
   int x,
   int y
)
{
   printf( "%c[%d;%df", 27, y, x );
}


static void CrsClrToEol( void )
{
   static char EscSequence[] = " [K";

   EscSequence[ 0 ] = 27;
   puts( EscSequence );
}


static void CrsClrScreen( void )
{
   static char EscSequence[] = " [2J";

   EscSequence[ 0 ] = 27;
   puts( EscSequence );
   CrsHome();
}


static Status_t mTesParseArgs
(
   int argc,
   char *argv[]
)
{
   int Arg;


   for ( Arg = 1; Arg < argc; Arg++ )
   {
      if ( mArgMatches( argv[ Arg ], "help" ) ||
           mArgMatches( argv[ Arg ], "h" ) )
      {
         mTesUsage( argv[ 0 ], NULL );
         exit( EXIT_SUCCESS );
      }

      else if ( mArgMatches( argv[ Arg ], "map" ) )
      {
         if ( ( Arg + 1 ) >= argc )
         {
            mTesUsage( argv[ 0 ], "Missing parameter after -map" );
            return SYS_ERROR_FATAL;
         }

         Arg++;
         if ( mCopyString( mCilMap, sizeof( mCilMap ), argv[ Arg ],
                           "CIL map" ) != SYS_NOMINAL )
         {
            return SYS_ERROR_FATAL;
         }
      }

      else if ( mArgMatches( argv[ Arg ], "cil" ) )
      {
         if ( ( Arg + 1 ) >= argc )
         {
            mTesUsage( argv[ 0 ], "Missing parameter after -cil" );
            return SYS_ERROR_FATAL;
         }

         Arg++;
         if ( mCopyString( mCilName, sizeof( mCilName ), argv[ Arg ],
                           "local CIL name" ) != SYS_NOMINAL )
         {
            return SYS_ERROR_FATAL;
         }
      }

      else if ( mArgMatches( argv[ Arg ], "noecho" ) )
      {
         mTesEcho = FALSE;
      }

      else if ( mArgMatches( argv[ Arg ], "refresh" ) )
      {
         if ( ( Arg + 1 ) >= argc )
         {
            mTesUsage( argv[ 0 ], "Missing parameter after -refresh" );
            return SYS_ERROR_FATAL;
         }

         Arg++;
         mRefreshRate = (Int32_t) strtol( argv[ Arg ], NULL, 0 );
         if ( mRefreshRate <= 0 )
         {
            mRefreshRate = DFLT_REFRESH_RATE;
         }
      }

      else if ( mArgMatches( argv[ Arg ], "timeout" ) )
      {
         if ( ( Arg + 1 ) >= argc )
         {
            mTesUsage( argv[ 0 ], "Missing parameter after -timeout" );
            return SYS_ERROR_FATAL;
         }

         Arg++;
         mTimeout = (Int32_t) strtol( argv[ Arg ], NULL, 0 );
         if ( mTimeout < 0 )
         {
            mTimeout = DFLT_TIMEOUT;
         }
      }

      else if ( mArgMatches( argv[ Arg ], "dynamic" ) )
      {
         mDynamic = TRUE;
      }

      else
      {
         mTesUsage( argv[ 0 ], "Unknown argument" );
         printf( "Argument: %s\n", argv[ Arg ] );
         return SYS_ERROR_FATAL;
      }
   }

   return SYS_NOMINAL;
}


static void mTesUsage
(
   const char *ExecNamePtr,
   const char *MessagePtr
)
{
   if ( MessagePtr != NULL )
   {
      printf( "%s\n\n", MessagePtr );
   }

   printf( "%s - %s\n", I_XXX_PROGRAM_NAME, I_XXX_PROGRAM_ABOUT );
   printf( "Usage:\n" );
   printf
   (
      "  %s [-map <file>] [-cil <name>] [-noecho] [-refresh <rate>] "
      "[-timeout <msec>] [-dynamic]\n",
      ExecNamePtr
   );
   printf( "\n" );
   printf( "Defaults:\n" );
   printf( "  map      : %s\n", M_XXX_DFLT_CIL_MAP );
   printf( "  cil      : %s\n", M_XXX_DFLT_CIL );
   printf( "  refresh  : %d\n", DFLT_REFRESH_RATE );
   printf( "  timeout  : %d msec\n", DFLT_TIMEOUT );
   printf( "\n" );
}


static Bool_t mArgMatches
(
   const char *ArgPtr,
   const char *NamePtr
)
{
   if ( ( ArgPtr == NULL ) || ( NamePtr == NULL ) )
   {
      return FALSE;
   }

   while ( *ArgPtr == '-' )
   {
      ArgPtr++;
   }

   if ( strcmp( ArgPtr, NamePtr ) == 0 )
   {
      return TRUE;
   }

   return FALSE;
}


static Status_t mCopyString
(
   char       *DestPtr,
   size_t      DestLen,
   const char *SrcPtr,
   const char *LabelPtr
)
{
   if ( ( DestPtr == NULL ) || ( SrcPtr == NULL ) || ( LabelPtr == NULL ) )
   {
      return SYS_ERROR_FATAL;
   }

   if ( strlen( SrcPtr ) >= DestLen )
   {
      printf( "%s is too long: %s\n", LabelPtr, SrcPtr );
      return SYS_ERROR_FATAL;
   }

   strcpy( DestPtr, SrcPtr );

   return SYS_NOMINAL;
}


static void mPrintStatus
(
   const char *TextPtr,
   Status_t   Status
)
{
   printf
   (
      "%s status: decimal=%d hex=0x%08X low=%d lowhex=0x%04X\n",
      TextPtr,
      Status,
      (unsigned int) Status,
      (int) ( Status & 0xffff ),
      (unsigned int) ( Status & 0xffff )
   );
}


/* EOF */
