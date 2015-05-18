#include <iostream>
#include <algorithm>    //fill_n()
#include "UdpSocket.h"
#include "Timer.h"

using namespace std;

int clientStopWait(UdpSocket &sock, const int max, int message[]) {
  cerr << "client: stop-wait test:" << endl;
  int retransmits = 0;
  int ackNum;
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
    sock.recvFrom((char*)ackNum, sizeof(ackNum));
    cerr << "acked message " << ackNum << endl;
  }
  return retransmits;
}

void serverReliable(UdpSocket &sock, const int max, int message[]) {
  cerr << "server: reliable test:" << endl;
  int ackNum;

  // receive message[] max times
  for (int i = 0; i < max; i++) {
    sock.recvFrom((char*)message, MSGSIZE);   // udp message receive
    ackNum = message[0];
    sock.ackTo((char*)ackNum, sizeof(ackNum)); // udp message send
    cerr << message[0] << endl;                     // print out message
  }
}


int clientSlidingWindow(UdpSocket &sock, const int max, int message[], int windowSize) {
  cerr << "client: sliding window test:" << endl;
  int retransmits = 0;
  int lastAck = 0;
  int messages = 0;
  int ackNum;
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
    sock.recvFrom((char*)ackNum, sizeof(ackNum));
    cerr << "next message: " << ackNum << endl;
    lastAck = ackNum;
  }
  return retransmits;
}

void serverEarlyRetrans(UdpSocket &sock, const int max, int message[], int windowSize) {
  cerr << "server: early retrans test:" << endl;
  bool msgsrecvd[max];
  fill_n(msgsrecvd, max, false); //set all values in msgsrecvd to false
  int ackNum;
  int i = 0;

  // receive message[] max times
  while (i < max) {
    sock.recvFrom((char*)message, MSGSIZE);   // udp message receive
    msgsrecvd[message[0]]=true;
    while(msgsrecvd[i])
    {
      i++;  //move to next unreceived packet
    }
    ackNum = i;
    sock.ackTo((char*)ackNum, sizeof(ackNum)); // ack with next unreceived packet
    cerr << message[0] << endl;                     // print out message
  }
}