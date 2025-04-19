/*
 * Copyright (c) 2025 Charles Benedict, Jr.
 * See LICENSE.md for licensing information.
 * This copyright notice must be retained in its entirety.
 * The LICENSE.md file must be retained and must be included with any distribution of this file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <sqlite3.h>

#define CRYPTOMATH_IMPLEMENTATION
#include "cryptomath.h"

// Test result tracking
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// Helper macro for testing valid decimal strings
#define TEST_VALID_DECIMAL(str) do { \
    total_tests++; \
    if (!crypto_is_valid_decimal(str)) { \
        printf("FAIL: Valid decimal '%s' was rejected\n", str); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

// Helper macro for testing invalid decimal strings
#define TEST_INVALID_DECIMAL(str) do { \
    total_tests++; \
    if (crypto_is_valid_decimal(str)) { \
        printf("FAIL: Invalid decimal '%s' was accepted\n", str); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

void verify_string_parsing(crypto_val_t* val, const char* expected_val_as_str) {
    total_tests++;
    char* str = mpz_get_str(NULL, 10, val->value);
    if (!str) {
        printf("ERROR: Failed to convert result to string\n");
        failed_tests++;
        return;
    }
    if (strcmp(str, expected_val_as_str) != 0) {
        printf("ERROR: Expected %s, got %s\n", expected_val_as_str, str);
        failed_tests++;
        return;
    }
    free(str);
    passed_tests++;
}

void verify_decimal_string(crypto_val_t* val, crypto_denom_t denom, const char* expected_val_as_str) {
    char* str = crypto_to_decimal_str(val, denom);
    total_tests++;
    if (strcmp(str, expected_val_as_str) != 0) {
        printf("ERROR: Expected %s, got %s\n", expected_val_as_str, str);
        failed_tests++;
    } else {
        passed_tests++;
    }
    free(str);
}

void verify_comparison(const crypto_val_t* a, const crypto_val_t* b, const char* a_str, const char* b_str, int expected_result) {
    int cmp_result = crypto_cmp(a, b);
    total_tests++;
    printf("%s %s %s: %s\n",
            a_str,
            cmp_result == 0 ? "==" : (cmp_result > 0 ? ">" : "<"),
            b_str,
            cmp_result == expected_result ? "✓ PASSED" : "✗ FAILED");
    if (cmp_result != expected_result) {
        failed_tests++;
    } else {
        passed_tests++;
    }
}

void verify_zero_comparison(const crypto_val_t* a, int comparison, int expected_result) {
    int cmp_result;
    switch (comparison) {
        case 0:
            cmp_result = crypto_eq_zero(a);
            break;
        case 1:
            cmp_result = crypto_gt_zero(a);
            break;
        case -1:
            cmp_result = crypto_lt_zero(a);
            break;
        default:
            printf("ERROR: Invalid comparison type\n");
            failed_tests++;
            return;
    }
    total_tests++;
    printf("%s %s %s %s: %s\n",
            expected_result == 1 ? "" : "NOT",
            mpz_get_str(NULL, 10, a->value),
            a->crypto_type == CRYPTO_BITCOIN ? "SAT" : "Unknown",
            comparison == 0 ? "== 0" : (comparison > 0 ? "> 0" : "< 0"),
            cmp_result == expected_result ? "✓ PASSED" : "✗ FAILED");
    if (cmp_result != expected_result) {
        failed_tests++;
    } else {
        passed_tests++;
    }
}

// Helper function to execute a SQL query and verify the result
static void verify_sql_result(sqlite3 *db, const char* sql, const char* expected_result, const char* test_name) {
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    rc = sqlite3_step(stmt);
    total_tests++;
    if (rc == SQLITE_ROW) {
        const char* result = (const char*)sqlite3_column_text(stmt, 0);
        if (strcmp(result, expected_result) != 0) {
            printf("FAIL: Expected %s, got %s\n", expected_result, result);
            failed_tests++;
        } else {
            printf("PASS: %s\n", test_name);
            passed_tests++;
        }
    } else {
        printf("FAIL: %s should have returned a row. Error: %s\n", test_name, sqlite3_errmsg(db));
        failed_tests++;
    }
    sqlite3_finalize(stmt);
}

// Helper function to verify SQL error handling
static void verify_sql_parse_error(sqlite3 *db, const char* sql, const char* test_name) {
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    total_tests++;
    if (rc != SQLITE_OK) {
        printf("PASS: %s\n", test_name);
        passed_tests++;
    } else {
        printf("FAIL: Should have rejected invalid input\n");
        failed_tests++;
        sqlite3_finalize(stmt);
    }
}

// Helper function to verify SQL error handling
static void verify_sql_runtime_error(sqlite3 *db, const char* sql, const char* test_name) {
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    total_tests++;
    if (rc != SQLITE_OK) {
        printf("FAIL: should have parsed input %s\n", test_name);
        failed_tests++;
    } else {
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ERROR) {
            printf("FAIL: should have raised runtime error %s\n", test_name);
            failed_tests++;
        } else {
            printf("PASS: %s\n", test_name);
            passed_tests++;
        }
    }
    sqlite3_finalize(stmt);
}


void test_sqlite_extension() {
    printf("\n=== Testing SQLite Extension ===\n");
    
    // Initialize SQLite
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Load the extension
    sqlite3_enable_load_extension(db, 1);
    #if defined(__APPLE__) && defined(__MACH__)
        rc = sqlite3_load_extension(db, "./build/crypto_decimal_extension.dylib", 0, &err_msg);
    #elif defined(__linux__)
        rc = sqlite3_load_extension(db, "./build/crypto_decimal_extension.so", 0, &err_msg);
    #endif
    if (rc != SQLITE_OK) {
        printf("Cannot load extension: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }

    // Test cases for crypto_add
    verify_sql_result(db, 
        "SELECT crypto_add('ETH', 'GWEI', '1.234567891', '0.765432109')",
        "2",
        "Basic addition test");

    verify_sql_result(db,
        "SELECT crypto_add('ETH', 'GWEI', '-1.234567891', '2.234567891')",
        "1",
        "Addition with negative numbers");

    verify_sql_runtime_error(db,
        "SELECT crypto_add('ETH', 'GWEI', 'invalid', '1.0')",
        "Invalid input handling");

    verify_sql_parse_error(db,
        "SELECT crypto_add('ETH', 'GWEI', '1.0')",
        "Wrong number of arguments handling");

    // Test case for crypto_sum
    const char* sql = "CREATE TABLE t(val TEXT);"
                     "INSERT INTO t VALUES('1.234567890000000001'),('0.765432109999999999');"
                     "SELECT crypto_sum('ETH', 'ETH', 'GWEI', val) FROM t;";
    const char *pzTail = sql;

    total_tests++;
    sqlite3_stmt *stmt = NULL;
    while ((rc = sqlite3_prepare_v2(db, pzTail, -1, &stmt, &pzTail)) == SQLITE_OK && stmt) {
        if (sqlite3_stmt_readonly(stmt)) {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                const char* result = (const char*)sqlite3_column_text(stmt, 0);
                if (strcmp(result, "2000000000") != 0) {
                    printf("FAIL: Expected 2000000000, got %s\n", result);
                    failed_tests++;
                } else {
                    printf("PASS: crypto_sum test with high precision values\n");
                    passed_tests++;
                }
                break;
            }
        }
        rc = sqlite3_step(stmt);
    }
    if (rc == SQLITE_ERROR) {
        printf("Error preparing crypto_sum: %s\n", sqlite3_errmsg(db));
        failed_tests++;
        sqlite3_close(db);
        return;
    }
    sqlite3_finalize(stmt);

    // Test cases for crypto_sub
    verify_sql_result(db,
        "SELECT crypto_sub('ETH', 'GWEI', '2', '1')",
        "1",
        "Basic subtraction test");

    verify_sql_result(db,
        "SELECT crypto_sub('ETH', 'GWEI', '1', '2')",
        "-1",
        "Subtraction with negative result");

    verify_sql_result(db,
        "SELECT crypto_sub('ETH', 'GWEI', '1.5', '0.5')",
        "1",
        "Subtraction with decimal values");

    verify_sql_result(db,
        "SELECT crypto_sub('BTC', 'mBTC', '1', '0.5')",
        "0.50000",
        "Subtraction and scale conversion with fractional result");

    // Test cases for crypto_mul
    verify_sql_result(db,
        "SELECT crypto_mul('ETH', 'GWEI', '2', '3')",
        "6",
        "Basic multiplication test");

    verify_sql_result(db,
        "SELECT crypto_mul('ETH', 'GWEI', '1.5', '2')",
        "3",
        "Multiplication with decimal values");

    verify_sql_result(db,
        "SELECT crypto_mul('ETH', 'GWEI', '-2', '3')",
        "-6",
        "Multiplication with negative numbers");

    verify_sql_result(db,
        "SELECT crypto_mul('BTC', 'BTC', '0.5', '0.5')",
        "0.25000000",
        "Multiplication with fractional result");

    verify_sql_runtime_error(db,
        "SELECT crypto_mul('ETH', 'GWEI', 'invalid', '2')",
        "Invalid input handling for multiplication");

    verify_sql_parse_error(db,
        "SELECT crypto_mul('ETH', 'GWEI', '2')",
        "Wrong number of arguments handling for multiplication");

    // Test cases for crypto_div
    verify_sql_result(db,
        "SELECT crypto_div_trunc('ETH', 'GWEI', '6', '2')",
        "3",
        "Basic division test");

    verify_sql_result(db,
        "SELECT crypto_div_trunc('ETH', 'GWEI', '3', '2')",
        "1.500000000",
        "Division with decimal result");

    verify_sql_result(db,
        "SELECT crypto_div_trunc('ETH', 'GWEI', '-6', '2')",
        "-3",
        "Division with negative numbers");

    verify_sql_result(db,
        "SELECT crypto_div_trunc('ETH', 'GWEI', '6', '-2')",
        "-3",
        "Division by negative numbers");

    verify_sql_result(db,
        "SELECT crypto_div_trunc('BTC', 'BTC', '1', '3')",
        "0.33333333",
        "Division with repeating decimal result");

    verify_sql_runtime_error(db,
        "SELECT crypto_div_trunc('ETH', 'GWEI', '6', '0')",
        "Division by zero handling");

    verify_sql_runtime_error(db,
        "SELECT crypto_div_trunc('ETH', 'GWEI', 'invalid', '2')",
        "Invalid input handling for division");

    verify_sql_parse_error(db,
        "SELECT crypto_div_trunc('ETH', 'GWEI', '6')",
        "Wrong number of arguments handling for division");

    sqlite3_close(db);
}

int main() {
    printf("Starting Sqlite Cryptomath Extension Test Suite\n");

    test_sqlite_extension();
    printf("\nTest Suite Summary:\n");
    printf("Total Tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success Rate: %.2f%%\n", (float)passed_tests / total_tests * 100);
    
    return failed_tests > 0 ? 1 : 0;
} 