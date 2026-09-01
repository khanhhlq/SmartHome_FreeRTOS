// Cấu hình những kiểu dữ liệu riêng của dự án

#pragma once
#include <Arduino.h>

struct SensorData {
    float temperature;
    float humidity;
    float light;
    bool occupancy; // Có phát hiện chuyện động true/false
    TickType_t sampleTick; // Dùng để lưu tick ví dụ sensortask tạo dữ liệu thì nó ghi lại timing tạo (Phục vụ đo timimg)
};

// enum = enumeration là liệt kê
enum SystemMode {
    COMFORT, // 0
    COOLING, // 1
    LIGHTING, // 2 
    ENERGY_SAVING, // 3
    WARNING // 4
};
// Thay vì viết int mode = 0 or 1 or 2 or 3 or 4 thì dùng enum để khởi tạo SystemMode mode ---> mode = COOLING <--> mode = 1 (khó nhớ)

// Chọn mode và đưa vào decisionQueue và controlTask sẽ đọc nó
struct DecisionData {
    SystemMode mode;
};

// Data lấy nguồn từ đâu
enum InputSource {
    REAL_SENSOR,
    WEB_SENSOR
};