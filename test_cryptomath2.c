#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <sqlite3.h>

#define CRYPTOMATH2_IMPLEMENTATION
#include "cryptomath2.h"

// Test result tracking
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// Global jump buffer for longjmp
static jmp_buf jumpBuffer;

// Custom SIGABRT handler that jumps back to our test
static void sigabrt_handler(int signo) {
    (void)signo;  // unused parameter
    longjmp(jumpBuffer, 1);
}

// Custom SIGFPE handler that jumps back to our test
static void sigfpe_handler(int signo) {
    (void)signo;  // unused parameter
    longjmp(jumpBuffer, 1);
}

// The macro sets up the signal handler, calls setjmp,
// executes your statement, and checks whether we jumped
// due to an assertion (SIGABRT).
#define SHOULD_ASSERT(STMT)                                          \
    do {                                                             \
        total_tests++;                                               \
        /* Install custom signal handler for SIGABRT */              \
        signal(SIGABRT, sigabrt_handler);                            \
                                                                     \
        /* setjmp returns 0 initially, and 1 if we longjmp from the handler */ \
        if (setjmp(jumpBuffer) == 0) {                               \
            /* Execute the statement (which should trigger assert) */\
            (void)(STMT);                                            \
            /* If we get here, no abort/assert occurred */           \
            printf("FAIL: Assertion not triggered by '%s'\n", #STMT);\
            failed_tests++;                                          \
        } else {                                                     \
            /* We arrived via longjmp => an assert called abort */   \
            printf("PASS: Assertion triggered by '%s'\n", #STMT);    \
            passed_tests++;                                          \
        }                                                            \
                                                                     \
        /* Optionally restore default SIGABRT handler for safety */  \
        signal(SIGABRT, SIG_DFL);                                    \
    } while (0)

// The macro sets up the signal handler, calls setjmp,
// executes your statement, and checks whether we jumped
// due to an exception (SIGFPE).
#define SHOULD_FPE(STMT)                                             \
    do {                                                             \
        total_tests++;                                               \
        /* Install custom signal handler for SIGFPE */               \
        signal(SIGFPE, sigfpe_handler);                              \
                                                                     \
        /* setjmp returns 0 initially, and 1 if we longjmp from the handler */ \
        if (setjmp(jumpBuffer) == 0) {                               \
            /* Execute the statement (which should trigger exception) */\
            (void)(STMT);                                            \
            /* If we get here, no abort/assert occurred */           \
            printf("FAIL: Exception not triggered by '%s'\n", #STMT);\
            failed_tests++;                                          \
        } else {                                                     \
            /* We arrived via longjmp => an exception called abort */   \
            printf("PASS: Exception triggered by '%s'\n", #STMT);    \
            passed_tests++;                                          \
        }                                                            \
                                                                     \
        /* Optionally restore default SIGFPE handler for safety */   \
        signal(SIGFPE, SIG_DFL);                                     \
    } while (0)

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

void test_arithmetic_operations() {
    printf("\n=== Testing Arithmetic Operations ===\n");
    
    // Test 1: Adding different units (1 BTC + 100 mBTC = 1.1 BTC)
    crypto_val_t btc, mbtc, result;
    crypto_init(&btc, CRYPTO_BITCOIN);
    crypto_init(&mbtc, CRYPTO_BITCOIN);
    crypto_init(&result, CRYPTO_BITCOIN);
    
    crypto_set_from_decimal(&btc, BTC_DENOM_BITCOIN, "1.1");    // 1.1 BTC
    crypto_set_from_decimal(&mbtc, BTC_DENOM_MILLIBIT, "100");  // 100 mBTC = 0.1 BTC
    
    crypto_add(&result, &btc, &mbtc);
    verify_decimal_string(&result, BTC_DENOM_BITCOIN, "1.20000000");

    // Test 2: Subtracting different units (1 BTC - 50000000 SAT = 0.5 BTC)
    crypto_val_t sat;
    crypto_init(&sat, CRYPTO_BITCOIN);
    crypto_set_from_decimal(&sat, BTC_DENOM_SATOSHI, "50000000");  // 0.5 BTC in satoshis
    
    crypto_sub(&result, &btc, &sat);
    verify_decimal_string(&result, BTC_DENOM_BITCOIN, "0.60000000");

    // Test 3: Subtracting a negative value (1.1 BTC - (-0.5 BTC) = 1.6 BTC)
    crypto_set_from_decimal(&sat, BTC_DENOM_SATOSHI, "-50000000");  // -0.5 BTC in satoshis
    crypto_sub(&result, &btc, &sat);
    verify_decimal_string(&result, BTC_DENOM_BITCOIN, "1.60000000");

    // Cleanup
    crypto_clear(&btc);
    crypto_clear(&mbtc);
    crypto_clear(&sat);
    crypto_clear(&result);
}

void test_comparison_operations() {
    printf("\n=== Testing Comparison Operations ===\n");

    crypto_val_t a, b;
    crypto_init(&a, CRYPTO_BITCOIN);
    crypto_init(&b, CRYPTO_BITCOIN);

    // Test 1: Comparison of different units (50000000 SAT == 500000 μBTC)
    crypto_set_from_decimal(&a, BTC_DENOM_SATOSHI, "50000000");  // 0.5 BTC in satoshis
    crypto_set_from_decimal(&b, BTC_DENOM_MICROBIT, "500000");    // 0.5 BTC in μBTC

    verify_comparison(&a, &b, "50000000 SAT", "500000 μBTC", 0);

    // Test 2: Comparison of different units (50000000 SAT > 500000 μBTC)
    crypto_set_from_decimal(&a, BTC_DENOM_SATOSHI, "50000001");  // 0.5 BTC in satoshis
    crypto_set_from_decimal(&b, BTC_DENOM_MICROBIT, "500000");    // 0.5 BTC in μBTC

    verify_comparison(&a, &b, "50000001 SAT", "500000 μBTC", 1);

    // Test 3: Comparison of different units (49999999 SAT < 500000 μBTC)
    crypto_set_from_decimal(&a, BTC_DENOM_SATOSHI, "49999999");  // 0.49999999 BTC in satoshis
    crypto_set_from_decimal(&b, BTC_DENOM_MICROBIT, "500000");    // 0.5 BTC in μBTC

    verify_comparison(&a, &b, "49999999 SAT", "500000 μBTC", -1);

    // Test 5: Test comparison of mixed sign and denominations
    crypto_set_from_decimal(&a, BTC_DENOM_SATOSHI, "1");  // 1 BTC in satoshis
    crypto_set_from_decimal(&b, BTC_DENOM_MICROBIT, "-1");    // -1 BTC in μBTC

    verify_comparison(&a, &b, "1 SAT", "-1 μBTC", 1);

    // Cleanup
    crypto_clear(&a);
    crypto_clear(&b);
}

void test_zero_comparison() {
    printf("\n=== Testing Zero Comparison ===\n");

    crypto_val_t a;
    crypto_init(&a, CRYPTO_BITCOIN);

    crypto_set_from_decimal(&a, BTC_DENOM_SATOSHI, "0");
    verify_zero_comparison(&a, 0, 1);

    crypto_set_from_decimal(&a, BTC_DENOM_MICROBIT, "0");
    verify_zero_comparison(&a, 0, 1);

    crypto_set_from_decimal(&a, BTC_DENOM_MICROBIT, "-0");
    verify_zero_comparison(&a, 0, 1);

    crypto_set_from_decimal(&a, BTC_DENOM_MICROBIT, "-1");
    verify_zero_comparison(&a, 0, 0);

    crypto_set_from_decimal(&a, BTC_DENOM_MICROBIT, "1");
    verify_zero_comparison(&a, 1, 1);

    crypto_set_from_decimal(&a, BTC_DENOM_MICROBIT, "-1");
    verify_zero_comparison(&a, -1, 1);

}

void test_decimal_string_parsing() {
    printf("\n=== Testing Decimal String Parsing ===\n");
    
    // Test 1: Basic positive decimal
    crypto_val_t amount;
    crypto_init(&amount, CRYPTO_BITCOIN);
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "1.23456789");
    verify_string_parsing(&amount, "123456789");
    
    // Test 2: Negative decimal
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "-0.00000001");
    verify_string_parsing(&amount, "-1");
    
    // Test 3: Whole number
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "42");
    verify_string_parsing(&amount, "4200000000");
        
    // Test 4: Small value in satoshis
    crypto_set_from_decimal(&amount, BTC_DENOM_SATOSHI, "12345");
    verify_string_parsing(&amount, "12345");
    
    // Test 5: Millibitcoin with decimal
    crypto_set_from_decimal(&amount, BTC_DENOM_MILLIBIT, "1.23456");
    verify_string_parsing(&amount, "123456");
    
    // Test 6: Microbitcoin with decimal
    crypto_set_from_decimal(&amount, BTC_DENOM_MICROBIT, "123.45");
    verify_string_parsing(&amount, "12345");
    
    // Test 7: Zero with decimal
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "0.00000000");
    verify_string_parsing(&amount, "0");
    
    // Test 8: Leading zeros
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "0001.23456789");
    verify_string_parsing(&amount, "123456789");
    
    // Test 9: Negative values in different units
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "-1.23456789");
    verify_string_parsing(&amount, "-123456789");
    
    crypto_set_from_decimal(&amount, BTC_DENOM_SATOSHI, "-12345");
    verify_string_parsing(&amount, "-12345");
    
    crypto_set_from_decimal(&amount, BTC_DENOM_MILLIBIT, "-1.2345");
    verify_string_parsing(&amount, "-123450");
    
    crypto_set_from_decimal(&amount, BTC_DENOM_MICROBIT, "-123.45");
    verify_string_parsing(&amount, "-12345");
    
    // Test 10: Leading spaces
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "  1.23456789");
    verify_string_parsing(&amount, "123456789");

    // Test 11: Trailing spaces
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "1.23456789  ");
    verify_string_parsing(&amount, "123456789");
    
    // Test 12: Test with a large number of decimal places (should be truncated)
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "1.23456789012345678901234567890123456789");
    verify_string_parsing(&amount, "123456789");

    // Test 13: Test an invalid decimal string (maybe these should assert?)
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "a.23456789012345678901234567890123456789a");
    verify_string_parsing(&amount, "23456789");

    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "1.a3456789012345678901234567890123456789");
    verify_string_parsing(&amount, "100000000");

    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "-a.a");
    verify_string_parsing(&amount, "0");

    // Test 15: Test assertions
    SHOULD_ASSERT(crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, NULL));
    SHOULD_ASSERT(crypto_set_from_decimal(NULL, BTC_DENOM_BITCOIN, "1"));
    SHOULD_ASSERT(crypto_set_from_decimal(&amount, 42, "1"));
    SHOULD_ASSERT(crypto_set_from_decimal(&amount, ETH_DENOM_ETHER, "1"));

    // Cleanup
    crypto_clear(&amount);
}

