#pragma once
#include <Arduino.h>
#define HTTP_GET 0
#define HTTP_POST 1
class WebServer {
public:WebServer(int){};
void send_P(int,const char*,const char*){}
void send(int,const char*,const char*){}
void send(int,const char*,const String&){}
void sendHeader(const char*,const char*){}
bool hasArg(const char*){return true;}
String arg(const char*){return String("25");}
void on(const char*,int,void(*)()){}
void begin(){} void handleClient(){}
};
