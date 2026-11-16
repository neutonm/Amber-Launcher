#!/usr/bin/env bash

# Generates or updates a game manifest from selected repository files
# Usage: ./generate_game_manifest.sh source_dir manifest.json
# Include list: relative files, directories or globs; blank lines and # comments are ignored
# Needs:  jq, sha256sum, GNU findutils, GNU coreutils (stat)

set -euo pipefail

readonly INCLUDE_FILE="release-files.txt"

if [[ $# -ne 2 ]]; then
  echo "Usage: $0 source_dir manifest.json" >&2
  exit 1
fi

root_dir=$(realpath "$1")
include_file=$(realpath "$root_dir/$INCLUDE_FILE")
manifest=$(realpath -m "$2")

if [[ ! -d "$root_dir" || ! -f "$include_file" ]]; then
  echo "Source directory and $INCLUDE_FILE must exist." >&2
  exit 1
fi

mkdir -p "$(dirname "$manifest")"

# If manifest doesn’t exist → create a skeleton
if [[ ! -f "$manifest" ]]; then
  cat >"$manifest" <<EOF
{
  "schema": "com.example.launcher/manifest-v2",
  "generated": null,
  "launcher": { "version": 16777216, "build": 0 },
  "files": []
}
EOF
  echo "Created new manifest: $manifest"
fi

# Current/next game update version
current_version=$(jq '.launcher.version' "$manifest")
next_version=$(( current_version + 1 ))

tmp_files=$(mktemp)
trap 'rm -f "$tmp_files" "${manifest}.tmp"' EXIT

shopt -s nullglob

scan() {
  local pattern path matched
  local IFS=

  while read -r pattern || [[ -n "$pattern" ]]; do
    pattern=${pattern%$'\r'}
    [[ -z "$pattern" || "$pattern" == \#* ]] && continue
    case "/$pattern/" in
      //*|*/../*)
        echo "Include paths must be relative: $pattern" >&2
        return 1
        ;;
    esac

    matched=false
    for path in "$root_dir"/$pattern; do
      [[ -e "$path" ]] || continue
      path=$(realpath "$path")
      if [[ "$path" != "$root_dir" && "$path" != "$root_dir/"* ]]; then
        echo "Include path is outside source directory: $pattern" >&2
        return 1
      fi
      if [[ -d "$path" ]]; then
        find "$path" -type f -print0
      elif [[ -f "$path" ]]; then
        printf '%s\0' "$path"
      else
        continue
      fi
      matched=true
    done
    if [[ "$matched" == false ]]; then
      echo "No files or directory match: $pattern" >&2
      return 1
    fi
  done < "$include_file"
}

scan | LC_ALL=C sort -zu |
while IFS= read -r -d '' f; do
  [[ "$f" == "$manifest" || "$f" == "${manifest}.tmp" ||
     "$f" == "$tmp_files" || "$f" == "$include_file" ]] && continue
  rel="${f#$root_dir/}"
  sha256=$(sha256sum < "$f" | awk '{print $1}')
  size=$(stat -c%s "$f")

  jq -nc --arg path "$rel" --arg sha "$sha256" --argjson size "$size" \
     '{path:$path, sha256:$sha, size:$size}' >> "$tmp_files"
done

jq \
  --arg generated "$(date -u +'%Y-%m-%dT%H:%M:%SZ')" \
  --argjson version "$next_version" \
  --slurpfile files "$tmp_files" \
  '.generated        = $generated
   | .launcher.version = $version
   | .files           = $files' \
  "$manifest" > "${manifest}.tmp"

mv "${manifest}.tmp" "$manifest"

echo "$(basename "$manifest") updated → version ${next_version}"
echo "Scanned: $root_dir"
