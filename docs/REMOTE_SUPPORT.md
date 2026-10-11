# Log từ iPad, cập nhật IPA và tiếp tục ở máy nhà

## Liar's Bar: lag ở build 600

Report `7ee2f3ca-2300-431e-9eed-9f5168bf5d9b` là Previous session,
build 600 / `70883fb`, iPad14,6 (M2, 16 GB). FPS có lúc gần 60,
nhưng từ 18:44:44 đến 18:51 phần lớn chỉ 15–18 FPS. Drawable 800×600;
thermal luôn 0. Footprint đạt 14.323 MB rồi giữ khoảng 12.900 MB.
Không dùng tổng `LIVE=26366MB` của census làm RAM thực: log tự ghi đó
là requested capacity; Metal `currentAllocatedSize` cuối phiên là 7.924 MB.

Pipeline cache hoạt động, nhưng vẫn có miss và native creation dài nhất
296,49 ms ở mốc 192 creations. Điều này giải thích một phần khựng khi
gặp pipeline mới, không chứng minh là nguyên nhân của nhiều phút FPS thấp.
Các mẫu Present cuối phiên có inside-average khoảng 41–57 ms; chưa có
GPU execution timing đủ để tách chính xác GPU, driver và CPU trong Present.
Player.log bị cắt đầu, phần cuối có hàng nghìn dòng debug và lỗi font lặp;
không dùng nó để khẳng định RTT/packet loss hoặc bước tìm trận cụ thể.

Hai chi phí có thể loại bỏ ở app đã xác định từ source và log:

- Steam/Dock chưa nhận `DXMT_CENSUS_THROTTLE=1` của profile game ngoài.
  Toàn bộ library launch nay mặc định giới hạn census mỗi 10 giây;
  cấu hình/env explicit vẫn được giữ, cảnh báo bộ nhớ vẫn báo ngay.
- Timer app-side gọi `ios_dump_all_thread_stacks()` mỗi 20 giây vẫn
  chạy khi heavy runtime profilers tắt (16.598 dòng trong phiên này).
  Timer và callback nay đều cần opt-in `MADEIRA_RUNTIME_PROFILERS`.

Hai thay đổi này không thay texture, giới hạn FPS, đồng bộ hay API Steam.
Chưa đo mức tăng FPS trên iPad; cần build IPA mới để đối chứng cùng cảnh.

## DST: thư mục cấu hình và cửa sổ ở build 600