void test_decimal_string_conversion() {
    printf("\n=== Testing Decimal String Conversion ===\n");

    crypto_val_t amount;
    crypto_init(&amount, CRYPTO_BITCOIN);
    // Test 1: Basic positive decimal
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "1.23456789");
    verify_decimal_string(&amount, BTC_DENOM_BITCOIN, "1.23456789");

    // Test 2: Negative decimal
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "-0.00000001");
    verify_decimal_string(&amount, BTC_DENOM_BITCOIN, "-0.00000001");

    // Test 3: Whole number
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "42");
    verify_decimal_string(&amount, BTC_DENOM_BITCOIN, "42");

    // Test 4: Small value in satoshis
    crypto_set_from_decimal(&amount, BTC_DENOM_SATOSHI, "12345");
    verify_decimal_string(&amount, BTC_DENOM_SATOSHI, "12345");
    
    // Test 6: Test a conversion from BITCOIN to SATOSHI
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "1");
    verify_decimal_string(&amount, BTC_DENOM_SATOSHI, "100000000");
    
    // Test 7: Test a conversion from SATOSHI to BITCOIN
    crypto_set_from_decimal(&amount, BTC_DENOM_SATOSHI, "100000000");
    verify_decimal_string(&amount, BTC_DENOM_BITCOIN, "1");
    
    // Test 8: Test a conversion from MILLIBITCOIN to BITCOIN
    crypto_set_from_decimal(&amount, BTC_DENOM_MILLIBIT, "123456");
    verify_decimal_string(&amount, BTC_DENOM_BITCOIN, "123.45600000");

    // Test 9: Test a conversion from MICROBITCOIN to BITCOIN
    crypto_set_from_decimal(&amount, BTC_DENOM_MICROBIT, "123456789");
    verify_decimal_string(&amount, BTC_DENOM_BITCOIN, "123.45678900");

    // Test 10: Test a conversion from BITCOIN to MILLIBITCOIN
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "123.456");
    verify_decimal_string(&amount, BTC_DENOM_MILLIBIT, "123456");
    
    // Test 11: Test a conversion from BITCOIN to MICROBITCOIN
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "123.456");
    verify_decimal_string(&amount, BTC_DENOM_MICROBIT, "123456000");

    // Test 12: Negative decimal and convert to Microbitcoin
    crypto_set_from_decimal(&amount, BTC_DENOM_BITCOIN, "-0.00000001");
    verify_decimal_string(&amount, BTC_DENOM_MICROBIT, "-0.01");

    // Test 13: Test assertions
    SHOULD_ASSERT(verify_decimal_string(&amount, ETH_DENOM_ETHER, "-0.01"));
    SHOULD_ASSERT(verify_decimal_string(NULL, BTC_DENOM_BITCOIN, "-0.01"));
    SHOULD_ASSERT(verify_decimal_string(&amount, 42, "-0.01"));

    crypto_clear(&amount);
}

