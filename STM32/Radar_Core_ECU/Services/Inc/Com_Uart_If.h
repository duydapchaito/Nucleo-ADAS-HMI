#ifndef COM_UART_IF_H
#define COM_UART_IF_H

#include <stdint.h>

// Định nghĩa mã trạng thái để biết truyền thành công hay thất bại
typedef enum {
    COM_TX_OK = 0,
    COM_TX_BUSY,
    COM_TX_ERROR
} Com_StatusType;

// Function Prototypes
// Hàm này nhận vào các giá trị đã được AppL xử lý xong
Com_StatusType Com_SendRadarNmeaMessage(char GearState, 
                                        uint16_t DistFL, uint16_t DistFC, uint16_t DistFR, 
                                        uint16_t DistRL, uint16_t DistRC, uint16_t DistRR);

#endif /* COM_UART_IF_H */
