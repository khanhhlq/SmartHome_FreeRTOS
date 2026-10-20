# SmartHome_FreeRTOS – REAL Monitoring & Config v3

**Mục đích:** ESP32-S3 đọc cảm biến REAL trong Wokwi, xử lý theo 5 chế độ có ưu tiên; Web Dashboard **chỉ giám sát** và **thay đổi hai ngưỡng điều khiển**. Không còn mô phỏng sensor bằng thanh trượt Web.

## Phần cứng, tác vụ và đồng bộ
- DHT22 (GPIO15): `temperature` °C và `humidity` %RH từ thư viện DHTesp; dữ liệu khung 40 bit được thư viện xử lý.
- LDR (AO → GPIO4, ADC1_CH3): `ldrAdcRaw` thực từ `analogRead`; điện áp và lux suy ra từ mô hình Wokwi, phần trăm sáng `100*(1-raw/4095)`. **Không khẳng định điện áp hay lux là kết quả đo trực tiếp.**
- PIR (GPIO16): ngắt `CHANGE` → ISR give Binary Semaphore → PIRTask cập nhật trạng thái vào `sensorQueue`. PIR chỉ phát hiện chuyển động, không phải cảm biến xác nhận hiện diện tuyệt đối.
- Relay đèn GPIO6; LED mô phỏng PWM quạt GPIO17; buzzer GPIO7; OLED SSD1306 I2C (SDA GPIO8, SCL GPIO9).
- Giữ **6 Task**: SensorTask, PIRTask, DecisionTask, ControlTask, DisplayTask, WebTask với ưu tiên cũ 2,4,2,3,1,1.
- `sensorQueue` và `decisionQueue`: mỗi Queue 1 phần tử, `xQueueOverwrite` / `xQueuePeek`.
- Binary Semaphore phục vụ ngắt PIR. **`configMutex` mới bảo vệ toàn bộ bộ đôi ngưỡng COOLING/LIGHTING**, thay `webMutex` vì không còn `webData`.

## Ngưỡng
- `WARNING`: **T >= 45 °C**, mặc định và cố định, ưu tiên cao nhất.
- Nếu PIR = 0: `ENERGY_SAVING` (trừ khi WARNING).
- Nếu PIR = 1 và T >= `coolingTempC`: `COOLING` (mặc định 30, có thể chỉnh 25–40 °C).
- Nếu PIR = 1, không vào COOLING và sáng < `darkLightPct`: `LIGHTING` (mặc định 30, chỉnh 10–60%).
- Còn lại: `COMFORT`.
- Ở COOLING, LED đèn có thể bật thêm nếu mức sáng thấp hơn `darkLightPct`. Tốc độ quạt tối đa khi T >= 35 °C là quy tắc cố định, không phải ngưỡng bật COOLING.
- **Config nằm trong RAM**, reset mô phỏng sẽ về 30 °C, 30%. Không dùng NVS trong bản này.

## API HTTP (port 80)

| HTTP | Đường dẫn | Mục đích |
|---|---|---|
| GET | `/` | Trang HTML, CSS, JS của ESP32-S3 |
| GET | `/api/state` | JSON cảm biến REAL, output, FSM, metrics, revision cấu hình |
| GET | `/api/config` | JSON hai ngưỡng đang áp dụng và WARNING bất biến |
| POST | `/api/config` | Form URL encoded: `coolingTempC`, `darkLightPct` |

`POST /api/sensors` và `POST /api/real` **đã xóa**. Không có API ghi trực tiếp relay/quạt/buzzer. Giá trị POST sai dải bị từ chối (HTTP 400), cấu hình bận có thể HTTP 503; `warningTempC` gửi lên cũng bị từ chối.

## Chạy trên máy sinh viên
1. Giải nén thư mục này và mở thư mục bằng VS Code/PlatformIO. Cần extension PlatformIO + Wokwi (như môi trường nhóm hiện có).
2. PlatformIO **Build** theo `platformio.ini` (đang pin `espressif32 @ 6.12.0`, Arduino-ESP32 2.x). Không dùng Arduino-ESP32 3.x cho API LEDC kiểu kênh hiện tại.
3. Khởi động Wokwi với `diagram.json`, `wokwi.toml`. Trong Wokwi, điều chỉnh DHT22 và LDR; không chỉnh trên Web nữa.
4. Mở `http://localhost:8180` theo chuyển tiếp mạng trong `wokwi.toml`.
5. Dashboard hiển thị REAL và cho lưu 2 ngưỡng. Chú ý đo chân GPIO và trạng thái thiết bị theo Serial/OLED/Wokwi.

## Kiểm tra cục bộ (không thay thế build firmware Wokwi)

```bash
bash tests/native/run_host_checks.sh
```

Test dùng các header C++ giả lập để bắt lỗi cú pháp, FSM/ưu tiên/ngưỡng, LDR và đối chiếu sơ đồ nối chân. **Test không chạy firmware trên ESP32-S3.**

Xem thêm: `HUONG_DAN_V3_VA_CAP_NHAT_BAO_CAO.md`.
