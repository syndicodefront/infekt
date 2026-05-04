#!/usr/bin/env bash

set -euo pipefail

TMP_ROOT="${TMPDIR:-/tmp}"
CPP_BIN=""
RUST_BIN=""
OUT_DIR=""
MANIFEST_INITIALIZED=0
FAIL_FAST=0
ALLOW_BOTH_FAIL=0
LIMIT=""

declare -a INPUTS=()

scanned=0
matched=0
different=0
failed=0
shared_failed=0
kept=0

usage() {
    cat <<'USAGE'
Usage:
  tools/compare-cli-utf8.sh \
    --cpp-bin /path/to/cpp/infekt-cli \
    --rust-bin /path/to/rust/infekt-cli \
    [--out-dir /tmp/infekt-cli-compare] \
    [--fail-fast] \
    [--limit N] \
    FILE_OR_DIR...

Compare UTF-8 output from the C++ and Rust iNFekt CLIs over .nfo/.diz files.

Options:
  --cpp-bin PATH    Path to the known-correct C++ infekt-cli binary.
  --rust-bin PATH   Path to the Rust infekt-cli binary.
  --out-dir PATH    Directory for mismatch artifacts. Defaults to a fresh
                    temp directory under ${TMPDIR:-/tmp}, created only if
                    artifacts need to be kept.
  --fail-fast       Stop after the first mismatch or CLI failure.
  --allow-both-fail Treat files as compatible when both CLIs return non-zero.
  --limit N         Compare at most N discovered files.
  -h, --help        Show this help.
USAGE
}

usage_error() {
    echo "ERROR: $*" >&2
    echo "Try --help." >&2
    exit 2
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --cpp-bin)
            [ "$#" -ge 2 ] || usage_error "--cpp-bin requires a path"
            CPP_BIN=$2
            shift 2
            ;;
        --rust-bin)
            [ "$#" -ge 2 ] || usage_error "--rust-bin requires a path"
            RUST_BIN=$2
            shift 2
            ;;
        --out-dir)
            [ "$#" -ge 2 ] || usage_error "--out-dir requires a path"
            OUT_DIR=$2
            shift 2
            ;;
        --fail-fast)
            FAIL_FAST=1
            shift
            ;;
        --allow-both-fail)
            ALLOW_BOTH_FAIL=1
            shift
            ;;
        --limit)
            [ "$#" -ge 2 ] || usage_error "--limit requires a number"
            LIMIT=$2
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            while [ "$#" -gt 0 ]; do
                INPUTS+=("$1")
                shift
            done
            ;;
        -*)
            usage_error "unknown option: $1"
            ;;
        *)
            INPUTS+=("$1")
            shift
            ;;
    esac
done

[ -n "$CPP_BIN" ] || usage_error "--cpp-bin is required"
[ -n "$RUST_BIN" ] || usage_error "--rust-bin is required"
[ "${#INPUTS[@]}" -gt 0 ] || usage_error "at least one FILE_OR_DIR is required"
[ -x "$CPP_BIN" ] || usage_error "--cpp-bin is not executable: $CPP_BIN"
[ -x "$RUST_BIN" ] || usage_error "--rust-bin is not executable: $RUST_BIN"

if [ -n "$LIMIT" ]; then
    case "$LIMIT" in
        ''|*[!0-9]*)
            usage_error "--limit must be a positive integer"
            ;;
        0)
            usage_error "--limit must be greater than zero"
            ;;
    esac
fi

for input in "${INPUTS[@]}"; do
    [ -e "$input" ] || usage_error "input does not exist: $input"
done

cleanup_tmp_workspace() {
    local dir=$1
    if [ -n "$dir" ] && [ -d "$dir" ]; then
        rm -rf "$dir"
    fi
}

ensure_out_dir() {
    if [ -z "$OUT_DIR" ]; then
        OUT_DIR=$(mktemp -d "${TMP_ROOT%/}/infekt-cli-compare.XXXXXX")
    else
        mkdir -p "$OUT_DIR"
    fi

    mkdir -p "$OUT_DIR/cpp" "$OUT_DIR/rust" "$OUT_DIR/diffs"

    if [ "$MANIFEST_INITIALIZED" -eq 0 ]; then
        printf 'status\tpath\tcpp_exit\trust_exit\tcpp_output\trust_output\tdiff\n' > "$OUT_DIR/manifest.tsv"
        MANIFEST_INITIALIZED=1
    fi
}

