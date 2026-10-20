#pragma once
#include <cstdint>
#include <cmath>
#include <cstring>
#include <string>
#include <algorithm>
using std::isfinite;
using std::isnan;
typedef uint32_t TickType_t;
typedef int BaseType_t;
typedef void* QueueHandle_t;
typedef void* SemaphoreHandle_t;
#define IRAM_ATTR
#define PROGMEM
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define CHANGE 2
#define pdTRUE 1
#define pdFALSE 0
#define pdPASS 1
#define portMAX_DELAY (0xFFFFFFFFU)
#define portTICK_PERIOD_MS 1
#define pdMS_TO_TICKS(x) (x)
#define portYIELD_FROM_ISR() ((void)0)
inline void vTaskDelay(unsigned){}
inline TickType_t xTaskGetTickCount(){return 1;}
inline TickType_t xTaskGetTickCountFromISR(){return 1;}
inline void pinMode(int,int){}
inline int analogRead(int){return 0;}
inline void analogReadResolution(int){}
inline int digitalRead(int){return 1;}
inline void digitalWrite(int,int){}
inline double ledcSetup(int,int,int){return 5000;}
inline void ledcAttachPin(int,int){}
inline void ledcWrite(int,int){}
inline void tone(int,int){}
inline void noTone(int){}
inline void attachInterrupt(int,void(*)(),int){}
inline int digitalPinToInterrupt(int i){return i;}
inline BaseType_t xTaskCreate(void (*)(void*),const char*,int,void*,int,void*){return pdPASS;}
inline void* xQueueCreate(int,int){return (void*)1;}
inline int xQueuePeek(void*,void*,int){return 1;}
inline int xQueueOverwrite(void*,const void*){return 1;}
inline void* xSemaphoreCreateBinary(){return (void*)1;}
inline void* xSemaphoreCreateMutex(){return (void*)1;}
inline int xSemaphoreTake(void*,int){return 1;}
inline void xSemaphoreGive(void*){}
inline void xSemaphoreGiveFromISR(void*,int*){}
inline void delay(int){}
inline unsigned long micros(){return 1;}
inline int constrain(int x,int a,int b){return std::max(a,std::min(x,b));}
class String: public std::string {
public:
    using std::string::string;
    String():std::string(){}
    String(const std::string &s):std::string(s){}
    String(float f,int p=2):std::string(std::to_string(f)){}
    String(int i):std::string(std::to_string(i)){}
    String(unsigned int i):std::string(std::to_string(i)){}
    const char* c_str() const {return std::string::c_str();}
};
inline String operator+(const char*a, const String&b){return String(std::string(a)+std::string(b));}
struct SerialMock{
 void begin(int){} void println(){} template<class T>void println(T){}
 template<class T>void print(T){} template<class...A>void printf(const char*,A...){}
};
extern SerialMock Serial;
