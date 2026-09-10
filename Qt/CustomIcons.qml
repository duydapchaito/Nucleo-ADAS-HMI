import QtQuick

// File chứa dữ liệu mã Vector SVG cho từng Icon
QtObject {
    id: icons

    // 1. Return / Back (Mũi tên quay lại)
    readonly property string back: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><path d="M9 14L4 9l5-5"/><path d="M4 9h10.5a5.5 5.5 0 0 1 5.5 5.5v0a5.5 5.5 0 0 1-5.5 5.5H11"/></svg>'

    // 2. 2D View
    readonly property string mode2D: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><text x="4" y="14" font-family="sans-serif" font-weight="bold" font-size="10" fill="white" stroke="none">2D</text><path d="M2 17c3 3 14 3 17 0"/><path d="M16 15l3 2-1 3"/></svg>'

    // 3. Front Sensor (Cảm biến trước)
    readonly property string sensorFront: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="2" stroke-linecap="round"><path d="M8 14h8v6H8z" rx="2"/><path d="M6 10a8 8 0 0 1 12 0"/><path d="M9 12a4 4 0 0 1 6 0"/></svg>'

    // 4. Hitch / Tow Bar (Móc kéo)
    readonly property string towBar: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="2" stroke-linecap="round"><path d="M9 11h6v5H9z"/><path d="M12 16v4"/><circle cx="12" cy="21" r="1.5" fill="white"/><path d="M7 8a7 7 0 0 1 10 0"/></svg>'

    // 5. Side View Left (Sườn trái)
    readonly property string sideLeft: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="2"><rect x="9" y="4" width="6" height="16" rx="3"/><path d="M5 9a4 4 0 0 0 0 6"/><path d="M2 7a8 8 0 0 0 0 10"/></svg>'

    // 6. Side View Right (Sườn phải)
    readonly property string sideRight: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="2"><rect x="9" y="4" width="6" height="16" rx="3"/><path d="M19 9a4 4 0 0 1 0 6"/><path d="M22 7a8 8 0 0 1 0 10"/></svg>'

    // 7. 360 Surround View (Toàn cảnh 360)
    readonly property string cam360: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="1.8"><rect x="9" y="5" width="6" height="14" rx="3"/><path d="M5 9a4 4 0 0 0 0 6"/><path d="M19 9a4 4 0 0 1 0 6"/><path d="M9 2a4 4 0 0 1 6 0"/><path d="M9 22a4 4 0 0 0 6 0"/></svg>'

    // 8. Settings (Cài đặt bánh răng)
    readonly property string settings: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="2" stroke-linecap="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>'

    // 9. Recording / Dashcam (Ghi hình)
    readonly property string recording: 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M23 7l-7 5 7 5V7z"/><rect x="1" y="5" width="15" height="14" rx="2" fill="white"/></svg>'
}
