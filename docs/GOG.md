# GOG account and installer integration

GOG OAuth follows the public Galaxy client protocol observed in Heroic/gogdl.
Sign in uses GOG's own web page and accepts only its HTTPS embed.gog.com
on_login_success callback. Tokens stay in the Keychain. Token URLs and response
payloads are not logged. Sign-out invalidates pending token/library responses.

Owned Windows games are listed through embed.gog.com/account/products with
bounded pagination. Installer metadata comes from api.gog.com/products with
downloads expansion, matching the GOG offline-installer API. English Windows
installers are preferred. All EXE/BIN parts are downloaded with progress/cancel,
size verification and MD5 verification when the metadata supplies a digest.
Bearer headers are used only for official API hosts, not signed CDN requests.

The original Windows installer opens through Madeira's existing installer flow.
Its suggested folder is C:\Program Files\GOG Games\<product id>. After installation,
Find installed game reads goggame-<id>.info, selects the primary FileTask and
registers the executable, arguments and working directory. Installed games are
normal library entries and remain playable after sign-out. Store rows filter
private entries/groups before displaying names or counts.

This is account/library plus offline-installer support. It does not implement
Galaxy depot extraction, Galaxy multiplayer, overlay or GOG cloud synchronization.
Completed installer parts stay on disk on cancel; retry presently downloads the
selected package again. Login/real downloads require device/account confirmation.

Sources: https://github.com/Heroic-Games-Launcher/heroic-gogdl/blob/main/gogdl/auth.py
https://github.com/Heroic-Games-Launcher/heroic-gogdl/blob/main/gogdl/api.py
https://github.com/Heroic-Games-Launcher/heroic-gogdl/blob/main/gogdl/dl/managers/linux.py