Report `f33b5e30-7b07-4fab-9b4c-90a8b697acb4`, build 600, ghi
`STATUS_OBJECT_PATH_NOT_FOUND` (`0xc000003a`) khi tạo
`C:\users\mobile\Documents\Klei\` và các thư mục con DST.
Template chỉ có Documents của user tạo template (`mythic`); bootstrap
trước đây chỉ đảm bảo LocalLow cho user đang chạy. Bootstrap nay tạo
Documents và LocalLow trước wineserver; giữ opt-out riêng LocalLow,
không đổi registry/shell-folder hay thay file/save đang có.

Log cũng ghi D3D9 swapchain windowed 1280×720, cửa sổ `0x2007e`,
client `{4,30,1284,750}` bên trong desktop 2560×1440. Dock nay chọn
đúng HWND từ scene classifier hiện có và phóng client này vào vùng chơi.
Không đổi độ phân giải render hoặc kích thước window trong Wine;
touch và cursor dùng phép biến đổi thuận/ngược với hệ số X/Y riêng.
Begin/finish hoặc DestroyWindow bỏ lựa chọn cũ; cửa sổ client/installer
không được chọn làm game trước bước showGame.

Fit vẫn có viền đen khi tỉ lệ game khác iPad. Fill phủ màn hình và cắt
mép; Stretch phủ màn hình và có thể làm hình giãn. Test host dùng
Foundation/QuartzCore thật để kiểm tra tạo thư mục Klei tuần tự, giữ save,
file collision, và tọa độ Fit/Stretch. Chưa chạy đối chứng trên iPad.

Đối chiếu lỗi cấu hình:
https://support.klei.com/hc/en-us/articles/360029882171-Getting-a-Unable-to-write-to-files-in-the-config-directory-Error-Message

## Server log riêng

### Silksong: khởi động, analog và mất input ở build601

Report `144de888-c458-4902-b3b3-c1a301c1c47e`: start 21:23:25,
first visible render khoảng 21:24:15, drawable 2560×1440, cpuReported=4.
Mono loaded assemblies 6,437 s, domain reset 0,133 s; các Present gap lớn
13.136 và 29.196,6 ms nằm ngoài phần inside Present. Unload/GC cũng có
các lần khoảng 175–352 ms. Chưa quy mọi thời gian đợi này cho shader.
Mốc cuối phiên có thermal=2; giảm độ phân giải có thể giảm tải đồ họa/nhiệt,
nhưng không tránh được cold CPU compilation và scene loading.

Log controller có slot0 paired=1 và physical input đến native publisher.
64 dòng đầu chủ yếu là nút A rồi cần xuống; không có enough samples ở các
đoạn người dùng báo gạt ngang thành lên hoặc mất input giữa phiên. Không
khẳng định đảo trục hay Bluetooth disconnect từ log này. Người dùng xác
nhận lệch dọc nhỏ khi gạt ngang cũng thành hướng lên: thêm eight-way gate
cho Silksong với Analog opt-out; mở rộng analog diagnostics, owner/lifecycle
logs và rebind profile thay thế trên cùng GCController.

CPU cache Automatic của Silksong nay dùng verified reuse của DLL hiện có.
Đây là thử nghiệm để đo repeated startup; không có số đo device chứng minh
đã xóa khựng. Không thay FEX core, exception/SMC/TSO hoặc Mono budget hooks.

### Liar's Bar: tay cầm và đối chứng build 601

Report `219a73d4-d127-456b-be03-2d0a60924612`, build 601, xác nhận
slot 0 có tay cầm vật lý trước Wine (`paired=1`), cần trái và nút A
được xuất qua `xinput-publish`. Đó là bằng chứng phía host nhận input;
chưa phải bằng chứng input layer của game dùng được thiết bị Windows.
Game đang ở controller default, không phải keyboard/mouse mapping.
Steam Store hiện không công bố controller support cho app 3097560.
Madeira có thể đổi riêng game sang **Controller → Keyboard and mouse**;
RS là chuột, RT click trái, Start là Esc; Controller binds chỉnh các phím.
Dock chạy client headless và không tự áp các community Steam Input layouts.

Early reservation trước đây dùng một cờ touch cho cả tay cầm vật lý.
Log do đó ghi `touch=1` dù overlay có 0 controller controls. Publisher nay
giữ reservation vật lý riêng, tắt publication đó trong keyboard/mouse mode,
kể cả lúc menu library giữ input; nguồn touch thật vẫn độc lập.

Lag chưa hết: cuối phiên 601 có 9,8–11 FPS; footprint giữ khoảng 13 GB,
peak Metal allocated 11.927 MB, compressed memory cuối phiên khoảng 845 MB.
Tổng census giảm 1.387 → 63, all-thread dumps 30 → 0, nên bản giảm thống kê
đã có tác dụng đúng chức năng. Không coi các cảnh game khác nhau là benchmark
FPS đối chứng: log này không chứng minh bản 601 làm FPS giảm so với 600.
Native pipeline creation dài nhất 664,81 ms; median Present inside ở phần
sau khoảng 49,2 ms. Bộ frame GPU instrument trong port vẫn là stub; không
dùng các số GPU/commit bằng 0 để khẳng định GPU rảnh. Cần GPU timing thật
và cùng scene để tách CPU/driver/GPU, trước khi hứa mức tăng FPS.

Nguồn Steam Input:
https://partner.steamgames.com/doc/features/steam_controller/legacy_mode

### Ren'Py / CrossRealms: child runtime ở build 601

Report `d8211d45-3a42-413c-9542-3bd290b61f92`, CrossRealms, build 601,
khởi động qua native session host và x64 child. Sau đó có fault native
`NtProtectVirtualMemory+0x71c`, địa chỉ `0x7023004000`, trước khi tải
librenpython/libpython3.9. Python gọi các cmd helper để đọc version Windows;
cuối phiên x64 RIP 0, exit `0xc0000005`, chưa có khung hình hoặc log phụ.
Không coi cảnh báo comctl32/activation context hay lựa chọn OpenGL là
nguyên nhân đã chứng minh của crash cuối.

App nay nhận diện DLL Ren'Py ở thư mục EXE hoặc `lib/*windows-*` và chọn
isolated/direct launch, giống đường tránh reusable runtime của Unity.
Đây là workaround của routing; chưa sửa core memory protection hay xác nhận
game chạy trên iPad. Nếu engine reusable đã sống, đường kiểm tra cũ vẫn yêu
cầu restart, không bootstrap chồng lên engine. Test Swift dùng các thư mục
Ren'Py thật theo cấu trúc phân phối để kiểm tra root/nested launchers, giữ
reuse cho game native thường và giữ isolation của Unity/CPU experiments.

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

Build IPA cần macOS/Xcode, hoặc workflow Codemagic `madeira-v016-bootstrap`.
`madeira-v016-ipa` chỉ dùng khi có native bundle đã rebuild tương thích.
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

### Build 597: đường chuyển CPU dùng nhầm assembly Windows

Log `abe0dade-2888-4d0c-837f-ff7e4b4a185e` là phiên Don't Starve Together
qua Madeira Dock, dù người gửi mô tả đang gửi log game cài ngoài. Build 597
đã tạo dispatcher nhưng chưa có frame game. Fault đầu ở pool PC `0x149e1b710`
thuộc trampoline x18 của `libarm64ecfex.dll`, lệnh `str x16,[sp,#-16]!`,
SP `0x702a6af4d8` lệch 8 byte. Các exception sau đó mất RSP/RIP; không dùng
các lỗi sau làm nguyên nhân gốc. Không có log `[cpu-cache]` kích hoạt trong phiên.

Script rebuild chỉ truyền `FEX_IOS_HOST` qua C_FLAGS/CXX_FLAGS. Module.S
thiếu cờ ở ASM_FLAGS, nên lắp ráp nhánh Windows: sau khi push địa chỉ trả
về x64 8 byte, `enter_jit` đọc `ldr x17,[x18,#0x1788]`. Patcher x18 chung
chèn trampoline dùng SP, gây alignment fault trên iOS. Nhánh iOS sẵn có
dùng TPIDRRO_EL0/TSD trực tiếp và có FFS bypass cùng sweep fences.

Builder truyền cùng cờ cho ASM và kiểm tra mã máy trong DLL qua
`tests/host/check-fex-transition-arch.py`: RetToEntryThunk, BeginSimulation
phải có bộ ba đọc TSD; ExitToX64 phải có FFS bypass. Native preflight dùng
cùng kiểm tra, nên marker C++ của cache không đủ để nhận một native bundle
là tương thích. Sửa này cần source-bootstrap rebuild; vẫn phải đo lại việc
chạy game trên iPad bằng IPA mới. Log cài ngoài và Steam khác nếu có cần
đối chiếu riêng, không suy rằng một log đã chứng minh mọi lỗi đều hết.

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
