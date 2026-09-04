#include "Swc_AdasWarning.h"
#include "Rte_Radar.h"
#include "Lib_Filter.h"
#include "Lib_Fusion.h"
#include "Eal_Buzzer.h"
#include "Eal_Encoder.h"
#include "Eal_RadarHw.h"
#include "Com_Uart_If.h"

// =========================================================
// KHU VỰC CALIBRATION (Hiệu chỉnh thông số)
// (Trong thực tế, các biến này sẽ được đặt vào bộ nhớ Flash/EEPROM)
// =========================================================
#define CALIB_KALMAN_Q          0.01f   // Nhiễu hệ thống (Tăng -> phản ứng nhanh, ít mượt)
#define CALIB_KALMAN_R          5.0f    // Nhiễu cảm biến (Tăng -> mượt, nhưng trễ pha)
#define CALIB_KALMAN_P_INIT     1.0f    // Sai số dự đoán ban đầu
#define CALIB_KALMAN_X_INIT     400.0f  // Khởi tạo an toàn tại Max Range (cm)

#define ALERT_ZONE_DANGER       30      // Khoảng cách nguy hiểm (cm) -> Kêu liên tục
#define ALERT_ZONE_WARNING      50     // Khoảng cách cảnh báo (cm) -> Bíp nhanh
#define ALERT_ZONE_ATTENTION    100     // Khoảng cách chú ý (cm) -> Bíp chậm
// =========================================================

// Khai báo các bộ lọc tĩnh cho 4 cảm biến vật lý
static Math_Median_t MedianFilter[4];
static Math_Kalman1D_t KalmanFilter[4];

// --- HÀM NỘI BỘ: Xử lý nhịp còi ---
static void Swc_Adas_BuzzerLogic(uint16_t min_distance)
{
    static uint8_t tick_counter = 0;
    tick_counter++;

    if (min_distance <= ALERT_ZONE_DANGER) {
        Eal_Buzzer_TurnOn(); // Nguy hiểm -> Bật
    } else if (min_distance <= ALERT_ZONE_WARNING) {
        // Mỗi 4 ticks (25*4 = 100ms) đảo trạng thái còi 1 lần -> Bíp nhanh
        if (tick_counter % 4 == 0) 
            Eal_Buzzer_Toggle();
    } else if (min_distance <= ALERT_ZONE_ATTENTION) {
        // Mỗi 12 ticks (25*12 = 300ms) đảo trạng thái còi 1 lần -> Bíp chậm
        if (tick_counter % 12 == 0)
            Eal_Buzzer_Toggle();
    } else {
        Eal_Buzzer_TurnOff(); // An toàn tuyệt đối -> Tắt
    }
}

void Swc_Adas_Init(void)
{
    for (int i = 0; i < RTE_MAX_SENSORS; i++) {
        Math_Filter_MedianInit(&MedianFilter[i]);
        Math_Filter_KalmanInit(&KalmanFilter[i], 
                            CALIB_KALMAN_Q,
                            CALIB_KALMAN_R,
                            CALIB_KALMAN_P_INIT,
                            CALIB_KALMAN_X_INIT);
    }
    Eal_Encoder_Init();
}

