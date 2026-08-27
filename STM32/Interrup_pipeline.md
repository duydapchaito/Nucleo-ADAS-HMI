```
[ IDE Gen Code ]           [ TẦNG ECUAL ]                     [ TẦNG MCAL ]
    main.c                  EcuAb_Radar.c                     Mcal_Timer.c
===============           =================                  ===============
      |                           |                                 |
      | 1. Gọi hàm khởi tạo       |                                 |
[ main() ] ----------------> [ EcuAb_Radar_Init() ]                 |
      |                           |                                 |
      |                           | 2. Chuẩn bị "Phích cắm"         |
      |                           | (Địa chỉ hàm nội bộ:            |
      |                           |  EcuAb_Radar_IC_Handler)        |
      |                           |                                 |
      |                           | 3. Gọi API đăng ký của MCAL     |
      |                           |------------------------> [ Mcal_Timer_Register_IC_Callback() ]
      |                           |                                 | 4. MCAL lưu địa chỉ hàm
      |                           |                                 | vào con trỏ nội bộ
      |                           |                                 | (App_IC_Callback = ptr)
```


```
[ PHẦN CỨNG ]      [ IDE Gen Code ]             [ TẦNG MCAL ]             [ TẦNG ECUAL ]
 Hardware IT        stm32f4xx_it.c              Mcal_Timer.c               EcuAb_Radar.c
=============      ================             =============             ================
      |                   |                           |                          |
      | 1. Sóng Echo dội về, kích ngắt NVIC           |                          |
[ TIM3_CH1 ] -----------> |                           |                          |
      |                   | 2. HAL gọi hàm Callback   |                          |
      |          [ HAL_TIM_IC_CaptureCallback() ]     |                          |
      |                   |                           |                          |
      |                   | 3. Đá quả bóng xuống MCAL |                          |
      |                   |-----------------> [ Mcal_Timer_Catch_IT() ]          |
      |                   |                           |                          |
      |                   |                           | 4. Giải mã thanh ghi CCR |
      |                   |                           | (Lấy Timestamp gốc)      |
      |                   |                           |                          |
      |                   |                           | 5. Truyền điện lên ống!  |
      |                   |                           | (Gọi Function Pointer)   |
      |                   |                           |-----------------> [ EcuAb_Radar_IC_Handler() ]
      |                   |                           |                          | 
      |                   |                           |                          | 6. Đích đến cuối cùng!
      |                   |                           |                          | - Tính Delta thời gian
      |                   |                           |                          | - Gọi lọc Kalman (CDD)
      |                   |                           |                          | - Cất vào Kho (RTE)
```