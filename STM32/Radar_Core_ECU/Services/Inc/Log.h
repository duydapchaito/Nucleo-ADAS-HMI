/**
 * @file Log.h
 * @brief BSW Logger Singleton Service - Quản lý định dạng và truyền log bất đồng bộ qua UART DMA.
 */

#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdbool.h>

#define LOG_QUEUE_LEN       32U     /**< Chiều sâu hàng đợi thông điệp Log */
#define LOG_MAX_STR_LEN     128U    /**< Độ dài tối đa của một chuỗi Log (Bytes) */

// /** @brief Cấu trúc bản tin lưu trong hàng đợi của Singleton Logger */
// typedef struct {
//     char buf[LOG_MAX_STR_LEN];
//     uint16_t len;
// } Log_MessageType;

/**
 * @brief Khởi tạo đối tượng Singleton Logger, tạo Queue/Semaphore và đăng ký Callback với MCAL UART.
 */
void Log_Init(void);

/**
 * @brief Vòng lặp xử lý chính của Logger (Được gọi bởi Task Logger của OS).
 */
void Log_TaskLoop(void);

/**
 * @brief Đẩy một chuỗi dữ liệu thô (như bản tin NMEA) vào hàng đợi truyền UART DMA.
 */
void Log_PushRawString(const char *pStr, uint16_t len);

/**
 * @brief Định dạng chuỗi log kèm System Tick và mức độ (I/W/E) rồi đẩy vào hàng đợi không chặn.
 */
void Log_Print(const char *pTag, const char *pFmt, ...);

/**
 * @brief Xuất thông báo lỗi nghiêm trọng (Fatal Error) trực tiếp bằng chế độ Polling khi RTOS dừng.
 */
void Log_FatalPolling(const char *pPrefix, const char *pDetail);

/* Bộ Macro chuẩn cho toàn bộ các phân hệ BSW & CDD */
#define LOG_I(fmt, ...)     Log_Print("I", fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...)     Log_Print("W", fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...)     Log_Print("E", fmt, ##__VA_ARGS__)

#endif /* LOG_H */
