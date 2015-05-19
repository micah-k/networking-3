#include <iostream>
#include "UdpSocket.h"
#include "Timer.h"

using namespace std;

#define PORT 9649        // my UDP port
#define MAX 20000        // times of message transfer
#define MAXWIN 30        // the maximum window size
#define LOOP 10          // loop in test 4 and 5
#define MAXN 10          // maximum chance of packet failure

// client packet sending functions
void clientUnreliable( UdpSocket &sock, const int max, int message[] );
int clientStopWait( UdpSocket &sock, const int max, int message[] );
int clientSlidingWindow( UdpSocket &sock, const int max, int message[], 
        int windowSize );
//int clientSlowAIMD( UdpSocket &sock, const int max, int message[],
//         int windowSize, bool rttOn );

// server packet receiving fucntions
void serverUnreliable( UdpSocket &sock, const int max, int message[] );
void serverReliable( UdpSocket &sock, const int max, int message[] );
void serverEarlyRetrans( UdpSocket &sock, const int max, int message[], 
       int n );
//void serverEarlyRetrans( UdpSocket &sock, const int max, int message[], 
//       int windowSize, bool congestion );

enum myPartType { CLIENT, SERVER, ERROR } myPart;

int main( int argc, char *argv[] ) {

  int message[MSGSIZE/4]; // prepare a 1460-byte message: 1460/4 = 365 ints;
  UdpSocket sock( PORT );  // define a UDP socket

  myPart = ( argc == 1 ) ? SERVER : CLIENT;

  if ( argc != 1 && argc != 2 ) {
    cerr << "usage: " << argv[0] << " [serverIpName]" << endl;
    return -1;
  }

  if ( myPart == CLIENT ) // I am a client and thus set my server address
    if ( sock.setDestAddress( argv[1] ) == false ) {
      cerr << "cannot find the destination IP name: " << argv[1] << endl;
      return -1;
    }

  int windowSize;
  cerr << "Choose a window size" << endl;
  cerr << "   1: 1 packet window" << endl;
  cerr << "   2: 30 packet window" << endl;
  cerr << "--> ";
  cin >> windowSize;

  if ( myPart == CLIENT ) {

    Timer timer;           // define a timer
    int retransmits = 0;   // # retransmissions

    switch( windowSize ) {
    case 1:
      for ( int n = 0; n <= MAXN; n++ ) {
        timer.start( );                                         // start timer
        retransmits =
        clientSlidingWindow( sock, MAX, message, 1 );           // actual test
        cerr << "Chance of failure = ";
        cout << n << " ";
        cerr << "Elasped time = "; 
        cout << timer.lap( ) << endl;                           // lap timer
        cerr << "retransmits = " << retransmits << endl;
      }
      break;
    case 2:
      for ( int n = 0; n <= MAXN; n++ ) {
        timer.start( );                                         // start timer
        retransmits =
        clientSlidingWindow( sock, MAX, message, 30 );          // actual test
        cerr << "Chance of failure = ";
        cout << n << " ";
        cerr << "Elasped time = "; 
        cout << timer.lap( ) << endl;                           // lap timer
        cerr << "retransmits = " << retransmits << endl;
      }
      break;
    default:
      cerr << "invalid selection" << endl;
      break;
    }
  }
  if ( myPart == SERVER ) {
    switch( windowSize ) {
    case 1:
      for ( int n = 1; n <= MAXN; n++ )
        serverEarlyRetrans( sock, MAX, message, n );
      break;
    case 2:
      for ( int n = 1; n <= MAXN; n++ )
        serverEarlyRetrans( sock, MAX, message, n );
      break;
    default:
      cerr << "invalid selection" << endl;
      break;
    }

    // The server should make sure that the last ack has been delivered to
    // the client. Send it three time in three seconds
    cerr << "server ending..." << endl;
    for ( int i = 0; i < 10; i++ ) {
      sleep( 1 );
      int ack = MAX - 1;
      sock.ackTo( (char *)&ack, sizeof( ack ) );
    }
  }

  cerr << "finished" << endl;

  return 0;
}

// Test 1: client unreliable message send -------------------------------------
void clientUnreliable( UdpSocket &sock, const int max, int message[] ) {
  cerr << "client: unreliable test:" << endl;

  // transfer message[] max times
  for ( int i = 0; i < max; i++ ) {
    message[0] = i;                            // message[0] has a sequence #
    sock.sendTo( ( char * )message, MSGSIZE ); // udp message send
    cerr << "message = " << message[0] << endl;
  }
}

// Test1: server unreliable message receive -----------------------------------
void serverUnreliable( UdpSocket &sock, const int max, int message[] ) {
  cerr << "server unreliable test:" << endl;

  // receive message[] max times
  for ( int i = 0; i < max; i++ ) {
    sock.recvFrom( ( char * ) message, MSGSIZE );   // udp message receive
    cerr << message[0] << endl;                     // print out message
  }
}
