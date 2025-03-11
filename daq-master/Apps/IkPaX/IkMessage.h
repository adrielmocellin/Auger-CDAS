#ifndef _IKMessage_
#define _IKMessage_

#include <cstring>
#include <iostream>

using namespace std;

class IkMessage {
public:
  string Sender;
  string Dest;
  string Type;
  string Content;
  IkMessage() {};
  IkMessage(const IkMessage &mess)
    {
      Sender = mess.Sender;
      Dest = mess.Dest;
      Type = mess.Type;
      Content = mess.Content;
    };
  IkMessage(string s,string d,string t,string c) {
    Sender=s;
    Dest=d;
    Type=t;
    Content=c;
  };
  string FullMessage() {
    return "|"+Sender+"|"+Dest+"|"+Type+"|"+Content+"|";
  }
  int FromBuffer(char *buf) {
    char *tmp=strchr(buf,'|');
    if (!tmp) {
      cerr << "Badly formated IkMessage" << endl;
      return 0;
    }
    *tmp='\0';
    char *tmp2=strchr(tmp+1,'|');
    if (!tmp2) {
      cerr << "Badly formated IkMessage" << endl;
      return 0;
    }
    *tmp2='\0';
    char *tmp3=strchr(tmp2+1,'|');
    if (!tmp3) {
      cerr << "Badly formated IkMessage" << endl;
      return 0;
    }
    *tmp3='\0';
    char *tmp4=strchr(tmp3+1,'|');
    if (!tmp4) {
      cerr << "Badly formated IkMessage" << endl;
      return 0;
    }
    *tmp4='\0';
    char *tmp5=strchr(tmp4+1,'|');
    if (!tmp5) {
      cerr << "Badly formated IkMessage" << endl;
      return 0;
    }
    *tmp5='\0';
    string s(tmp+1);
    Sender=s;
    string s1(tmp2+1);
    Dest=s1;
    string s2(tmp3+1);
    Type=s2;
    string s3(tmp4+1);
    Content=s3;
    *tmp='|';
    *tmp2='|';
    *tmp3='|';
    *tmp4='|';
    *tmp5='|';
    return 1;
  }
  // for compatibility
  int get_type();
  string get_sender() {return Sender;}
  string to_text() {return FullMessage();}
  int send(const char *dest);
};

#endif
