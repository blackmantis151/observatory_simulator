/* Native CIL transport bridge. It never interprets or echoes payloads. */

#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "TtlSystem.h"
#include "Cil.h"

#define FRAME_VERSION 1u
#define FRAME_TX      1u
#define FRAME_RX      2u
#define FRAME_READY   3u
#define FRAME_WORDS   10u
#define FRAME_HEADER_BYTES ( FRAME_WORDS * sizeof( uint32_t ) )
#define MAX_FRAME_BYTES ( 1024u * 1024u )
#define RX_BYTES 65536u

static volatile sig_atomic_t g_stop = 0;

static void on_signal( int signal_number )
{
   (void) signal_number;
   g_stop = 1;
}

static int read_exact( int fd, void *buffer, size_t size )
{
   unsigned char *cursor = buffer;
   size_t done = 0;
   while ( done < size )
   {
      ssize_t count = read( fd, cursor + done, size - done );
      if ( count == 0 ) return 0;
      if ( count < 0 )
      {
         if ( errno == EINTR ) continue;
         return -1;
      }
      done += (size_t) count;
   }
   return 1;
}

static int write_exact( int fd, const void *buffer, size_t size )
{
   const unsigned char *cursor = buffer;
   size_t done = 0;
   while ( done < size )
   {
      ssize_t count = write( fd, cursor + done, size - done );
      if ( count < 0 )
      {
         if ( errno == EINTR ) continue;
         return -1;
      }
      done += (size_t) count;
   }
   return 0;
}

static void put_word( unsigned char *header, size_t index, uint32_t value )
{
   uint32_t network_value = htonl( value );
   memcpy( header + index * sizeof( network_value ),
           &network_value, sizeof( network_value ) );
}

static uint32_t get_word( const unsigned char *header, size_t index )
{
   uint32_t network_value;
   memcpy( &network_value, header + index * sizeof( network_value ),
           sizeof( network_value ) );
   return ntohl( network_value );
}

static int write_frame( uint32_t kind, Int32_t route_id,
                        const eCilMsg_t *message )
{
   uint32_t body_size = FRAME_HEADER_BYTES + (uint32_t) message->DataLen;
   uint32_t network_size = htonl( body_size );
   unsigned char header[ FRAME_HEADER_BYTES ];

   if ( body_size > MAX_FRAME_BYTES ) return -1;
   put_word( header, 0, FRAME_VERSION );
   put_word( header, 1, kind );
   put_word( header, 2, (uint32_t) route_id );
   put_word( header, 3, (uint32_t) message->SourceId );
   put_word( header, 4, (uint32_t) message->DestId );
   put_word( header, 5, (uint32_t) message->Class );
   put_word( header, 6, (uint32_t) message->Service );
   put_word( header, 7, message->SeqNum );
   put_word( header, 8, (uint32_t) message->TimeStamp.t_sec );
   put_word( header, 9, (uint32_t) message->TimeStamp.t_nsec );

   if ( write_exact( STDOUT_FILENO, &network_size, sizeof( network_size ) ) != 0 ||
        write_exact( STDOUT_FILENO, header, sizeof( header ) ) != 0 )
   {
      return -1;
   }
   if ( message->DataLen > 0 &&
        write_exact( STDOUT_FILENO, message->DataPtr, message->DataLen ) != 0 )
   {
      return -1;
   }
   return 0;
}

