# Log từ iPad, cập nhật IPA và tiếp tục ở máy nhà

## Server log riêng

Server: https://madeira-diagnostics-sonnx0868.desprinterval-team2.chatgpt.site

Sites project ID: `appgprj_6ac4905c4c6881919a5b9e065c3fd854`.

Server chỉ cho tài khoản chủ sở hữu truy cập. Giữ quyền truy cập này: API nhận
log dựa vào xác thực của Sites, nên không chuyển server sang public.

1. Cài bản Madeira có tính năng này trên iPad.
2. Mở server trong Safari, đăng nhập cùng tài khoản ChatGPT, chọn **Tải cấu hình iPad**.
3. Madeira → **Settings → Diagnostics → Send diagnostic log → Import server configuration**.
4. Chọn file `madeira-log-server.json` vừa tải. Khóa được lưu trong Keychain của iPad.
5. Khi test lỗi, mở lại màn hình này, chọn **Current session** nếu chưa đóng app,
   hoặc **Previous session** nếu đã mở lại Madeira (kể cả không crash), điền
   tên game/vấn đề rồi gửi. Chỉ giữ một phiên trước; gửi trước khi mở lại lần nữa.
6. Gửi mã log cho Codex. Cài/kết nối plugin **Madeira Diagnostics** ở mỗi máy
   để Codex gọi `list_game_logs` và `read_game_log`.

Nếu upload lỗi, file log vẫn ở iPad. Gửi lại cùng báo cáo dùng cùng ID để tránh
nhân đôi khi server đã nhận file nhưng iPad chưa nhận được phản hồi. Sau khi
gửi thành công, lần gửi tiếp theo chụp nội dung mới. Không tự gửi nền.

`Extended logging` bổ sung metadata phiên test, bản build/source commit, model
thiết bị, iPadOS, số CPU/RAM, kiến trúc game/API đồ họa, display/controller và
một danh sách cấu hình hiệu năng cố định. Ghi mốc launch, game-visible, lỗi
launch và finish. Trong phiên game, tối đa mỗi 10 giây ghi FPS trung bình,
memory footprint, thermal state, kích thước monitor và drawable thực tế,
FPS mode và CPU được báo cho game. Trước frame đầu, drawable ghi 0x0 để không
nhầm kích thước khởi tạo với độ phân giải game. Không xuất toàn bộ biến môi trường, tài
khoản, khóa server hay launch arguments vào metadata.

Log phiên trước giữ sidecar metadata của bản build đã tạo nó. Log cũ không có
sidecar được đánh dấu build unknown. File lớn hơn 20 MB giữ 64 KB đầu và phần
cuối, có marker thông báo phần giữa bị lược bỏ. Server lưu bytes trong R2 và
metadata trong D1; Codex đọc từng đoạn tối đa 64 KB hoặc tải file đầy đủ khi cần.

Source server ở `services/log-server/` trên máy đang làm việc và được Sites
đồng bộ vào repo riêng của Site. Thư mục này không được commit vào repo app.
Trên máy khác, yêu cầu Codex mở Site theo project ID trên bằng Sites hosting
để lấy đúng source hiện có; không tạo Site mới. Khóa runtime do Sites quản lý.

## Cập nhật app

**Settings → App updates → Check for updates** kiểm tra GitHub Releases của
`sonnx0868/Madeira-Build79-Direct-Source`. **Include test builds** bật mặc định
vì các IPA hiện tại là prerelease. Nút **Download IPA in browser** mở asset
IPA trên GitHub. Cài IPA bằng công cụ sideload bạn vẫn dùng.

Mỗi workflow Codemagic tạo cả IPA và `build/release-info/madeira-update.json`.
Khi đăng release thủ công, đính kèm cả hai asset. Manifest ghi version, build,
source commit và đúng tên IPA. Nó giúp nhận biết build mới cùng version, không
báo lại source commit đang cài. Release cũ thiếu manifest chỉ được nhận là bản
mới khi semantic version lớn hơn; không đoán thứ tự từ tên tag chứa commit hash.

Build number lấy từ số commit sau khi tải đầy đủ Git history, áp dụng cho app
và JIT helper bằng `CURRENT_PROJECT_VERSION`. Giữ chuỗi lịch sử tiến về phía
trước; khi dùng lịch sử khác/đã squash, kiểm tra build number trước khi publish.

Nếu muốn Codemagic tự đăng prerelease, thêm biến secret `GITHUB_RELEASE_TOKEN`
có quyền Contents write với repo này. Script tạo draft, tải cả IPA và manifest,
rồi publish release hoàn chỉnh. Khi không có token, bước này bỏ qua và IPA vẫn
là artifact Codemagic. Không đặt token trong source hoặc file handoff.

## Tiếp tục trên máy khác

Nhánh sửa: `codex/log-upload-updates`, bắt đầu từ
`origin/codex/v0.1.4-source-build` (`b2cc2a5`).

Nếu GitHub chưa cấp quyền push cho máy công ty, server riêng có link **Tải source
để tiếp tục ở máy khác**. File `madeira-source-changes.bundle` chứa commit của
nhánh sửa (không chứa khóa server). Trên máy nhà, clone repo bình thường, tải
bundle vào thư mục đó rồi chạy:

```powershell
git clone https://github.com/sonnx0868/Madeira-Build79-Direct-Source.git Madeira
Set-Location Madeira
git bundle verify .\madeira-source-changes.bundle
git fetch .\madeira-source-changes.bundle refs/heads/codex/log-upload-updates:refs/heads/codex/log-upload-updates
git switch codex/log-upload-updates
git submodule update --init --recursive
```

Chạy pipeline patch của repo trước các kiểm tra liên quan đến Wine:
`bash scripts/apply-wine-patches.sh` (Git Bash/WSL trên Windows hoặc terminal
macOS). Thay đổi trong submodule Wine lúc này là bản patch có sẵn của v0.1.4,
không phải commit submodule mới.

Sau khi tài khoản GitHub có quyền ghi và nhánh đã được push, trên máy nhà:

```powershell
git clone --branch codex/log-upload-updates --recurse-submodules https://github.com/sonnx0868/Madeira-Build79-Direct-Source.git Madeira
Set-Location Madeira
```

Nếu đã clone:

```powershell
git fetch origin
git switch codex/log-upload-updates
git pull --ff-only
git submodule update --init --recursive
```

Mở thư mục này trong Codex và yêu cầu đọc `docs/REMOTE_SUPPORT.md`, kiểm tra
nhánh hiện tại rồi tiếp tục. Git đồng bộ source/commit; không giả định chat local,
toolchain, file chưa commit hay secret sẽ tự chuyển giữa hai máy.

Build IPA cần macOS/Xcode, hoặc workflow Codemagic `madeira-v015-bootstrap`.
`madeira-v015-ipa` chỉ dùng khi có native bundle đã rebuild tương thích.
Windows kiểm tra source/server được, nhưng không xác nhận build
hay chạy IPA trên iPad. Codemagic bắt buộc chạy
`tests/host/check-remote-support.py --require-swift` trước khi build app.

Native bundle của workflow cached dùng các biến `MADEIRA_NATIVE_DEPS_URL` và
`MADEIRA_NATIVE_DEPS_SHA256` trong nhóm `madeira_build_inputs` hiện có; token
download private tùy chọn là `MADEIRA_NATIVE_DEPS_TOKEN`.
