# Đối chiếu OpenGL test 7 và sửa build báo cáo

Ngày kiểm tra: 09/10/2026, múi giờ Asia/Bangkok.

Nguồn tham khảo: [c-gow/Madeira OpenGL test 7](https://github.com/c-gow/Madeira/releases/tag/opengl-test-7),
commit `a14444490087c9504a19b9a168482ab35df2fc6c`.
Build 557 của nhánh mình tương ứng `6fe2263979f897cddf8c5e94856fa920bda8b21a`.
Source trước lần sửa này là build 577 (`31900d3`).

| Thay đổi trong test 7 | Source 557/577 của mình | Xử lý |
| --- | --- | --- |
| Xóa scratch audio trước khi trả GetBuffer; downmix 7.1.4 | Chưa có | Nhập sửa từ upstream |
| Audio event theo ring fill, xử lý luồng client ngừng trả lời | Timer 10 ms cố định | Nhập sửa và bản sửa bổ sung giảm polling khi client không trả lời |
| AVAudioSession I/O 5 ms | Không yêu cầu I/O duration | Nhập; log ghi duration thực tế iOS cấp |
| Desktop OpenGL qua Mesa/Zink/MoltenVK, triangle fan và tên shader Metal | ANGLE cho OpenGL ES; chưa có Mesa backend | Đã tích hợp renderer, unix dispatch, build từ source, bundle và ký dylib; nhập đủ sáu patch Mesa |
| Thin reservations cho Mewgenics | Chưa có | Đã nhập dưới dạng thử nghiệm theo game, mặc định tắt; vẫn có giới hạn và rủi ro với mẫu cấp phát khác |
| Thêm gdiplus/mlang/sspicli 64-bit | Không có các DLL này trong arm64ec-windows | Rebuild từ Wine đang ghim, cùng opengl32/glu32; không chứng minh tăng FPS cho Unity/D3D11 |
| FPS overlay cho OpenGL | Overlay đọc present counter DXMT | Frame counter mới cộng frame GL và DXMT, dùng chung cho overlay và kiểm tra frame đầu |

Các tối ưu startup/shader cache đã thêm sau build 557 (async cache writer,
identity theo compiler source, giới hạn compiler workers, Unity startup sync,
swap theo headroom, quiet gameplay observers) được giữ trong source hiện tại.
Không có số đo thiết bị để kết luận các thay đổi audio làm tăng FPS của
Silksong hoặc giải quyết lỗi tìm trận Liar's Bar.

## Tích hợp và chỉnh sửa OpenGL

Trong Game details → **OpenGL & memory compatibility**:

- Automatic giữ ANGLE cho LÖVE/SDL OpenGL ES đã nhận diện; game desktop GL
  dùng Zink nếu các thư viện khởi tạo được. Native GLES là fallback khi Zink
  không khởi tạo được.
- Desktop OpenGL (Zink), Native OpenGL ES và ANGLE có thể chọn riêng theo game.
  Đổi native renderer rồi lưu và khởi động lại Madeira trước khi thử.
- Thin reservations mặc định tắt, chỉ bật cho Mewgenics/mẫu reserve tương tự.
  Đây là cách mô phỏng vùng reserve lớn bằng các head nhỏ: đuôi vùng báo cáo
  chồng lấn head kế tiếp. Commit đi xa có thể thất bại hoặc chạm vùng khác;
  không bật cho tất cả game và không coi nó là tối ưu FPS chung.

Các sửa bổ sung so với source test 7:

- Kiểm tra tên renderer khi context còn sống; không dùng chuỗi glGetString
  sau khi hủy context. Probe vẫn từ chối softpipe.
- Present OpenGL áp dụng giới hạn 60/30 FPS; không ép panel 120 Hz.
- Các cửa sổ dùng chung Metal library của shader present theo device.
- Bật Mesa disk shader cache, giới hạn mặc định 256 MB. Namespace theo hash
  của hai renderer binary, tránh sử dụng shader của bản compiler cũ.
- Native build dùng MoltenVK commit cố định, Mesa archive kiểm tra SHA256,
  Python build tools ghim version. Kiểm tra patch đã áp dụng bằng nội dung,
  không chỉ dựa vào marker; reconfigure cả Mesa cache có sẵn.
- Thin reserve yêu cầu giá trị `1` rõ ràng, tắt được sau khi cấu hình đã khởi
  tạo. Khởi tạo geometry nằm trong virtual mutex; giới hạn threshold 64 GB.
- Các lựa chọn renderer/thin của game được xuất lại sau cấu hình global,
  và game kế tiếp xóa lựa chọn renderer cũ, tắt thin nếu không được chọn.

Zink/MoltenVK không cung cấp geometry shaders của desktop GL đầy đủ.
Version override trong backend giúp một số game yêu cầu GL 3.3/4.1 khởi tạo;
game thật sự dùng tính năng Metal không hỗ trợ vẫn có thể lỗi shader/pipeline.

Workflow **OpenGL native integration** rebuild năm PE DLL ARM64EC và hai DLL
OpenGL/GLU ARM64 cho đường WoW64 của game 32-bit, win32u archive,
kiểm tra virtual memory/OpenGL unix source với iOS SDK, build MoltenVK/Mesa
và xuất artifact. Artifact này là các thành phần native, chưa phải IPA.
IPA vẫn cần **madeira-v015-bootstrap**; native bundle cũ bị preflight từ chối.

## Phần audio đã nhập

Giữ nguồn gốc các sửa của spitefulowl/Connor Gow:

- [95c70c1: GetBuffer silence và 7.1.4](https://github.com/c-gow/Madeira/commit/95c70c15de5ccdc62de8a897f53e7c5595c28e26)
- [fd8e1eb: event pacing theo ring fill](https://github.com/c-gow/Madeira/commit/fd8e1eb24aa665e726257cdeaee0ee207130cc4c)
- `791aa6b`: phần audio, thêm chế độ lead=0 và giảm polling khi client ngừng trả lời.
- [a144444: I/O 5 ms](https://github.com/c-gow/Madeira/commit/a14444490087c9504a19b9a168482ab35df2fc6c)

`env.MADEIRA_AUDIO_LEAD_MS` mặc định 60 ms, giới hạn theo dung lượng ring;
0 giữ timer 10 ms. Audio lead giúp chịu trễ wake-up nhưng tăng độ trễ âm thanh.
`env.MADEIRA_AUDIO_IO_MS` mặc định 5 ms, 1..40 chọn duration khác, 0 không
yêu cầu duration; iOS và route Bluetooth có thể cấp giá trị khác. Đổi các
thiết lập này rồi mở lại Madeira để tránh session giữ giá trị cũ.

## Hold the display at its maximum rate

Đã bỏ UI, catalog và runtime override. Cấu hình cũ
`env.MADEIRA_PROMOTE=1` không được đọc nữa. Chế độ 60/30 FPS luôn release
CADisplayLink ép refresh. Display maximum/Uncapped vẫn là lựa chọn FPS riêng.
Giảm giật thực tế cần kiểm tra lại trên iPad; source không hứa mức FPS cụ thể.

## Build lỗi

Log `build_unsigned_debug_app (1).log` dừng ở assert dòng 253 trong Swift
harness của `check-remote-support.py`, trước xcodebuild. Collector chuẩn hóa
đường dẫn bằng `resolvingSymlinksInPath`, nhưng test so URL nguyên bản của
temporary directory. macOS có thể viết cùng đường dẫn là `/var` và `/private/var`.
Sửa test so tập đường dẫn đã resolve ở cả hai phía. Lần chạy GitHub Actions
đã tái hiện thêm việc recursive enumerator bỏ sót Player.log; collector đổi
sang duyệt rõ hai cấp company/product, giữ kiểm tra số log, filter theo phiên,
giới hạn dung lượng và loại symlink ra ngoài prefix.

GitHub Actions `Host support and audio checks` chạy Swift tests trên macOS,
UDP/downmix behavior tests và syntax compile C/ObjC với iPhoneOS SDK.
Đây là kiểm tra source; IPA vẫn phải build bằng Codemagic source-bootstrap.
