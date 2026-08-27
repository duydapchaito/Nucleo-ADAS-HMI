```
[GIAO DIỆN UI - Desktop Qt C++]
            ^ (NMEA Data)
            |
================================================================================
                           TẦNG ỨNG DỤNG (AppL)
                 (Chủ thể: Vòng lặp main() - Synchronous)
                 
    [Swc_HmiRouter]  <------------------------->  [Swc_AdasWarning]
           |                                             | (Ra quyết định)
===================|=====================================|======================
           | (Kéo Data)          R T E                   | (Gọi lệnh bật còi)
           V                                             V
    [ Rte_Radar_Data ]                            [ Rte_Call_Buzzer() ]
           ^ (Đẩy Data)                                  | 
===========|=============================================|======================
           |                                             |
[CDD_MathLibs (Kalman/Fusion)]                           |
           ^                                             V
           |                                     [ECUAL_Buzzer]
      [ECUAL_Radar]                                      |
           ^ (Tính toán thô)                             V
===========|=============================================|======================
           |                                             |
     [MCAL_Timer_IC]                               [MCAL_GPIO]
           ^ (Ngắt NVIC - Asynchronous)                  | (Xóa/Lập thanh ghi)
===========|=============================================|======================
      [PHẦN CỨNG (Phát/Thu Sóng, Còi, Nút Xoay, Mạch UART)]
```