void test_multiplication_division() {
    printf("\n=== Testing Multiplication and Division Operations ===\n");
    
    // Test 1: Bitcoin multiplication (2 BTC * 2 = 4 BTC)
    crypto_val_t btc1, result;
    crypto_init(&btc1, CRYPTO_BITCOIN);
    crypto_init(&result, CRYPTO_BITCOIN);

    crypto_set_from_decimal(&btc1, BTC_DENOM_BITCOIN, "2");      // 2.0 BTC
    
    crypto_mul_i64(&result, &btc1, 2);
    verify_decimal_string(&result, BTC_DENOM_BITCOIN, "4");

    // Test 2: Bitcoin division (2 BTC / 2 = 100000000 SAT)
    crypto_divt_ui64(&result, &btc1, 2);
    verify_decimal_string(&result, BTC_DENOM_SATOSHI, "100000000");

    // Test 3: Bitcoin signed multiplication (2 BTC * -2 = -4 BTC)
    crypto_mul_i64(&result, &btc1, -2);
    verify_decimal_string(&result, BTC_DENOM_BITCOIN, "-4");

    // Test 4: Bicoin division by 0
    SHOULD_FPE(crypto_divt_ui64(&result, &btc1, 0));

    // Test 5: Bitcoin division should truncate
    crypto_set_from_decimal(&btc1, BTC_DENOM_BITCOIN, "1.23456788");
    crypto_divt_ui64(&result, &btc1, 3);
    verify_decimal_string(&result, BTC_DENOM_BITCOIN, "0.41152262");

    // Test 6: In-place multiplication (2 BTC *= 2 = 4 BTC)
    crypto_set_from_decimal(&btc1, BTC_DENOM_BITCOIN, "2");
    crypto_mul_i64(&btc1, &btc1, 2);
    verify_decimal_string(&btc1, BTC_DENOM_BITCOIN, "4");
        
    // Cleanup
    crypto_clear(&btc1);
    crypto_clear(&result);
}

