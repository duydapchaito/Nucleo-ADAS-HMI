/**
 * @file    Cdd_Eeprom.c
 * @brief   Triển khai chi tiết logic điều khiển EEPROM AT24C02.
 * @details Thực thi cơ chế chia trang bộ nhớ vật lý, quản lý chu kỳ ghi nội bộ (tWR)
 *          kết hợp với tầng MCAL I2C và cơ chế điều phối thời gian thực của OS.
 * @version 1.0
 */

#include "Cdd_Eeprom.h"
#include "Mcal_I2c.h"
#include "Os.h"

/* ==================================================================== */
/* ĐỊNH NGHĨA THỜI GIAN VÀ CẤU HÌNH GIAO THỨC NỘI BỘ                     */
/* ==================================================================== */

#define EEPROM_MEM_ADDR_SIZE        1U      /**< Kích thước trường địa chỉ nội bộ của AT24C02 (1 Byte) */
#define EEPROM_I2C_TIMEOUT_MS       20U     /**< Thời gian timeout tối đa cho một giao dịch bus I2C (ms) */
#define EEPROM_MAX_STANDBY_RETRIES  10U     /**< Số lần kiểm tra tối đa trạng thái bận nung Flash của chip */

/**
 * @brief  Thăm dò và chờ chip EEPROM hoàn thành chu kỳ ghi vật lý nội bộ (Internal Write Cycle).
 * @details Sau khi nhận đủ byte trên bus, chip AT24C02 ngắt liên lạc và mất tối đa 5ms để ghi
 *          vào ô nhớ flash. Hàm này ping thử địa chỉ bus (Acknowledge Polling): nếu chip phản hồi
 *          NACK thì nhường quyền điều phối CPU cho Task khác trong 1ms qua Os_DelayMs rồi thử lại.
 * @retval CDD_EEPROM_OK nếu chip đã ghi xong và sẵn sàng; CDD_EEPROM_ERR_I2C nếu quá số lần thử.
 */
static Cdd_Eeprom_StatusType Cdd_Eeprom_WaitStandby(void)
{
    for (uint8_t retry = 0U; retry < EEPROM_MAX_STANDBY_RETRIES; retry++)
    {
        /* Ping 1 lần với thời gian chờ ngắn: Nếu trả về OK tức chip đã ghi xong và kéo ACK */
        if (Mcal_I2c_IsDeviceReady(CDD_EEPROM_I2C_ADDR, 1U, 2U) == MCAL_I2C_OK) {
            return CDD_EEPROM_OK;
        }
        /* Chip vẫn đang nung ô nhớ: Đưa Task vào trạng thái Blocked 1ms để giải phóng CPU */
        Os_DelayMs(1U);
    }
    return CDD_EEPROM_ERR_I2C;
}

/**
 * @brief Khởi tạo và kiểm tra sự hiện diện của chip EEPROM AT24C02 trên đường bus I2C.
 */
Cdd_Eeprom_StatusType Cdd_Eeprom_Init(void)
{
    /* Kiểm tra ping thử 3 lần tới địa chỉ phần cứng để xác thực đường truyền vật lý */
    if (Mcal_I2c_IsDeviceReady(CDD_EEPROM_I2C_ADDR, 3U, 10U) == MCAL_I2C_OK) {
        OS_LOG_I("EEPROM: AT24C02 Ready at Addr 0x%02X", CDD_EEPROM_I2C_ADDR);
        return CDD_EEPROM_OK;
    }
    OS_LOG_E("EEPROM: Hardware Not Responding!");
    return CDD_EEPROM_ERR_I2C;
}

/**
 * @brief Ghi mảng dữ liệu với thuật toán tự động băm nhỏ theo ranh giới trang 8 byte.
 */
Cdd_Eeprom_StatusType Cdd_Eeprom_WriteBuffer(uint8_t MemAddr, const uint8_t *pData, uint16_t Len)
{
    /* Kiểm tra tính hợp lệ của tham số: Chống tràn dung lượng bộ nhớ 256 Byte */
    if ((pData == NULL) || (Len == 0) || (((uint16_t)MemAddr + Len) > CDD_EEPROM_TOTAL_SIZE)) {
        return CDD_EEPROM_ERR_PARAM;
    }

    uint8_t curr_addr = MemAddr;
    const uint8_t *p_curr_data = pData;
    uint16_t bytes_left = Len;
    
    /* Vòng lặp chia nhỏ dữ liệu thành từng khối vừa vặn với kích thước trang vật lý */
    while (bytes_left > 0U) 
    {
        /* Tính toán độ lệch ô nhớ hiện tại so với đầu trang vật lý gần nhất */
        uint8_t page_offset = curr_addr % CDD_EEPROM_PAGE_SIZE;
        uint8_t space_in_page = CDD_EEPROM_PAGE_SIZE - page_offset;
        
        /* Giới hạn kích thước gói truyền chỉ bằng khoảng trống còn lại của trang hiện tại */
        uint16_t chunk_size = (bytes_left < space_in_page) ? bytes_left : space_in_page;

        /* Đẩy dữ liệu trang xuống tầng MCAL I2C qua cơ chế ngắt không chặn CPU */
        if (Mcal_I2c_WriteMem(CDD_EEPROM_I2C_ADDR, 
                            (uint16_t)curr_addr,
                            EEPROM_MEM_ADDR_SIZE, 
                            (uint8_t *)p_curr_data, 
                            chunk_size, 
                            EEPROM_I2C_TIMEOUT_MS) != MCAL_I2C_OK) 
        {
            return CDD_EEPROM_ERR_I2C;
        }

        /* Chờ chip hoàn tất ghi trang nội bộ trước khi thực hiện giao dịch tiếp theo */
        if (Cdd_Eeprom_WaitStandby() != CDD_EEPROM_OK) {
            return CDD_EEPROM_ERR_I2C;
        }

        /* Cập nhật con trỏ dữ liệu và địa chỉ sang phân đoạn tiếp theo */
        curr_addr += (uint8_t)chunk_size;
        p_curr_data += chunk_size;
        bytes_left -= chunk_size;
    }

    return CDD_EEPROM_OK;
}

/**
 * @brief Đọc tuần tự dữ liệu từ bộ nhớ nội bộ EEPROM.
 */
Cdd_Eeprom_StatusType Cdd_Eeprom_ReadBuffer(uint8_t MemAddr, uint8_t *pData, uint16_t Len)
{
    /* Kiểm tra tính hợp lệ của bộ đệm và dải địa chỉ đọc */
    if ((pData == NULL) || (Len == 0U) || (((uint16_t)MemAddr + Len) > CDD_EEPROM_TOTAL_SIZE)) {
        return CDD_EEPROM_ERR_PARAM;
    }

    /* Đọc dữ liệu tuần tự trực tiếp qua MCAL mà không cần băm trang */
    if (Mcal_I2c_ReadMem(CDD_EEPROM_I2C_ADDR, 
                        (uint16_t)MemAddr, 
                        EEPROM_MEM_ADDR_SIZE, 
                        pData, 
                        Len, 
                        EEPROM_I2C_TIMEOUT_MS) != MCAL_I2C_OK) 
    {
        return CDD_EEPROM_ERR_I2C;
    }

    return CDD_EEPROM_OK;
}
