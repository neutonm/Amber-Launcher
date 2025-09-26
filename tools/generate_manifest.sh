#!/usr/bin/env bash

# Generates manifest.json for update tool
# Usage:  ./update_manifest.sh /path/to/manifest.json
# Needs:  jq, sha256sum, GNU findutils, GNU coreutils (stat)

set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 path/to/manifest.json" >&2
  exit 1
fi

manifest=$(realpath "$1")
[[ -f "$manifest" ]] || { echo "Manifest not found: $manifest" >&2; exit 1; }

root_dir=$(dirname "$manifest") # relative

# Whitelisted relative paths under $root_dir
targets=("Data" "Scripts" "Amber Launcher" "Amber Launcher.exe")

# launcher version
current_version=$(jq '.launcher.version' "$manifest")
next_version=$(( current_version + 1 ))

# build list of files
tmp_files=$(mktemp)
printf '[]' > "$tmp_files"

# Helper: emit every file (recursively for dirs) as NUL-separated list
scan() {
  local abs="$1"
  if [[ -d "$abs" ]]; then
    find "$abs" -type f -print0
  elif [[ -f "$abs" ]]; then
    printf '%s\0' "$abs"
  fi
}

for t in "${targets[@]}"; do
  scan "$root_dir/$t"
done | while IFS= read -r -d '' f; do

  [[ $(realpath "$f") == "$manifest" ]] && continue

  rel="${f#$root_dir/}" # path inside JSON = relative to root_dir
  sha256=$(sha256sum "$f" | awk '{print $1}')
  size=$(stat -c%s "$f")

  jq --arg path "$rel" --arg sha "$sha256" --argjson size "$size" \
     '. + [{path:$path, sha256:$sha, size:$size}]' \
     "$tmp_files" > "${tmp_files}.new"
  mv "${tmp_files}.new" "$tmp_files"
done

# write manifest
jq \
  --arg generated "$(date -u +'%Y-%m-%dT%H:%M:%SZ')" \
  --argjson version "$next_version" \
  --slurpfile files "$tmp_files" \
  '.generated        = $generated
   | .launcher.version = $version
   | .files           = $files[0]' \
  "$manifest" > "${manifest}.tmp"

mv "${manifest}.tmp" "$manifest"
rm "$tmp_files"

echo "$(basename "$manifest") updated → version ${next_version}"
echo "Scanned (relative to manifest dir): ${targets[*]}"
