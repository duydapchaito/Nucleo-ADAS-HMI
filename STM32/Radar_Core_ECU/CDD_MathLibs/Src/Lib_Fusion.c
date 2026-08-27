#include "Lib_Fusion.h"

uint16_t Math_Fusion_VirtualCenter(uint16_t dist_left, uint16_t dist_right)
{
    /*
    * THUẬT TOÁN ĐÁNH GIÁ (Heuristic Fusion)
    * Nếu có 1 vật thể xuất hiện, cảm biến siêu âm vốn có góc mở rộng (dạng nón),
    * nên vật nằm ở giữa chắc chắn sẽ bị quét trúng bởi 2 rìa sóng của cảm biến Trái và Phải.
    * Do bị quét bởi rìa, cạnh huyền luôn dài hơn cạnh góc vuông -> Thực tế vật thế gần hơn số đo được.
    */
    
    uint16_t min_dist = (dist_left < dist_right) ? dist_left : dist_right;

    // Nếu cả 2 cảm biến không thấy gì cả (ví dụ >= 200cm), Center coi như clear
    if (min_dist >= 200) {
        return min_dist;
    }

    // Nếu có vật trong tầm, dùng hệ số lượng giác suy giảm (Cos góc lệch, xấp xỉ 0.85)
    float virtual_dist = (float)min_dist * 0.85;

    return (uint16_t)virtual_dist;
}
