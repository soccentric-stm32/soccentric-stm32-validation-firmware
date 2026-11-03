#!/usr/bin/env bash
#
# herdr.sh - Join this project's Herdr session, or create it if missing
#
# Author:  Sandesh Ghimire <sandesh@soccentric.com>
#
# Session name: stm32 (override with the first argument or HERDR_SESSION)
#
#   running  -> attach to it
#   stopped  -> start it again and attach (its workspaces are restored)
#   missing  -> start a new session with one workspace at the project root,
#               then attach
#
# Usage:
#   ./herdr.sh                     join or create the "stm32" session
#   ./herdr.sh <name>              join or create a session with another name
#   ./herdr.sh [name] --no-attach  make sure the session is running, don't attach
#

set -Eeuo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SESSION="${HERDR_SESSION:-stm32}"
ATTACH=1

info() { printf 'herdr.sh: %s\n' "$*"; }
die()  { printf 'herdr.sh: %s\n' "$*" >&2; exit 1; }

usage() {
    sed -n '/^# Usage:/,/^$/p' "$0" | sed 's/^# \{0,1\}//'
}

parse_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --no-attach) ATTACH=0 ;;
            -h|--help)   usage; exit 0 ;;
            -*)          die "unknown option: $1 (try --help)" ;;
            *)           SESSION="$1" ;;
        esac
        shift
    done
}

check_preconditions() {
    command -v herdr >/dev/null 2>&1 \
        || die "herdr not found on PATH (install: curl -fsSL https://herdr.dev/install.sh | sh)"

    # A Herdr client cannot run inside a Herdr pane.
    if [[ $ATTACH -eq 1 && "${HERDR_ENV:-}" == "1" ]]; then
        die "already inside a Herdr pane; detach first, or run with --no-attach."
    fi
}

# Print "running", "stopped" or "missing" for $SESSION.
session_state() {
    # `session list --json` is one line of compact JSON; split it into one
    # session object per line and look for ours.
    local entry
    entry="$(herdr session list --json 2>/dev/null \
        | sed 's/},{/}\n{/g' \
        | grep -F "\"name\":\"$SESSION\"" || true)"

    if [[ -z "$entry" ]]; then
        echo missing
    elif [[ "$entry" == *'"running":true'* ]]; then
        echo running
    else
        echo stopped
    fi
}

start_server() {
    cd "$PROJECT_DIR"
    nohup herdr --session "$SESSION" server >/dev/null 2>&1 &

    local _
    for _ in $(seq 1 60); do
        if herdr --session "$SESSION" workspace list >/dev/null 2>&1; then
            return 0
        fi
        sleep 0.25
    done
    die "timed out waiting for session '$SESSION' to start."
}

create_workspace() {
    herdr --session "$SESSION" workspace create \
        --cwd "$PROJECT_DIR" \
        --label "$(basename "$PROJECT_DIR")" \
        --focus >/dev/null \
        || die "could not create a workspace in session '$SESSION'."
}

attach() {
    if [[ $ATTACH -eq 0 ]]; then
        info "session '$SESSION' is running (not attaching)."
        exit 0
    fi
    cd "$PROJECT_DIR"
    exec herdr session attach "$SESSION"
}

main() {
    parse_args "$@"
    check_preconditions

    case "$(session_state)" in
        running)
            info "joining running session '$SESSION'."
            ;;
        stopped)
            info "session '$SESSION' is stopped; starting it again."
            start_server
            ;;
        missing)
            info "creating session '$SESSION' at $PROJECT_DIR."
            start_server
            create_workspace
            ;;
    esac

    attach
}

main "$@"
