#include "Eal_Encoder.h"
#include "Mcal_Timer.h"

static uint32_t Last_Count = 0;
static int32_t Accumulated_Delta = 0;

void Eal_Encoder_Init(void)
{
    // Đọc giá trị khởi tạo để tránh giật số lần đầu
    Last_Count = Mcal_Timer_GetEncoderCount();
    Accumulated_Delta = 0;
}

int32_t Eal_Encoder_GetDelta(void)
{
    uint32_t Current_Count = Mcal_Timer_GetEncoderCount();

    // Ép kiểu để tính toán Delta chuẩn xác kể cả khi tràn qua 0 hoặc 0xFFFFFFFF
    int32_t diff = (int32_t)(Current_Count - Last_Count);

    // Cập nhật lại Last_Count cho lần sau
    Last_Count = Current_Count;

    // Tích luỹ xung
    Accumulated_Delta += diff;

    // Khi nào đủ 4 xung mới chuyển 1 nấc
    if (Accumulated_Delta >= 4) {
        Accumulated_Delta -= 4; // Giữ lại phần dư nếu vặn quá nhanh
        return 1;
    } else if (Accumulated_Delta <= -4) {
        Accumulated_Delta += 4;
        return -1;
    }

    return 0;
}
