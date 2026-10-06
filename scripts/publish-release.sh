#!/bin/bash
# Optional Codemagic step. Set GITHUB_RELEASE_TOKEN in the build environment.
set -euo pipefail
[[ -n "${GITHUB_RELEASE_TOKEN:-}" ]] || { echo "GitHub release skipped: GITHUB_RELEASE_TOKEN is not configured."; exit 0; }
root="$(cd "$(dirname "$0")/.." && pwd)"
ipa="$1"
test -s "$ipa"
command -v gh >/dev/null || brew install gh
export GH_TOKEN="$GITHUB_RELEASE_TOKEN"
export GH_PROMPT_DISABLED=1
commit="$(git -C "$root" rev-parse HEAD)"
build="$(cat "$root/build/release-info/build-number.txt")"
version="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["version"])' "$root/build/release-info/madeira-update.json")"
tag="v${version}-build${build}-${commit:0:7}"
notes="$root/build/release-info/notes.txt"
printf 'Madeira source build %s (%s).\n\nSource commit: %s\n\nIncludes diagnostic log upload and in-app update checks.\n' "$version" "$build" "$commit" > "$notes"
repo="sonnx0868/Madeira-Build79-Direct-Source"
if ! gh release view "$tag" --repo "$repo" >/dev/null 2>&1; then
  gh release create "$tag" --repo "$repo" --target "$commit" --prerelease \
    --title "Madeira $version source build $build" --notes-file "$notes" --draft
fi
gh release upload "$tag" "$ipa" "$root/build/release-info/madeira-update.json" --repo "$repo" --clobber
gh release edit "$tag" --repo "$repo" --draft=false
