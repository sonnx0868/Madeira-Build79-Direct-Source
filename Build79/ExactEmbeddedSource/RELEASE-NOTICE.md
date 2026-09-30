# Madeira 0.34 Compatibility · Release 6 candidate

Community compatibility build for legally owned BeamNG.drive 0.34 files.
No game files are included. This build is experimental and unaffiliated with
BeamNG GmbH, Apple, or CodeWeavers.

Changes from its preserved working baseline: focused BeamNG 0.34 launcher,
unrelated game and diagnostic-test UI removed, clearer setup/support text, clean app
metadata, and diagnostic helper executables removed. The proven runtime engine
is unchanged apart from the separately documented portable desktop dimensions.
Release 4 introduced the two reviewed app-local Wine CEF dependencies required
by a fresh BeamNG 0.34 installation. Release 5 removes a Menu launch action
that depended on an unsupported Wine Explorer hotkey and directs users to the
working desktop shortcut. Release 6 locates the Madeira guest's actual dyld
image before calling its existing input/status functions, avoiding an image-0
assumption when launched by LiveContainer. It also keeps support-file setup and
disabled-controller cleanup off UIKit startup, and records UI milestones in
the native log. The iOS 26 LiveContainer startup test passed; Wine and BeamNG
launches in that environment still need validation. No BeamNG game files are
included.