void test_decimal_validation() {
    printf("\n=== Testing Decimal Validation ===\n");
    
    // Test valid cases
    TEST_VALID_DECIMAL("123.45");
    TEST_VALID_DECIMAL("  -123.45  ");
    TEST_VALID_DECIMAL("+123");
    TEST_VALID_DECIMAL("123");
    TEST_VALID_DECIMAL("0.123");

    // Test invalid cases
    TEST_INVALID_DECIMAL("123.45.67");
    TEST_INVALID_DECIMAL("abc");
    TEST_INVALID_DECIMAL("123abc");
    TEST_INVALID_DECIMAL("  ");
    TEST_INVALID_DECIMAL("+");
    TEST_INVALID_DECIMAL("123 456");

    // Test edge cases
    TEST_VALID_DECIMAL(".01");
    TEST_VALID_DECIMAL("0.01");
    TEST_VALID_DECIMAL("-.01");
    TEST_VALID_DECIMAL("-0.01");
}

// Helper function to execute a SQL query and verify the result
static void verify_sql_result(sqlite3 *db, const char* sql, const char* expected_result, const char* test_name) {
    sqlite3_stmt *stmt;
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
    }
    sqlite3_finalize(stmt);
}

// Helper function to verify SQL error handling
static void verify_sql_error(sqlite3 *db, const char* sql, const char* test_name) {
    sqlite3_stmt *stmt;
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
    rc = sqlite3_load_extension(db, "./crypto_decimal_extension.dylib", 0, &err_msg);
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

    verify_sql_error(db,
        "SELECT crypto_add('ETH', 'GWEI', 'invalid', '1.0')",
        "Invalid input handling");

    verify_sql_error(db,
        "SELECT crypto_add('ETH', 'GWEI', '1.0')",
        "Wrong number of arguments handling");

    // Test case for crypto_sum
    const char* sql = "CREATE TABLE t(val TEXT);"
                     "INSERT INTO t VALUES('1.234567890000000001'),('0.765432109999999999');"
                     "SELECT crypto_sum('ETH', 'ETH', 'GWEI', val) FROM t;";
    const char *pzTail = sql;

    total_tests++;
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
        "SELECT crypto_sub('BTC', 'BTC', '1', '0.5')",
        "0.50000000",
        "Subtraction with different cryptocurrencies");

    sqlite3_close(db);
}

int main() {
    printf("Starting Cryptomath Test Suite\n");

    test_decimal_string_parsing();
    test_decimal_string_conversion();
    test_arithmetic_operations();
    test_comparison_operations();
    test_zero_comparison();
    test_multiplication_division();
    test_decimal_validation();
    test_sqlite_extension();
    printf("\nTest Suite Summary:\n");
    printf("Total Tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success Rate: %.2f%%\n", (float)passed_tests / total_tests * 100);
    
    return failed_tests > 0 ? 1 : 0;
} 