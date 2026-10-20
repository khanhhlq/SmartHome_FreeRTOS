#pragma once
#define WL_CONNECTED 3
class WiFiMock {public:void begin(const char*,const char*,int){} int status(){return WL_CONNECTED;}};
extern WiFiMock WiFi;
