#pragma once
#include <math.h>
#include <stdint.h>

// Mô hình mặc định của wokwi-photoresistor-sensor, VCC=3V3 theo diagram.json.
// Trên ESP32-S3 thật, ADC có suy hao, sai số và cần hiệu chuẩn: không xem
// V=raw*3.3/4095 là phép đo điện áp trực tiếp trên phần cứng.
constexpr float LDR_MODEL_VCC = 3.3f;
constexpr float LDR_MODEL_FIXED_R = 10000.0f; // 10 kohm
constexpr float LDR_MODEL_R_AT_10_LUX = 50000.0f; // rl10=50 kohm
constexpr float LDR_MODEL_GAMMA = 0.7f;
constexpr int LDR_MAX_ADC = 4095;

struct LdrDiagnostics {
    int adcRaw;
    float voltageV;       // Giá trị SUY RA từ raw, giả sử 3.3V toàn thang
    float resistanceKOhm; // Quang trở tính theo mạch chia áp
    float lux;            // Độ rọi ƯỚC TÍNH theo mô hình Wokwi
    float brightnessPct;  // 0=tối, 100=sáng; chỉ số quy ước FSM
};

inline LdrDiagnostics ldrFromAdc(int raw) {
    if (raw < 0) raw = 0;
    if (raw > LDR_MAX_ADC) raw = LDR_MAX_ADC;
    LdrDiagnostics out = {};
    out.adcRaw = raw;
    out.voltageV = (float)raw * (LDR_MODEL_VCC / LDR_MAX_ADC);
    out.brightnessPct = 100.0f * (1.0f - (float)raw / LDR_MAX_ADC);
    if (raw >= LDR_MAX_ADC) {
        out.resistanceKOhm = 1000000.0f; // cắt trên để JSON hữu hạn
        out.lux = 0.0f;
    } else if (raw <= 0) {
        out.resistanceKOhm = 0.0f;
        out.lux = 100000.0f; // Cắt trên để JSON và log luôn hữu hạn
    } else {
        const float r = LDR_MODEL_FIXED_R * out.voltageV /
                        (LDR_MODEL_VCC - out.voltageV);
        out.resistanceKOhm = r / 1000.0f;
        float lux = 10.0f * powf(LDR_MODEL_R_AT_10_LUX / r,
                                  1.0f / LDR_MODEL_GAMMA);
        if (!isfinite(lux) || lux > 100000.0f) lux = 100000.0f;
        out.lux = lux;
    }
    return out;
}

// Ham phu dung TRONG KIEM THU: tu do roi mo phong suy ra ma ADC ly tuong
// de doi chieu voi Wokwi. Luong ung dung REAL KHONG su dung ham nay.
inline LdrDiagnostics ldrFromLux(float lux) {
    if (!isfinite(lux) || lux < 0) lux = 0;
    if (lux > 100000.0f) lux = 100000.0f;
    if (lux == 0) return ldrFromAdc(LDR_MAX_ADC);
    const float r = LDR_MODEL_R_AT_10_LUX *
                    powf(10.0f / lux, LDR_MODEL_GAMMA);
    const float v = LDR_MODEL_VCC * r / (LDR_MODEL_FIXED_R + r);
    int raw = (int)lroundf(v * LDR_MAX_ADC / LDR_MODEL_VCC);
    return ldrFromAdc(raw);
}
