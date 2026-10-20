#pragma once
#define SSD1306_SWITCHCAPVCC 1
#define SSD1306_WHITE 1
class WireMock;
class Adafruit_SSD1306{
public:Adafruit_SSD1306(int,int,WireMock*,int){};
bool begin(int,int){return true;} void clearDisplay(){};
void setTextColor(int){};void setTextSize(int){};void setCursor(int,int){};
template<class T>void print(T){};
template<class T>void print(T,int){};
template<class T>void println(T){};
template<class T>void println(T,int){};
void display(){};
};
