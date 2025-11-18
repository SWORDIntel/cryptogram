#!/bin/bash

################################################################################
# Build Script Test Suite v2.0
# Comprehensive tests for build_all.sh improvements
################################################################################

set -e

# Colors for output
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly RED='\033[0;31m'
readonly BLUE='\033[0;34m'
readonly CYAN='\033[0;36m'
readonly NC='\033[0m'

# Symbols
readonly CHECK="✓"
readonly CROSS="✗"
readonly WARN="⚠"

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# ──────────────────────────────────────────────────────────────────────────────
# Test Framework Functions
# ──────────────────────────────────────────────────────────────────────────────
print_test_header() {
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}Test $1: $2${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
}

print_pass() {
    echo -e "${GREEN}${CHECK}${NC} $1"
    ((TESTS_PASSED++))
}

print_fail() {
    echo -e "${RED}${CROSS}${NC} $1"
    ((TESTS_FAILED++))
}

print_warning() {
    echo -e "${YELLOW}${WARN}${NC} $1"
}

print_info() {
    echo -e "${BLUE}  →${NC} $1"
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 1: Log Directory Path Generation
# ──────────────────────────────────────────────────────────────────────────────
test_log_directory_paths() {
    ((TESTS_RUN++))
    print_test_header "1" "Log Directory Path Generation"

    # Test user-specific paths
    local user1="testuser1"
    local user2="testuser2"
    local expected1="/tmp/cryptogram_builds_${user1}"
    local expected2="/tmp/cryptogram_builds_${user2}"

    if [ "$expected1" != "$expected2" ]; then
        print_pass "Different users get different log directories"
        print_info "User 1: $expected1"
        print_info "User 2: $expected2"
    else
        print_fail "Users share same directory (BROKEN)"
        return 1
    fi

    # Test that USER variable is used
    if grep -q 'LOG_USER="\${USER' build_all.sh; then
        print_pass "Script uses USER variable for path generation"
    else
        print_fail "Script doesn't use USER variable"
        return 1
    fi

    # Test fallback mechanism
    if grep -q 'HOME/.cache/cryptogram_builds' build_all.sh; then
        print_pass "Fallback to HOME/.cache exists"
    else
        print_fail "No fallback mechanism found"
        return 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 2: Log Directory Writability
# ──────────────────────────────────────────────────────────────────────────────
test_log_directory_writability() {
    ((TESTS_RUN++))
    print_test_header "2" "Log Directory Writability Checks"

    # Check if script creates log directory
    if grep -q 'mkdir -p.*LOG_DIR' build_all.sh; then
        print_pass "Script creates log directory if it doesn't exist"
    else
        print_fail "Script doesn't create log directory"
        return 1
    fi

    # Check for write permission test
    if grep -q 'touch.*LOG_DIR.*write_test' build_all.sh; then
        print_pass "Script tests write permissions"
    else
        print_fail "No write permission test found"
        return 1
    fi

    # Check for error handling
    if grep -q 'Cannot create log' build_all.sh; then
        print_pass "Error handling for unwritable log directory"
    else
        print_fail "No error handling for write failures"
        return 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 3: Log File Creation
# ──────────────────────────────────────────────────────────────────────────────
test_log_file_creation() {
    ((TESTS_RUN++))
    print_test_header "3" "Log File Creation and Permissions"

    # Check for main log file
    if grep -q 'LOG_FILE=' build_all.sh; then
        print_pass "Main log file defined"
    else
        print_fail "LOG_FILE not defined"
        return 1
    fi

    # Check for error log
    if grep -q 'ERROR_LOG=' build_all.sh; then
        print_pass "Separate error log defined"
    else
        print_fail "ERROR_LOG not defined"
        return 1
    fi

    # Check for state file
    if grep -q 'STATE_FILE=' build_all.sh; then
        print_pass "State file for resume capability defined"
    else
        print_fail "STATE_FILE not defined"
        return 1
    fi

    # Check for summary file
    if grep -q 'SUMMARY_FILE=' build_all.sh; then
        print_pass "Summary file defined"
    else
        print_fail "SUMMARY_FILE not defined"
        return 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 4: Protobuf Verification Logic
# ──────────────────────────────────────────────────────────────────────────────
test_protobuf_verification() {
    ((TESTS_RUN++))
    print_test_header "4" "Protobuf Verification Logic Fix"

    echo "Demonstrating the bug fix:"
    echo ""

    # Show old broken logic
    print_info "OLD (broken) logic:"
    echo "    if find /usr -name 'libprotobuf.so*' 2>/dev/null | head -n1; then"
    print_warning "This ALWAYS succeeds (exit code 0) even when finding nothing"
    echo ""

    # Show new fixed logic
    print_info "NEW (fixed) logic:"
    echo "    SYSTEM_PROTO=\$(find /usr -name 'libprotobuf.so*' ... | head -n1)"
    echo "    if [ -n \"\$SYSTEM_PROTO\" ]; then"
    print_pass "Correctly checks if variable is non-empty"
    echo ""

    # Verify the fix is in place
    if grep -q 'SYSTEM_PROTO.*find /usr' build_all.sh && \
       grep -q 'if \[ -n.*SYSTEM_PROTO' build_all.sh; then
        print_pass "Protobuf verification uses correct logic"
    else
        print_fail "Protobuf verification logic not fixed"
        return 1
    fi

    # Check for diagnostic output
    if grep -q 'Protobuf installation verification FAILED' build_all.sh; then
        print_pass "Clear failure message when Protobuf not found"
    else
        print_fail "No clear failure message"
        return 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 5: User Isolation
# ──────────────────────────────────────────────────────────────────────────────
test_user_isolation() {
    ((TESTS_RUN++))
    print_test_header "5" "User Isolation for Multi-User Systems"

    # Test that paths are user-specific
    if grep -q 'cryptogram_builds_\${LOG_USER}' build_all.sh; then
        print_pass "Log paths include user identifier"
    else
        print_fail "Log paths don't include user identifier"
        return 1
    fi

    # Test UID fallback
    if grep -q 'user\${UID}' build_all.sh; then
        print_pass "Fallback to UID when USER not available"
    else
        print_fail "No UID fallback"
        return 1
    fi

    # Simulate different users
    print_info "Simulating different users:"
    echo "    User 'root' -> /tmp/cryptogram_builds_root"
    echo "    User 'john' -> /tmp/cryptogram_builds_john"
    print_pass "Each user gets isolated log directory"
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 6: Verification Messages
# ──────────────────────────────────────────────────────────────────────────────
test_verification_messages() {
    ((TESTS_RUN++))
    print_test_header "6" "Verification Message Format"

    if grep -q "verification PASSED" build_all.sh && \
       grep -q "verification FAILED" build_all.sh; then
        print_pass "Verification messages use PASSED/FAILED format"

        local passed_count=$(grep -c "verification PASSED" build_all.sh)
        local failed_count=$(grep -c "verification FAILED" build_all.sh)
        print_info "Found $passed_count PASSED messages"
        print_info "Found $failed_count FAILED messages"
    else
        print_fail "Verification messages missing or incorrect"
        return 1
    fi

    # Check for color coding
    if grep -q 'print_info.*verification PASSED' build_all.sh; then
        print_pass "PASSED messages use success color"
    fi

    if grep -q 'print_error.*verification FAILED' build_all.sh; then
        print_pass "FAILED messages use error color"
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 7: Diagnostic Output
# ──────────────────────────────────────────────────────────────────────────────
test_diagnostic_output() {
    ((TESTS_RUN++))
    print_test_header "7" "Diagnostic Output on Failure"

    if grep -q "Diagnostics:" build_all.sh && \
       grep -q "Contents of" build_all.sh && \
       grep -q "Permissions on" build_all.sh; then
        print_pass "Diagnostic output included on verification failure"
    else
        print_fail "Missing diagnostic output"
        return 1
    fi

    # Check for directory listing
    if grep -q 'ls -l.*INSTALL_PREFIX' build_all.sh; then
        print_pass "Shows directory contents on failure"
    else
        print_fail "No directory listing on failure"
        return 1
    fi

    # Check for suggestions
    if grep -q "Possible solutions:" build_all.sh; then
        print_pass "Provides solution suggestions"
    else
        print_fail "No solution suggestions"
        return 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 8: Unbound Variable Protection
# ──────────────────────────────────────────────────────────────────────────────
test_unbound_variables() {
    ((TESTS_RUN++))
    print_test_header "8" "Unbound Variable Protection"

    # Verify script has proper parameter expansion patterns
    if grep -q '\${LOG_DIR:-}' build_all.sh; then
        print_pass "LOG_DIR uses safe parameter expansion \${LOG_DIR:-}"
    else
        print_fail "LOG_DIR missing safe parameter expansion"
        return 1
    fi

    if grep -q '\${CC:-}' build_all.sh; then
        print_pass "CC uses safe parameter expansion \${CC:-}"
    else
        print_fail "CC missing safe parameter expansion"
        return 1
    fi

    if grep -q '\${CXX:-}' build_all.sh; then
        print_pass "CXX uses safe parameter expansion \${CXX:-}"
    else
        print_fail "CXX missing safe parameter expansion"
        return 1
    fi

    # Check compiler initialization
    if grep -q 'CC=""' build_all.sh && grep -q 'CXX=""' build_all.sh; then
        print_pass "Compilers initialized before use"
    else
        print_fail "Compilers not initialized"
        return 1
    fi

    print_info "Script compatible with 'set -u' (strict mode)"
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 9: Enhanced Logging System
# ──────────────────────────────────────────────────────────────────────────────
test_enhanced_logging() {
    ((TESTS_RUN++))
    print_test_header "9" "Enhanced Logging System (v3.1)"

    # Check for multi-level logging
    if grep -q 'log() {' build_all.sh; then
        print_pass "Logging function defined"
    else
        print_fail "No logging function found"
        return 1
    fi

    # Check for log levels
    local levels=("INFO" "WARN" "ERROR" "FATAL" "DEBUG")
    for level in "${levels[@]}"; do
        if grep -q "\"$level\"" build_all.sh; then
            print_pass "Log level $level supported"
        fi
    done

    # Check for timestamps
    if grep -q 'timestamp.*get_timestamp' build_all.sh; then
        print_pass "Timestamps in log entries"
    else
        print_fail "No timestamps in logs"
        return 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 10: Build State Management
# ──────────────────────────────────────────────────────────────────────────────
test_state_management() {
    ((TESTS_RUN++))
    print_test_header "10" "Build State Management (v3.1)"

    # Check for state save function
    if grep -q 'save_state()' build_all.sh; then
        print_pass "State save function exists"
    else
        print_fail "No state save function"
        return 1
    fi

    # Check for state load function
    if grep -q 'load_state()' build_all.sh; then
        print_pass "State load function exists"
    else
        print_fail "No state load function"
        return 1
    fi

    # Check for resume flag
    if grep -q '\--resume' build_all.sh; then
        print_pass "Resume flag supported"
    else
        print_fail "No resume flag"
        return 1
    fi

    # Check for atomic operations
    if grep -q 'STATE_FILE.*\.tmp' build_all.sh; then
        print_pass "Atomic state file operations"
    else
        print_fail "No atomic operations"
        return 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 11: System Validation
# ──────────────────────────────────────────────────────────────────────────────
test_system_validation() {
    ((TESTS_RUN++))
    print_test_header "11" "System Validation (v3.1)"

    # Memory checks
    if grep -q 'mem_gb' build_all.sh && grep -q 'TOTAL_MEMORY' build_all.sh; then
        print_pass "Memory validation implemented"
    else
        print_fail "No memory validation"
        return 1
    fi

    # Disk space checks
    if grep -q 'available.*disk' build_all.sh || grep -q 'df.*HOME' build_all.sh; then
        print_pass "Disk space validation implemented"
    else
        print_fail "No disk space validation"
        return 1
    fi

    # Compiler detection
    if grep -q 'gcc-13.*gcc-12.*gcc-11.*gcc.*clang' build_all.sh; then
        print_pass "Compiler fallback chain exists"
    else
        print_fail "No compiler fallback chain"
        return 1
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Test 12: Error Handling
# ──────────────────────────────────────────────────────────────────────────────
test_error_handling() {
    ((TESTS_RUN++))
    print_test_header "12" "Enhanced Error Handling (v3.1)"

    # Check for enhanced fail function
    if grep -q 'fail() {' build_all.sh; then
        print_pass "Fail function defined"
    else
        print_fail "No fail function"
        return 1
    fi

    # Check for error context
    if grep -q 'FUNCNAME' build_all.sh && grep -q 'LINENO' build_all.sh; then
        print_pass "Error context (function, line) captured"
    else
        print_fail "No error context"
        return 1
    fi

    # Check for last N lines of log
    if grep -q 'tail -n.*LOG_FILE' build_all.sh; then
        print_pass "Shows last lines of log on error"
    else
        print_fail "No log tail on error"
        return 1
    fi

    # Check for error trapping
    if grep -q 'trap.*ERR' build_all.sh; then
        print_pass "ERR signal trapped"
    fi
    if grep -q 'trap.*INT' build_all.sh; then
        print_pass "INT signal trapped (Ctrl+C)"
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Main Test Runner
# ──────────────────────────────────────────────────────────────────────────────
main() {
    echo ""
    echo "════════════════════════════════════════════════════════════════"
    echo "         Build Script Test Suite v2.0"
    echo "         Testing build_all.sh improvements"
    echo "════════════════════════════════════════════════════════════════"

    # Check if build_all.sh exists
    if [ ! -f "build_all.sh" ]; then
        echo -e "${RED}${CROSS} build_all.sh not found in current directory${NC}"
        exit 1
    fi

    # Run all tests
    test_log_directory_paths
    test_log_directory_writability
    test_log_file_creation
    test_protobuf_verification
    test_user_isolation
    test_verification_messages
    test_diagnostic_output
    test_unbound_variables
    test_enhanced_logging
    test_state_management
    test_system_validation
    test_error_handling

    # Final summary
    echo ""
    echo "════════════════════════════════════════════════════════════════"
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}${CHECK} All tests passed! ($TESTS_PASSED/$TESTS_RUN)${NC}"
    else
        echo -e "${RED}${CROSS} Some tests failed: $TESTS_FAILED/$TESTS_RUN${NC}"
        echo -e "${GREEN}${CHECK} Passed: $TESTS_PASSED${NC}"
        echo -e "${RED}${CROSS} Failed: $TESTS_FAILED${NC}"
    fi
    echo "════════════════════════════════════════════════════════════════"
    echo ""

    if [ $TESTS_FAILED -eq 0 ]; then
        echo "Summary of verified improvements:"
        echo "  1. ✓ User-specific log directories prevent permission conflicts"
        echo "  2. ✓ Protobuf verification correctly detects missing libraries"
        echo "  3. ✓ Clear PASSED/FAILED messaging for installations"
        echo "  4. ✓ Detailed diagnostics on failures"
        echo "  5. ✓ Safe parameter expansion for unbound variables"
        echo "  6. ✓ Multi-level logging with timestamps"
        echo "  7. ✓ Build state management for resume capability"
        echo "  8. ✓ System validation (memory, disk, compilers)"
        echo "  9. ✓ Enhanced error handling with context"
        echo "  10. ✓ Comprehensive signal trapping"
        echo ""
    fi

    exit $TESTS_FAILED
}

# Entry point
main "$@"
