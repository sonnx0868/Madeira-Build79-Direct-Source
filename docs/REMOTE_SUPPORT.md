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
sidecar được đánh dấu build unknown. Khi gửi, báo cáo bổ sung các file Unity
`users/*/AppData/LocalLow/*/*/Player.log` và các tên log mạng Steam cố định trong
`Program Files (x86)/Steam/logs`. Chỉ chọn file được cập nhật từ mốc test đến
mốc kết thúc phiên; báo cáo phiên trước không lấy file đã bị phiên mới ghi đè.
Không duyệt save game, tài khoản hay toàn bộ prefix. Giới hạn 8 file, mỗi file
lấy tối đa 512 KB cuối; tổng báo cáo vẫn tối đa 20 MB. Phần log Madeira giữ
64 KB đầu và phần cuối khi cần nhường chỗ, có marker thông báo phần bị lược bỏ.
Metadata `companionLogs` cho biết số log phụ thực sự được đính kèm.
Server lưu bytes trong R2 và
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

## Chuẩn bị Steam và đo khựng lúc tải game

Chuẩn bị các one-time installs chạy ở background, chụp cấu hình trước khi
bắt đầu và chỉ công bố plan hoàn tất trên main actor. Registry được quét một
lần cho các DWORD cần tìm; log dùng lại kết quả đó. `[dock-prepare]` ghi các
mốc begin, registry-scanned và ready cùng thời gian chuẩn bị. Sau bước async,
Madeira kiểm tra lại trạng thái JIT, đăng nhập và phiên trước khi chạy Wine.
Các lần bấm Play khác chờ bước chuẩn bị kết thúc.

Wineserver gom các yêu cầu đánh thức thành một tín hiệu đang chờ. Chỉ bỏ cờ
sau khi semaphore thực sự được tiêu thụ, trước lúc quét request; timeout
không bỏ tín hiệu. Giữ tick fallback 1 ms và đường poll mạng hiện có.
Preflight yêu cầu `request-wake-coalesced-v1`, nên native bundle cũ phải
rebuild bằng source-bootstrap. Host CI kiểm tra burst đa luồng với semaphore
Mach thật và khả năng main actor tiếp tục chạy khi chuẩn bị prefix lớn.

FPS mỗi 10 giây, nhiệt độ, shader-cache hits và Player.log chưa đủ tách thời
gian tải asset, dịch mã x64, GC và tạo Metal pipeline. Không quy các đoạn
0 FPS cho một nguyên nhân duy nhất hoặc khẳng định hết khựng từ host tests.

## Cache pipeline Metal

Cache shader DXMT trung gian và cache pipeline GPU là hai tầng khác nhau.
`build/dxmt-ios/pipeline-cache.patch` nối render/compute pipeline của winemetal
vào `pipeline_cache_ios.m`. `MTLBinaryArchive` giữ pipeline đã biên dịch cho
GPU; namespace gồm tên GPU và phiên bản OS. Lookup archive là bất biến, writer
là object riêng, chạy tuần tự ở QoS background. Tối đa 128 descriptor đang
chờ, 2048 lượt học mỗi app run và file tối đa 64 MB. Ghi file tạm rồi rename;
không I/O đồng bộ trên đường vẽ. File cũ được giữ nếu serialize không thành công.

Archive thiếu, hỏng hoặc không chứa pipeline yêu cầu đều quay về biên dịch
Metal bình thường. Caller có archive/policy riêng giữ nguyên đường đó. Chỉ
học pipeline đã tạo thành công; không bỏ draw hoặc giả pipeline thành công.
Log `pipeline-cache` có hits/misses, thời gian native creation trung bình/tối
đa và trạng thái save. Đây là timing từng API call, không phải tổng thời gian
game đứng chờ. `env.MADEIRA_PIPELINE_CACHE=0` tắt lookup/training.

Tầng này hỗ trợ DXMT D3D9/11, không thay cache của backend Vulkan/OpenGL/D3D12.
Shader mới vẫn cần biên dịch lần đầu. Hiệu quả cache giữa các lần mở app và
ảnh hưởng CPU của background training phải đo trên iPad; cache không giải
quyết toàn bộ thời gian tải scene hoặc dịch mã CPU x64. Preflight yêu cầu
`pipeline-binary-v1`; dùng source-bootstrap để rebuild native dependencies.

