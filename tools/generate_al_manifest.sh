#!/usr/bin/env bash

# Generates or updates manifest.json for update tool
# Usage:  ./generate_al_manifest.sh /path/to/manifest.json
# Needs:  jq, sha256sum, GNU findutils, GNU coreutils (stat)

set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 path/to/manifest.json" >&2
  exit 1
fi

manifest=$(realpath "$1")
root_dir=$(dirname "$manifest")

# If manifest doesn’t exist → create a skeleton
if [[ ! -f "$manifest" ]]; then
  cat >"$manifest" <<EOF
{
  "schema": "com.example.launcher/manifest-v2",
  "generated": null,
  "launcher": { "version": 0 },
  "files": []
}
EOF
  echo "Created new manifest: $manifest"
fi

# Whitelisted relative paths under $root_dir
targets=("Data/Launcher" "Scripts/Launcher" "AmberLauncher" "AmberLauncher.exe")

# Current/next launcher version
current_version=$(jq '.launcher.version' "$manifest")
next_version=$(( current_version + 1 ))

tmp_files=$(mktemp)
printf '[]' > "$tmp_files"

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

  rel="${f#$root_dir/}"
  sha256=$(sha256sum "$f" | awk '{print $1}')
  size=$(stat -c%s "$f")

  jq --arg path "$rel" --arg sha "$sha256" --argjson size "$size" \
     '. + [{path:$path, sha256:$sha, size:$size}]' \
     "$tmp_files" > "${tmp_files}.new"
  mv "${tmp_files}.new" "$tmp_files"
done

jq \
  --arg schema "com.example.launcher/manifest-v2" \
  --arg generated "$(date -u +'%Y-%m-%dT%H:%M:%SZ')" \
  --argjson version "$next_version" \
  --slurpfile files "$tmp_files" \
  '.schema          = $schema
   | .generated        = $generated
   | .launcher.version = $version
   | .files           = $files[0]' \
  "$manifest" > "${manifest}.tmp"

mv "${manifest}.tmp" "$manifest"
rm "$tmp_files"

echo "$(basename "$manifest") updated → version ${next_version}"
echo "Scanned (relative to manifest dir): ${targets[*]}"
