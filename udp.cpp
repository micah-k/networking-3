#include <iostream>
#include <algorithm>    //fill_n()
#include "UdpSocket.h"
#include "Timer.h"

using namespace std;

int clientStopWait(UdpSocket &sock, const int max, int message[]) {
  cerr << "client: stop-wait test:" << endl;
  int retransmits = 0;
  int ackNum[1];
  Timer timeout;

  // transfer message[] max times
  for (int i = 0; i < max; i++) {
    message[0] = i;                                     // message[0] has a sequence #
    sock.sendTo((char*)message, MSGSIZE);               // udp message send
    cerr << "message " << message[0] << endl;

    if(sock.pollRecvFrom()==0)
    {
      timeout.start();
      while(sock.pollRecvFrom()==0)
      {
        if(timeout.lap()>1500)
        {
          retransmits++;
          sock.sendTo( ( char * )message, MSGSIZE );    // udp message send
          cerr << "resending message " << message[0] << endl;
          timeout.start();
        }
      }
    }
    sock.recvFrom((char*)ackNum, 4);
    cerr << "acked message " << ackNum[0] << endl;
  }
  return retransmits;
}

void serverReliable(UdpSocket &sock, const int max, int message[]) {
  cerr << "server: reliable test:" << endl;
  int ackNum[1];
  bool recvd;

  // receive message[] max times
  for (int i = 0; i < max; i++) {
    recvd = false;
    while (!recvd)
    {
      cerr << "Expecting " << i << endl;
      sock.recvFrom((char*)message, MSGSIZE);           // udp message receive
      cerr << "Received " << message[0] << endl;        // print out message
      if(message[0] <= i)
      {
        ackNum[0] = message[0];
        sock.ackTo((char*)ackNum, 4);                   // udp message send
        if(message[0] == i)
          recvd = true;
      }
    }
  }
}


int clientSlidingWindow(UdpSocket &sock, const int max, int message[], int windowSize) {
  cerr << "client: sliding window test:" << endl;
  int retransmits = 0;
  int lastAck = 0;
  int messages = 0;
  int ackNum[1];
  Timer timeout;



  // transfer message[] max times
  for (int i = 0; i < max; i++) {
    if ((int)((char)i) < lastAck)
    {
      messages--;
      continue;
    }
    while(messages < windowSize && i + messages < max)
    {
      message[0] = i + messages;                        // message[0] has a sequence #
      sock.sendTo((char*)message, MSGSIZE);             // udp message send
      cerr << "message " << message[0] << endl;
      messages++;
    }

    if(sock.pollRecvFrom()==0)
    {
      timeout.start();
      while(sock.pollRecvFrom()==0)
      {
        if(timeout.lap()>2000)
        {
          retransmits++;
          message[0] = i;
          sock.sendTo( ( char * )message, MSGSIZE );    // udp message send
          cerr << "resending message " << message[0] << endl;
          timeout.start();
        }
      }
    }
    sock.recvFrom((char*)ackNum, 4);
    cerr << "next message: " << ackNum[0] << endl;
    lastAck = ackNum[0];
  }
  return retransmits;
}

void serverEarlyRetrans(UdpSocket &sock, const int max, int message[], int windowSize) {
  cerr << "server: early retrans test:" << endl;
  bool msgsrecvd[max];
  fill_n(msgsrecvd, max, false);                        //set all values in msgsrecvd to false
  int ackNum[1];
  int i = 0;

  // receive message[] max times
  while (i < max) {
    sock.recvFrom((char*)message, MSGSIZE);             // udp message receive
    msgsrecvd[message[0]]=true;
    while(msgsrecvd[i])
    {
      i++;  //move to next unreceived packet
    }
    ackNum[0] = i;
    sock.ackTo((char*)ackNum, 4);                       // ack with next unreceived packet
    cerr << message[0] << endl;                         // print out message
  }
}