escape_tsv() {
    local value=$1
    value=${value//$'\t'/\\t}
    value=${value//$'\n'/\\n}
    value=${value//$'\r'/\\r}
    printf '%s' "$value"
}

write_manifest_row() {
    local status=$1
    local path=$2
    local cpp_exit=$3
    local rust_exit=$4
    local cpp_output=$5
    local rust_output=$6
    local diff_file=$7

    ensure_out_dir
    {
        escape_tsv "$status"; printf '\t'
        escape_tsv "$path"; printf '\t'
        escape_tsv "$cpp_exit"; printf '\t'
        escape_tsv "$rust_exit"; printf '\t'
        escape_tsv "$cpp_output"; printf '\t'
        escape_tsv "$rust_output"; printf '\t'
        escape_tsv "$diff_file"; printf '\n'
    } >> "$OUT_DIR/manifest.tsv"
}

make_id() {
    local input=$1
    local base
    local hash

    base=$(basename "$input")
    hash=$(printf '%s' "$input" | shasum | awk '{print substr($1, 1, 12)}')

    printf '%s-%s' "$hash" "$base"
}

copy_if_exists() {
    local source=$1
    local dest=$2

    if [ -f "$source" ]; then
        cp "$source" "$dest"
        printf '%s' "$dest"
    fi
}

process_file() {
    local input=$1
    local work_dir=""
    local cpp_out
    local rust_out
    local cpp_exit=0
    local rust_exit=0
    local status=""
    local id
    local kept_cpp=""
    local kept_rust=""
    local diff_file=""

    scanned=$((scanned + 1))

    work_dir=$(mktemp -d "${TMP_ROOT%/}/infekt-cli-file.XXXXXX")
    cpp_out="$work_dir/cpp.nfo"
    rust_out="$work_dir/rust.nfo"

    if "$CPP_BIN" --utf-8 --out-file "$cpp_out" "$input" >/dev/null 2>&1; then
        cpp_exit=0
    else
        cpp_exit=$?
    fi

    if "$RUST_BIN" --utf-8 --out-file "$rust_out" "$input" >/dev/null 2>&1; then
        rust_exit=0
    else
        rust_exit=$?
    fi

    if [ "$cpp_exit" -eq 0 ] && [ "$rust_exit" -eq 0 ]; then
        if [ ! -f "$cpp_out" ] || [ ! -f "$rust_out" ]; then
            status="missing_output"
        elif cmp -s "$cpp_out" "$rust_out"; then
            matched=$((matched + 1))
            cleanup_tmp_workspace "$work_dir"
            return 0
        else
            status="diff"
        fi
    elif [ "$cpp_exit" -ne 0 ] && [ "$rust_exit" -ne 0 ]; then
        status="both_fail"
    elif [ "$cpp_exit" -ne 0 ]; then
        status="cpp_fail"
    else
        status="rust_fail"
    fi

    id=$(make_id "$input")
    ensure_out_dir

    kept_cpp=$(copy_if_exists "$cpp_out" "$OUT_DIR/cpp/$id")
    kept_rust=$(copy_if_exists "$rust_out" "$OUT_DIR/rust/$id")

    if [ -n "$kept_cpp" ] && [ -n "$kept_rust" ]; then
        diff_file="$OUT_DIR/diffs/$id.diff"
        if ! diff -u "$kept_cpp" "$kept_rust" > "$diff_file"; then
            :
        fi
    fi

    write_manifest_row "$status" "$input" "$cpp_exit" "$rust_exit" "$kept_cpp" "$kept_rust" "$diff_file"

    if [ "$status" = "both_fail" ] && [ "$ALLOW_BOTH_FAIL" -eq 1 ]; then
        shared_failed=$((shared_failed + 1))
    elif [ "$status" = "diff" ]; then
        different=$((different + 1))
    else
        failed=$((failed + 1))
    fi

    kept=$((kept + 1))
    cleanup_tmp_workspace "$work_dir"

    if [ "$FAIL_FAST" -eq 1 ] && { [ "$status" != "both_fail" ] || [ "$ALLOW_BOTH_FAIL" -eq 0 ]; }; then
        return 1
    fi

    return 0
}

limit_reached() {
    [ -n "$LIMIT" ] && [ "$scanned" -ge "$LIMIT" ]
}

should_process_path() {
    local path=$1

    case "$path" in
        *.nfo|*.NFO|*.diz|*.DIZ)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

stop=0

for input in "${INPUTS[@]}"; do
    if limit_reached || [ "$stop" -eq 1 ]; then
        break
    fi

    if [ -d "$input" ]; then
        while IFS= read -r -d '' file; do
            if limit_reached || [ "$stop" -eq 1 ]; then
                break
            fi

            if ! process_file "$file"; then
                stop=1
            fi
        done < <(find "$input" -type f \( -name '*.nfo' -o -name '*.NFO' -o -name '*.diz' -o -name '*.DIZ' \) -print0)
    elif [ -f "$input" ]; then
        if should_process_path "$input"; then
            if ! process_file "$input"; then
                stop=1
            fi
        fi
    else
        usage_error "input is neither a file nor directory: $input"
    fi
done

if [ "$scanned" -eq 0 ]; then
    echo "ERROR: no .nfo or .diz files found in the supplied inputs" >&2
    exit 2
fi

echo "Compared $scanned file(s): $matched matched, $different differed, $failed failed, $shared_failed shared-failed."

if [ "$kept" -gt 0 ]; then
    echo "Artifacts: $OUT_DIR"
fi

if [ "$different" -gt 0 ] || [ "$failed" -gt 0 ]; then
    exit 1
fi

exit 0
