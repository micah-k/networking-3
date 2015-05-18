#include <iostream>
#include <algorithm>    //fill_n()
#include "UdpSocket.h"
#include "Timer.h"

using namespace std;

int clientStopWait(UdpSocket &sock, const int max, int message[]) {
  cerr << "client: stop-wait test:" << endl;
  int retransmits = 0;
  char c;
  Timer timeout;

  // transfer message[] max times
  for (int i = 0; i < max; i++) {
    message[0] = i;                            // message[0] has a sequence #
    sock.sendTo((char*)message, MSGSIZE); // udp message send
    cerr << "message " << message[0] << endl;

    if(sock.pollRecvFrom()==0)
    {
      timeout.start();
      while(sock.pollRecvFrom()==0)
      {
        if(timeout.lap()>1500)
        {
          retransmits++;
          sock.sendTo( ( char * )message, MSGSIZE ); // udp message send
          cerr << "resending message " << message[0] << endl;
          timeout.start();
        }
      }
    }
    sock.recvFrom(&c, sizeof(c));
    cerr << "acked message " << (int)c << endl;
  }
  return retransmits;
}

void serverReliable(UdpSocket &sock, const int max, int message[]) {
  cerr << "server: reliable test:" << endl;
  char c;

  // receive message[] max times
  for (int i = 0; i < max; i++) {
    sock.recvFrom((char*)message, MSGSIZE);   // udp message receive
    c = (char)message[0];
    sock.ackTo(&c, sizeof(c)); // udp message send
    cerr << c << endl;                     // print out message
  }
}


int clientSlidingWindow(UdpSocket &sock, const int max, int message[], int windowSize) {
  cerr << "client: sliding window test:" << endl;
  int retransmits = 0;
  int lastAck = 0;
  int messages = 0;
  char c;
  Timer timeout;

  // transfer message[] max times
  for (int i = 0; i < max; i++) {
    if (i < lastAck)
    {
      messages--;
      continue;
    }
    while(messages < windowSize && i + messages < max)
    {
      message[0] = i + messages;                            // message[0] has a sequence #
      sock.sendTo((char*)message, MSGSIZE); // udp message send
      cerr << "message " << message[0] << endl;
      messages++;
    }

    if(sock.pollRecvFrom()==0)
    {
      timeout.start();
      while(sock.pollRecvFrom()==0)
      {
        if(timeout.lap()>1500)
        {
          retransmits++;
          message[0] = i;
          sock.sendTo( ( char * )message, MSGSIZE ); // udp message send
          cerr << "resending message " << message[0] << endl;
          timeout.start();
        }
      }
    }
    sock.recvFrom(&c, sizeof(c));
    cerr << "next message: " << (int)c << endl;
    lastAck = (int)c;
  }
  return retransmits;
}

void serverEarlyRetrans(UdpSocket &sock, const int max, int message[], int windowSize) {
  cerr << "server: early retrans test:" << endl;
  bool msgsrecvd[max];
  fill_n(msgsrecvd, max, false); //set all values in msgsrecvd to false
  char c;
  int i = 0;

  // receive message[] max times
  while (i < max) {
    sock.recvFrom((char*)message, MSGSIZE);   // udp message receive
    msgsrecvd[message[0]]=true;
    while(msgsrecvd[i])
    {
      i++;  //move to next unreceived packet
    }
    c = (char)i;
    sock.ackTo(&c, sizeof(c)); // ack with next unreceived packet
    cerr << c << endl;                     // print out message
  }
}