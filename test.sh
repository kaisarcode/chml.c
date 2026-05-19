#!/bin/sh
# test.sh
# Summary: Validation suite for chml functionality.
# Author:  KaisarCode
# Website: https://kaisarcode.com
# License: https://www.gnu.org/licenses/gpl-3.0.html

# Prints one failure line.
# @param $1 Failure message.
# @return 1 on failure.
kc_test_fail() {
    printf '\033[31m[FAIL]\033[0m %s\n' "$1"
    return 1
}

# Prints one success line.
# @param $1 Success message.
# @return 0 on success.
kc_test_pass() {
    printf '\033[32m[PASS]\033[0m %s\n' "$1"
}

# Detects the artifact architecture for the current machine.
# @return Architecture name on stdout.
kc_test_arch() {
    case "$(uname -m)" in
        x86_64 | amd64)
            printf '%s\n' "x86_64"
            ;;
        aarch64 | arm64)
            printf '%s\n' "aarch64"
            ;;
        armv7l | armv7)
            printf '%s\n' "armv7"
            ;;
        i386 | i486 | i586 | i686)
            printf '%s\n' "i686"
            ;;
        ppc64le | powerpc64le)
            printf '%s\n' "powerpc64le"
            ;;
        *)
            uname -m
            ;;
    esac
}

# Detects the artifact platform for the current machine.
# @return Platform name on stdout.
kc_test_platform() {
    case "$(uname -s)" in
        Linux)
            printf '%s\n' "linux"
            ;;
        *)
            uname -s | tr '[:upper:]' '[:lower:]'
            ;;
    esac
}

# Returns the CLI path for the current architecture and platform.
# @return CLI path on stdout.
kc_test_binary_path() {
    printf './bin/%s/%s/chml\n' "$(kc_test_arch)" "$(kc_test_platform)"
}

# Verifies the binary exists and is executable.
# @return 0 on success, 1 on failure.
kc_test_check_binary() {
    if [ ! -x "$BIN" ]; then
        kc_test_fail "binary not found: $BIN"
        return 1
    fi
    return 0
}

# Tests that an unknown flag exits with non-zero.
# @return 0 on success, 1 on failure.
kc_test_unknown_flag() {
    if "$BIN" --unknown 2>/dev/null; then
        kc_test_fail "unknown flag should fail"
        return 1
    fi
    kc_test_pass "unknown flag"
}

# Tests that empty stdin returns 0 with no output.
# @return 0 on success, 1 on failure.
kc_test_empty_stdin() {
    output=$("$BIN" < /dev/null 2>/dev/null)
    rc=$?
    if [ "$rc" -ne 0 ]; then
        kc_test_fail "empty stdin should exit 0"
        return 1
    fi
    if [ -n "$output" ]; then
        kc_test_fail "empty stdin should produce no output"
        return 1
    fi
    kc_test_pass "empty stdin"
}

# Tests that piping "Hello" produces correct ChatML with default role (user).
# @return 0 on success, 1 on failure.
kc_test_default_role() {
    output=$(printf 'Hello' | "$BIN" 2>/dev/null)
    expected='<|im_start|>user
Hello
<|im_end|>'
    if [ "$output" != "$expected" ]; then
        kc_test_fail "default role output mismatch"
        return 1
    fi
    kc_test_pass "default role"
}

# Tests that --role system produces correct ChatML.
# @return 0 on success, 1 on failure.
kc_test_role_system() {
    output=$(printf 'You are a bot.' | "$BIN" --role system 2>/dev/null)
    expected='<|im_start|>system
You are a bot.
<|im_end|>'
    if [ "$output" != "$expected" ]; then
        kc_test_fail "role system output mismatch"
        return 1
    fi
    kc_test_pass "role system"
}

# Tests that --role assistant produces correct ChatML.
# @return 0 on success, 1 on failure.
kc_test_role_assistant() {
    output=$(printf 'I am a bot.' | "$BIN" -r assistant 2>/dev/null)
    expected='<|im_start|>assistant
I am a bot.
<|im_end|>'
    if [ "$output" != "$expected" ]; then
        kc_test_fail "role assistant output mismatch"
        return 1
    fi
    kc_test_pass "role assistant"
}

# Tests that an invalid role value fails.
# @return 0 on success, 1 on failure.
kc_test_invalid_role() {
    if "$BIN" --role invalid 2>/dev/null; then
        kc_test_fail "invalid role should fail"
        return 1
    fi
    kc_test_pass "invalid role"
}

# Tests that --role without a value fails.
# @return 0 on success, 1 on failure.
kc_test_missing_role_value() {
    if "$BIN" --role 2>/dev/null; then
        kc_test_fail "missing role value should fail"
        return 1
    fi
    kc_test_pass "missing role value"
}

# Runs the full validation suite.
# @return 0 on success, 1 on failure.
kc_test_main() {
    failed=0

    BIN=$(kc_test_binary_path)

    kc_test_check_binary || exit 1

    kc_test_unknown_flag      || failed=$((failed + 1))
    kc_test_empty_stdin       || failed=$((failed + 1))
    kc_test_default_role      || failed=$((failed + 1))
    kc_test_role_system       || failed=$((failed + 1))
    kc_test_role_assistant    || failed=$((failed + 1))
    kc_test_invalid_role      || failed=$((failed + 1))
    kc_test_missing_role_value || failed=$((failed + 1))

    return $failed
}

kc_test_main
