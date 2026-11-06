/**
 * @file Eal_Encoder.c
 * @brief EAL Encoder Driver - Xử lý tín hiệu bộ mã hóa xung xoay (Quadrature Encoder) để xác định vị trí cần số.
 */

#include "Eal_Encoder.h"
#include "Mcal_Timer.h"

static uint32_t Last_Count = 0U;
static int32_t Accumulated_Delta = 0;

/**
 * @brief Khởi tạo giá trị ban đầu cho bộ đếm Encoder.
 */
void Eal_Encoder_Init(void)
{
    // Đọc giá trị khởi tạo để tránh giật số lần đầu
    Last_Count = Mcal_Timer_GetEncoderCount();
    Accumulated_Delta = 0;
}

/**
 * @brief Tính toán độ dịch chuyển xung (Delta) của cần số sau mỗi chu kỳ.
 * @return int32_t 1: Tiến 1 nấc, -1: Lùi 1 nấc, 0: Không đổi.
 */
int32_t Eal_Encoder_GetDelta(void)
{
    uint32_t Current_Count = Mcal_Timer_GetEncoderCount();

    // Xử lý an toàn khi bộ đếm 32-bit bị tràn qua 0 hoặc 0xFFFFFFFF
    int32_t diff = (int32_t)(Current_Count - Last_Count);
    Last_Count = Current_Count;

    Accumulated_Delta += diff;

    // Quy chuẩn: Cần 4 xung cơ học mới dịch chuyển được 1 nấc số
    if (Accumulated_Delta >= 4) {
        Accumulated_Delta -= 4;
        return 1;   // Tăng số (Ví dụ: N lên D)
    } else if (Accumulated_Delta <= -4) {
        Accumulated_Delta += 4;
        return -1;  // Giảm số (Ví dụ: D xuống N)
    }

    return 0;   // Chưa đủ xung dịch chuyển
}