API đối chiếu: https://developer.apple.com/documentation/metal/mtlbinaryarchive

## Liar's Bar: kiểm tra chất lượng kết nối

Log build 585 có Player.log: Unity Authentication lặp `Curl error 60`,
`SSL CA certificate error`, rồi QoS lỗi vì chưa có access token. Native UDP
đã ghi `darwin-tos-v1` và không còn unknown cmsg. Lỗi HTTPS xảy ra trước QoS;
không coi thông báo chất lượng kết nối là bằng chứng ping/packet loss xấu.

Unix crypt32 dùng chung từng lấy và xoá root certificate khi caller 64-bit
đọc nó. Caller sau có thể nhận danh sách rỗng; Wine còn đối chiếu danh sách
đó với root đã nhập. Sửa bằng cache CA bất biến và cursor riêng mỗi thread,
dùng cho cả 64-bit/WoW64, có dọn cursor khi thread kết thúc. Preflight yêu cầu
`shared-roots-v2`; cần rebuild source-bootstrap. Host test kiểm tra import
lặp, buffer resize và 128 lượt đọc đồng thời. Vẫn phải xác nhận đăng nhập
Unity thành công bằng log từ iPad.

Upload-before-quit dùng kết nối Steam CM native. Một lần bắt tay TLS lỗi
không tự chứng minh file save hỏng; flow cũ kết thúc ngay ở CM đầu tiên.
Transport thử tối đa ba endpoint khác nhau do directory cung cấp, đánh dấu
endpoint lỗi và dùng cache directory khi fetch gặp lỗi mạng. Cancellation
dừng retry; callback từ socket cũ không đóng socket mới. Log `steam-network`
ghi stage, host CM, error domain/code, không ghi credential/URL có token.
Certificate verification và xác nhận upload vẫn bắt buộc.

Log build 576 có SteamAPI khởi tạo thành công, DoH Cloudflare trả lời và
kết nối Steam truyền dữ liệu. Luồng `SteamNetworkingSockets` nhận UDP nhưng
Wine bỏ qua ancillary message IPv4 type 27. Trên Darwin, đó là `IP_RECVTOS`
với payload một byte; Windows cần `IP_TOS` với payload INT. Bản patch
`patches/wine-ios-udp-tos.patch` chuyển đúng kiểu và giữ các bit DSCP/ECN.
Không thay payload game, xác thực Steam hay kết quả kiểm tra chất lượng.

Đây là lỗi tương thích đã xác định, chưa chứng minh là nguyên nhân duy nhất
của “Connection quality check failed”. Báo cáo cũ không chứa Player.log nên
chưa có chi tiết timeout, RTT, packet loss hoặc trạng thái relay của game.
Sau khi cài IPA mới, thử tìm trận rồi gửi Current session để lấy log phụ.
So sánh cùng build qua Wi-Fi hiện tại và hotspot của nhà mạng khác nếu lỗi
vẫn xảy ra; không coi truy cập được Steam Store là bằng chứng UDP/relay tốt.

Video https://www.youtube.com/watch?v=DZtOWT2Sh8c hướng dẫn Windows sửa hosts
và DNS để truy cập Steam. Madeira đã có DoH cho miền Steam ở chế độ Automatic.
Không ghim IP từ video: CDN/relay có thể đổi địa chỉ. Windows Defender trong
video không phải firewall của iPad.

Nguồn đối chiếu:

- Apple XNU: https://github.com/apple-oss-distributions/xnu/blob/main/bsd/netinet/ip_input.c
- Microsoft Winsock: https://learn.microsoft.com/en-us/windows/win32/winsock/ipproto-ip-socket-options
- Valve UDP TOS: https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/src/steamnetworkingsockets/clientlib/steamnetworkingsockets_socketthread.cpp

Kiểm tra converter thực tế bằng `tests/host/check-udp-control.py --require-tools`.
Workflow bootstrap chạy kiểm tra này sau khi áp dụng patch. Preflight native
bắt buộc marker `darwin-tos-v1`; native bundle cũ cần rebuild source-bootstrap.

Native bundle của workflow cached dùng các biến `MADEIRA_NATIVE_DEPS_URL` và
`MADEIRA_NATIVE_DEPS_SHA256` trong nhóm `madeira_build_inputs` hiện có; token
download private tùy chọn là `MADEIRA_NATIVE_DEPS_TOKEN`.