void Swc_Adas_MainFunction_25ms(void)
{
    static uint8_t CurrentPhase = 0;

    // ====================================================
    // BƯỚC 0.1: ĐỌC ENCODER VÀ XỬ LÝ SANG SỐ (P <-> R <-> N <-> D)
    // ====================================================
    int32_t enc_delta = Eal_Encoder_GetDelta();
    Rte_GearType currentGear = Rte_Read_Gear(); // Đọc số hiện tại

    if (enc_delta >= 1) { // Vặn núm sang phải (Tiến số)
        if (currentGear == RTE_GEAR_P) 
            currentGear = RTE_GEAR_R;
        else if (currentGear == RTE_GEAR_R) 
            currentGear = RTE_GEAR_N;
        else 
            currentGear = RTE_GEAR_D;
    } else if (enc_delta <= -1) { // Vặn núm sang trái (Lùi số)
        if (currentGear == RTE_GEAR_D) 
            currentGear = RTE_GEAR_N;
        else if (currentGear == RTE_GEAR_N)
            currentGear = RTE_GEAR_R;
        else
            currentGear = RTE_GEAR_P;
    }
    Rte_Write_Gear(currentGear); // Lưu số mới vào RTE

    // ====================================================
    // BƯỚC 0.2: KÉO DỮ LIỆU TỪ EAL LÊN RTE
    // ====================================================
    for (int i = 0; i < RTE_MAX_SENSORS; i++) {
        // Rút data phần cứng (Ticks) nhét vào kho RTE
        uint32_t raw_hw_ticks = Eal_Radar_GetRawTicks(i);
        Rte_Write_RawTicks(i, raw_hw_ticks);
    }

    // ----------------------------------------------------
    // BƯỚC 1: XỬ LÝ DỮ LIỆU CỦA PHA TRƯỚC BẰNG TOÁN HỌC
    // ----------------------------------------------------
    uint16_t LimitMin = Rte_Prm_GetMinSensorRange();
    uint16_t LimitMax = Rte_Prm_GetMaxSensorRange();

    for (int i = 0; i < RTE_MAX_SENSORS; i++) {
        uint32_t raw_ticks = Rte_Read_RawTicks(i);
        uint16_t raw_cm = (uint16_t)(raw_ticks * 0.01715f); // Đổi tick ra cm tại đây

        // Pipeline: Outlier -> Median -> Kalman
        uint16_t step1 = Math_Filter_Outlier(raw_cm, LimitMin, LimitMax, Rte_Read_Distance(i));
        uint16_t step2 = Math_Filter_MedianProcess(&MedianFilter[i], step1);
        float step3 = Math_Filter_KalmanProcess(&KalmanFilter[i], step2);

        Rte_Write_Distance(i, (uint16_t)step3);
    }

    // ----------------------------------------------------
    // BƯỚC 2: TẠO CẢM BIẾN ẢO (Sensor Fusion)
    // ----------------------------------------------------
    uint16_t dist_FL = Rte_Read_Distance(RTE_SENSOR_FL);
    uint16_t dist_FR = Rte_Read_Distance(RTE_SENSOR_FR);
    uint16_t dist_RL = Rte_Read_Distance(RTE_SENSOR_RL);
    uint16_t dist_RR = Rte_Read_Distance(RTE_SENSOR_RR);

    uint16_t virtual_FC = Math_Fusion_VirtualCenter(dist_FL, dist_FR);
    uint16_t virtual_RC = Math_Fusion_VirtualCenter(dist_RL, dist_RR);

    // ----------------------------------------------------
    // BƯỚC 3: LOGIC CẢNH BÁO
    // ----------------------------------------------------
    uint16_t closest_obstacle = 400; // Mặc định an toàn

    if (currentGear == RTE_GEAR_R) {
        // Đang lùi -> Tìm vật cản gần nhất ở cụm Sau (RL, RC, RR)
        closest_obstacle = dist_RL;
        if (virtual_RC < closest_obstacle) 
            closest_obstacle = virtual_RC;
        if (dist_RR < closest_obstacle)
            closest_obstacle = dist_RR;
        Swc_Adas_BuzzerLogic(closest_obstacle);
    } else {
        Eal_Buzzer_TurnOff();
    }

    // ----------------------------------------------------
    // BƯỚC 4: XUẤT DỮ LIỆU LÊN HMI (GỌI SERVICES COM)
    // ----------------------------------------------------
    // Chuyển đổi Enum Hộp số sang ký tự cho chuẩn NMEA
    char gearChar = 'P';
    if (currentGear == RTE_GEAR_R) gearChar = 'R';
    else if (currentGear == RTE_GEAR_N) gearChar = 'N';
    else if (currentGear == RTE_GEAR_D) gearChar = 'D';

    // Đẩy toàn bộ 6 mắt (4 thật + 2 ảo) xuống tầng Services để gửi UART
    Com_SendRadarNmeaMessage(gearChar, 
                            dist_FL, virtual_FC, dist_FR, 
                            dist_RL, virtual_RC, dist_RR);

    // ----------------------------------------------------
    // BƯỚC 5: KÍCH HOẠT PHẦN CỨNG CHO CHU KỲ MỚI (PHÂN THỜI GIAN)
    // ----------------------------------------------------
    if (CurrentPhase == 0) {
        if (currentGear != RTE_GEAR_D) Eal_Radar_Trigger(RTE_SENSOR_RL);
        if (currentGear != RTE_GEAR_R) Eal_Radar_Trigger(RTE_SENSOR_FL);
        CurrentPhase = 1;
    } else {
        if (currentGear != RTE_GEAR_D) Eal_Radar_Trigger(RTE_SENSOR_RR);
        if (currentGear != RTE_GEAR_R) Eal_Radar_Trigger(RTE_SENSOR_FR);
        CurrentPhase = 0;
    }
}
