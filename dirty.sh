#!/usr/bin/env bash
# commit-submodules-interactive.sh
#
# Walk all git submodules with changes and offer to:
#   - show status
#   - stage everything
#   - commit with a per-submodule message
#
# Run from the project root: ./commit-submodules-interactive.sh

set -Eeuo pipefail

# Ensure we're in a git repo
if ! git rev-parse --show-toplevel >/dev/null 2>&1; then
  echo "Not inside a git repository."
  exit 1
fi

REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "$REPO_ROOT"

# Collect all submodule paths from .gitmodules
if [ ! -f .gitmodules ]; then
  echo "No .gitmodules found – no submodules to process."
  exit 0
fi

mapfile -t SUBMODULES < <(git config --file .gitmodules --get-regexp path | awk '{print $2}')

if [ "${#SUBMODULES[@]}" -eq 0 ]; then
  echo "No submodules found in .gitmodules."
  exit 0
fi

echo "Scanning submodules for changes..."
echo

for path in "${SUBMODULES[@]}"; do
  # Skip if submodule directory is missing
  if [ ! -d "$path" ]; then
    echo "Submodule path missing, skipping: $path"
    continue
  fi

  pushd "$path" >/dev/null

  # Check if this is actually a git repo
  if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "Not a git repo, skipping: $path"
    popd >/dev/null
    continue
  fi

  # Check if there are any changes (tracked or untracked)
  if [ -z "$(git status --porcelain)" ]; then
    echo "[CLEAN] $path"
    popd >/dev/null
    continue
  fi

  echo
  echo "============================================================"
  echo "[DIRTY] Submodule: $path"
  echo "------------------------------------------------------------"
  git status
  echo "============================================================"
  echo

  # Ask whether to commit this one
  read -r -p "Commit changes in $path ? [y/N] " answer
  case "$answer" in
    [yY][eE][sS]|[yY]) ;;
    *) 
      echo "Skipping $path"
      popd >/dev/null
      continue
      ;;
  esac

  # Stage everything (modified + untracked)
  git add .

  # If still nothing staged (e.g., only ignored files), skip
  if git diff --cached --quiet; then
    echo "Nothing staged after git add . – skipping commit for $path"
    popd >/dev/null
    continue
  fi

  # Ask for commit message
  default_msg="Update submodule $path"
  read -r -p "Commit message for $path [${default_msg}]: " msg
  if [ -z "${msg}" ]; then
    msg="$default_msg"
  fi

  git commit -m "$msg"
  echo "Committed changes in $path"
  popd >/dev/null
done

echo
echo "============================================================"
echo "Submodule pass complete."
echo "Top-level repo status:"
echo "------------------------------------------------------------"
git status
echo "============================================================"

# Optional: offer to commit top-level changes too
if [ -n "$(git status --porcelain)" ]; then
  echo
  read -r -p "Stage and commit top-level changes as well? [y/N] " top_answer
  case "$top_answer" in
    [yY][eE][sS]|[yY])
      git add .
      if git diff --cached --quiet; then
        echo "Nothing staged at top level – nothing to commit."
      else
        default_top_msg="Update top-level repo after submodule commits"
        read -r -p "Top-level commit message [${default_top_msg}]: " top_msg
        if [ -z "${top_msg}" ]; then
          top_msg="$default_top_msg"
        fi
        git commit -m "$top_msg"
      fi
      ;;
    *)
      echo "Leaving top-level changes uncommitted."
      ;;
  esac
fi

echo "Done."