static int process_tx_frame( Int32_t local_id )
{
   uint32_t network_size;
   uint32_t body_size;
   unsigned char *body;
   eCilMsg_t message;
   Int32_t route_id;
   Status_t status;
   int read_status;

   read_status = read_exact( STDIN_FILENO, &network_size, sizeof( network_size ) );
   if ( read_status <= 0 ) return read_status;
   body_size = ntohl( network_size );
   if ( body_size < FRAME_HEADER_BYTES || body_size > MAX_FRAME_BYTES )
   {
      fprintf( stderr, "Invalid bridge frame size: %u\n", body_size );
      return -1;
   }
   body = malloc( body_size );
   if ( body == NULL ) return -1;
   read_status = read_exact( STDIN_FILENO, body, body_size );
   if ( read_status <= 0 )
   {
      free( body );
      return read_status;
   }
   if ( get_word( body, 0 ) != FRAME_VERSION || get_word( body, 1 ) != FRAME_TX )
   {
      fprintf( stderr, "Invalid bridge TX frame header\n" );
      free( body );
      return -1;
   }

   memset( &message, 0, sizeof( message ) );
   route_id = (Int32_t) get_word( body, 2 );
   message.SourceId = (Int32_t) get_word( body, 3 );
   message.DestId = (Int32_t) get_word( body, 4 );
   message.Class = (Int32_t) get_word( body, 5 );
   message.Service = (Int32_t) get_word( body, 6 );
   message.SeqNum = get_word( body, 7 );
   message.TimeStamp.t_sec = (Int32_t) get_word( body, 8 );
   message.TimeStamp.t_nsec = (Int32_t) get_word( body, 9 );
   message.DataPtr = body + FRAME_HEADER_BYTES;
   message.DataLen = body_size - FRAME_HEADER_BYTES;

   if ( message.SourceId != local_id )
   {
      fprintf( stderr, "Rejected TX frame with foreign SourceId=%d local=%d\n",
               (int) message.SourceId, (int) local_id );
      free( body );
      return -1;
   }
   status = eCilSend( route_id, &message );
   free( body );
   if ( status != SYS_NOMINAL )
   {
      fprintf( stderr, "eCilSend failed: status=%d\n", (int) status );
      return -1;
   }
   return 1;
}

static int parse_options( int argc, char **argv, const char **map_path,
                          const char **name, int *timeout_ms )
{
   int index;
   *map_path = NULL;
   *name = NULL;
   *timeout_ms = 20;
   for ( index = 1; index < argc; index++ )
   {
      if ( strcmp( argv[ index ], "--map" ) == 0 && index + 1 < argc )
         *map_path = argv[ ++index ];
      else if ( strcmp( argv[ index ], "--name" ) == 0 && index + 1 < argc )
         *name = argv[ ++index ];
      else if ( strcmp( argv[ index ], "--timeout-ms" ) == 0 && index + 1 < argc )
         *timeout_ms = atoi( argv[ ++index ] );
      else
         return -1;
   }
   return ( *map_path != NULL && *name != NULL && *timeout_ms > 0 ) ? 0 : -1;
}

int main( int argc, char **argv )
{
   const char *map_path;
   const char *name;
   int timeout_ms;
   Int32_t local_id;
   Status_t status;
   eCilMsg_t ready;

   if ( parse_options( argc, argv, &map_path, &name, &timeout_ms ) != 0 )
   {
      fprintf( stderr, "Usage: %s --map PATH --name CIL_NAME [--timeout-ms N]\n",
               argv[ 0 ] );
      return EXIT_FAILURE;
   }
   signal( SIGINT, on_signal );
   signal( SIGTERM, on_signal );
   status = eCilLookup( map_path, name, &local_id );
   if ( status == SYS_NOMINAL ) status = eCilSetup( map_path, local_id );
   if ( status != SYS_NOMINAL )
   {
      fprintf( stderr, "CIL setup failed for %s: status=%d\n", name, (int) status );
      return EXIT_FAILURE;
   }

   memset( &ready, 0, sizeof( ready ) );
   ready.SourceId = local_id;
   ready.DestId = local_id;
   if ( write_frame( FRAME_READY, local_id, &ready ) != 0 ) return EXIT_FAILURE;

   while ( !g_stop )
   {
      struct pollfd input = { STDIN_FILENO, POLLIN | POLLHUP, 0 };
      unsigned char rx_data[ RX_BYTES ];
      eCilMsg_t message;
      Int32_t rx_id = 0;
      int poll_status = poll( &input, 1, 0 );

      if ( poll_status < 0 && errno != EINTR ) break;
      if ( poll_status > 0 && ( input.revents & POLLIN ) )
      {
         int tx_status = process_tx_frame( local_id );
         if ( tx_status <= 0 ) break;
      }
      else if ( poll_status > 0 && ( input.revents & POLLHUP ) )
      {
         break;
      }

      memset( &message, 0, sizeof( message ) );
      message.DataPtr = rx_data;
      message.DataLen = sizeof( rx_data );
      status = eCilReceive( timeout_ms, &rx_id, &message );
      if ( status == E_CIL_TIMEOUT ) continue;
      if ( status != SYS_NOMINAL )
      {
         fprintf( stderr, "eCilReceive failed: status=%d\n", (int) status );
         break;
      }
      if ( write_frame( FRAME_RX, rx_id, &message ) != 0 ) break;
   }
   return EXIT_SUCCESS;
}
