# Madeira translation experiment

Nguyên mẫu đo độ lớn của công việc khi chuyển x64 ↔ ARM64EC. Chạy cùng một
thuật toán và so checksum qua bốn đường:

1. Vòng lặp x64 trong EXE: trên Madeira, FEX dịch đường này.
2. Gọi helper ARM64EC cho từng phần tử: 65.536 lần qua ranh giới ABI.
3. Gọi helper theo lô 64 phần tử: 1.024 lần qua ranh giới ABI.
4. Giao cả khối cho helper ARM64EC: một lần qua ranh giới ABI.

Đây là tải CPU tổng hợp, **không phải GTA V, benchmark đồ họa hoặc phép dự đoán
FPS game**. Native DLL có CHPE metadata, code ranges và entry thunks thật.
Checksum phải bằng nhau; sai checksum hoặc helper không phải ARM64EC thì phép
so bị từ chối.

## Thử trên iPad

- Trên bản Madeira mới: vào Settings › Send diagnostic log › Run CPU translation
  comparison. App đã có sẵn EXE/DLL, tự kiểm tra và chép vào thư mục Tools.
  Không cần giải nén hoặc Add game. Phép thử còn bật cache CPU thử nghiệm để
  log cho biết có tái sử dụng mã ARM64 đã xác minh hay không. Khởi động lại
  Madeira giữa các lần chạy và gửi log sau khi phép thử kết thúc.
- Các bước dưới đây dành cho IPA cũ chưa tích hợp nút thử:
- Giải nén `Madeira-translation-lab.zip` vào cùng một thư mục trong drive_c,
  chẳng hạn `Tools/TranslationLab`. Giữ DLL cạnh EXE.
- Thoát phiên game đang chạy và mở lại Madeira. Add game chọn
  `madeira-translation-lab.exe`, kiến trúc 64-bit. Không cần Steam.
- Chạy với JIT như một chương trình Windows. Sau khi nó kết thúc, gửi log
  của phiên này; kết quả có tag `[translation-lab]`.
  Chương trình cũng ghi `C:\madeira-translation-lab.txt`. Madeira có collector
  mới sẽ đính kèm file này; trên IPA cũ, nếu stdout không có kết quả thì gửi
  riêng file trong `drive_c`.
- So `local-x64-ms`, `native-small-ms`, `native-chunk64-ms` và
  `native-batch-ms`. Lặp lại vài lần với nhiệt độ tương tự.

Thư viện kiểm tra không thay game, DRM, DLL hệ thống hay cài đặt đồng bộ bộ nhớ.
Code có thể đo overhead của cơ chế chuyển ABI đang dùng nhưng chưa triển khai
proxy command buffer cho Direct3D hoặc chuyển mã GTA V sang ARM64.

## Build và host control

```sh
python3 build/translation-lab/build.py --toolchain /path/to/llvm-mingw/bin --out /tmp/translation-lab
```

Trên Windows x64 có thể thêm `--smoke`. Control DLL cũng là x64: kiểm tra
thuật toán/loader hoạt động, **không đo FEX hay iPad**. CI build ARM64EC trên
macOS và chạy control EXE trên Windows.

## Quyết định sau phép đo

Nếu native theo lô nhanh nhưng lời gọi native nhỏ chậm, ưu tiên gom công việc
ở các thư viện có source thay vì chuyển từng hàm nhỏ sang native. Một proxy
Direct3D x64 ghi command packets rồi replay bằng ARM64EC là ứng viên kế tiếp;
cần kiểm chứng lifetime COM/resource, Map/GetData, thứ tự lệnh và fallback
trước khi dùng cho game thật.
