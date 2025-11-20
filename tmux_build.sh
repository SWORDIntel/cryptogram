#!/bin/bash
set -euo pipefail

# A helper that runs the CRYPTOGRAM install while tailing the latest log
# inside a single tmux session so you can watch CMake output + logs live.

LOG_DIR="${LOG_DIR:-/tmp/cryptogram_builds_root}"
SESSION="cryptogram-build"
INSTALL_CMD="sudo ./build_all.sh --resume"

latest_log() {
    ls -t "${LOG_DIR}"/build_*.log 2>/dev/null | head -n1
}

main() {
    local log_file
    log_file="$(latest_log)"
    if [ -z "$log_file" ]; then
        echo "No build log found in ${LOG_DIR}; create one by running ./build_all.sh once."
        exit 1
    fi

    if tmux has-session -t "$SESSION" 2>/dev/null; then
        tmux kill-session -t "$SESSION"
    fi
    tmux new-session -d -s "$SESSION" "tail -F '$log_file'"
    tmux split-window -v -t "$SESSION" "cd '$(pwd)' && ${INSTALL_CMD}"
    tmux select-layout -t "$SESSION" even-vertical
    tmux attach -t "$SESSION"
}

main "$@"
