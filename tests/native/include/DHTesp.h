#pragma once
struct TempAndHumidity{float temperature;float humidity;};
class DHTesp{public:enum{DHT22};void setup(int,int){};TempAndHumidity getTempAndHumidity(){return{25,55};}};
