/**
 * @file Os.h
 * @brief Services OS Header - Định nghĩa các API đồng bộ, hàng đợi log và nguyên mẫu Task hệ điều hành.
 */

#ifndef OS_H
#define OS_H

#include <stdint.h>
#include <stdbool.h>
#include "Mcal_Can.h"

#define OS_LOG_QUEUE_LEN    16U     /**< Chiều dài tối đa của hàng đợi log */
#define OS_LOG_MAX_STR      96U     /**< Ký tự tối đa trong một thông điệp log */

/** @brief Cấu trúc thông điệp log hệ thống */
typedef struct {
    char        buf[OS_LOG_MAX_STR];    /**< Chuỗi ký tự nội dung log */
    uint16_t    len;                    /**< Độ dài thực tế của chuỗi */
} Os_LogMsgType;

/**
 * @brief Khởi tạo hệ thống điều hành và tạo các task thời gian thực.
 */
void Os_Init(void);

/**
 * @brief Khóa Mutex bảo vệ RTE.
 */
void Os_Rte_Lock(void);

/**
 * @brief Mở khóa Mutex RTE.
 */
void Os_Rte_Unlock(void);

/**
 * @brief Đẩy khung CAN từ ngắt vào Queue.
 */
void Os_CanRx_PushFromISR(const Mcal_Can_FrameType *pFrame);

/**
 * @brief Báo hiệu từ ngắt MCAL UART khi DMA đã truyền xong chuỗi log.
 */
void Os_Uart_TxCpltFromISR(void);

/**
 * @brief Đẩy chuỗi log thô ra hàng đợi DMA an toàn đa nhiệm.
 */
void Os_Uart_PushRawString(const char *str, uint16_t len);

/**
 * @brief Hàm in log tổng quát hỗ trợ định dạng chuỗi (printf-like).
 */
void Os_Log_Print(const char *tag, const char *fmt, ...);

// Macro tiện ích in log theo các mức độ khác nhau
#define OS_LOG_I(fmt, ...)  Os_Log_Print("I", fmt, ##__VA_ARGS__)
#define OS_LOG_W(fmt, ...)  Os_Log_Print("W", fmt, ##__VA_ARGS__)
#define OS_LOG_E(fmt, ...)  Os_Log_Print("E", fmt, ##__VA_ARGS__)

/* ==================================================================== */
/* OSAL - THỜI GIAN VÀ ĐỒNG BỘ HÓA NGOẠI VI I2C                         */
/* ==================================================================== */

/**
 * @brief  Đưa Task gọi hàm vào trạng thái Blocked (ngủ) trong khoảng thời gian xác định.
 * @note   Nhường CPU cho các Task khác thực thi; không sử dụng trong ngữ cảnh ngắt (ISR).
 * @param  ms: Thời gian tạm dừng tính bằng mili-giây.
 */
void Os_DelayMs(uint32_t ms);

/**
 * @brief  Chặn Task hiện tại chờ phần cứng I2C hoàn tất truyền nhận qua cơ chế Semaphore.
 * @note   Task tự động nhường CPU và chuyển sang trạng thái Blocked cho đến khi
 *         hàm ngắt (ISR) gọi Os_I2c_TransferDoneFromISR() hoặc hết thời gian Timeout.
 * @param  TimeoutMs: Thời gian chờ tối đa (ms).
 * @retval true nếu nhận được tín hiệu hoàn tất từ phần cứng, false nếu quá hạn timeout.
 */
bool Os_I2c_WaitForTransfer(uint32_t TimeoutMs);
 
/**
 * @brief  Làm rỗng cờ Semaphore I2C trước khi kích hoạt một phiên truyền/nhận mới.
 * @note   Ngăn ngừa hiện tượng unblock sai do các cờ ngắt còn sót lại từ phiên trước.
 */
void Os_I2c_ClearSemaphore(void);

/**
 * @brief  Báo hiệu hoàn tất giao dịch I2C từ ngữ cảnh ngắt phần cứng (ISR).
 * @note   Giải phóng Semaphore và kích hoạt bộ lập lịch (Scheduler) chuyển ngữ cảnh
 *         ngay lập tức nếu Task đang chờ có mức ưu tiên cao hơn Task bị ngắt.
 */
void Os_I2c_TransferDoneFromISR(void);
 
#endif /* OS_H */
