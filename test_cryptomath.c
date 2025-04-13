#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CRYPTOMATH_IMPLEMENTATION
#include "cryptomath.h"

// Test result tracking
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// Helper function to verify a result
void verify_result(const char* operation, const crypto_amount_t* result, 
                  crypto_type_t type, uint8_t expected_unit, uint64_t expected_whole, 
                  uint64_t expected_fraction, int is_negative) {
    total_tests++;
    
    // Convert result to string for comparison
    char* str = mpq_get_str(NULL, 10, result->value);
    if (!str) {
        printf("ERROR: Failed to convert result to string\n");
        failed_tests++;
        return;
    }
    
    // Create expected value
    crypto_amount_t expected;
    switch (type) {
        case CRYPTO_BITCOIN:
            crypto_init_bitcoin(&expected, expected_unit);
            break;
        case CRYPTO_ETHEREUM:
            crypto_init_ethereum(&expected, expected_unit);
            break;
        default:
            printf("ERROR: Invalid crypto type\n");
            free(str);
            failed_tests++;
            return;
    }
    
    crypto_set_value(&expected, type, expected_unit, expected_whole, expected_fraction);
    
    // Apply sign if needed
    if (is_negative) {
        mpq_neg(expected.value, expected.value);
    }
    
    // Compare
    int cmp = crypto_cmp(result, &expected);
    printf("%s = ", operation);
    crypto_print_amount(result);
    printf("  %s\n", cmp == 0 ? "✓ PASSED" : "✗ FAILED");
    
    if (cmp != 0) {
        printf("  Expected: ");
        crypto_print_amount(&expected);
        failed_tests++;
    } else {
        passed_tests++;
    }
    
    // Verify the result has the correct type and unit
    if (result->type != type) {
        printf("  ERROR: Result has wrong crypto type (got %d, expected %d)\n", result->type, type);
        failed_tests++;
    }
    if (result->unit != expected_unit) {
        printf("  ERROR: Result has wrong unit (got %d, expected %d)\n", result->unit, expected_unit);
        failed_tests++;
    }
    
    free(str);
    crypto_clear(&expected);
}

void test_bitcoin_conversions() {
    printf("\n=== Testing Bitcoin Conversions ===\n");
    
    // Test 1: 1 BTC to SAT
    crypto_amount_t btc, sat;
    crypto_init_bitcoin(&btc, BTC_UNIT_BITCOIN);
    crypto_set_value(&btc, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 1, 0);  // 1.0 BTC
    
    crypto_convert_to_unit(&btc, &sat, BTC_UNIT_SATOSHI);
    printf("1 BTC = ");
    crypto_print_amount(&sat);
    
    // Test 2: 100000000 SAT to BTC
    crypto_init_bitcoin(&sat, BTC_UNIT_SATOSHI);
    crypto_set_value(&sat, CRYPTO_BITCOIN, BTC_UNIT_SATOSHI, 100000000, 0);  // 100000000 SAT
    
    crypto_convert_to_unit(&sat, &btc, BTC_UNIT_BITCOIN);
    printf("100000000 SAT = ");
    crypto_print_amount(&btc);
    
    // Test 3: 0.001 BTC to mBTC
    crypto_init_bitcoin(&btc, BTC_UNIT_BITCOIN);
    crypto_set_value(&btc, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 0, 100000);  // 0.001 BTC
    
    crypto_amount_t mbtc;
    crypto_convert_to_unit(&btc, &mbtc, BTC_UNIT_MILLIBIT);
    printf("0.001 BTC = ");
    crypto_print_amount(&mbtc);
    
    // Test 4: 1.234 mBTC
    crypto_init_bitcoin(&mbtc, BTC_UNIT_MILLIBIT);
    crypto_set_value(&mbtc, CRYPTO_BITCOIN, BTC_UNIT_MILLIBIT, 1, 234);  // 1.234 mBTC
    printf("1.234 mBTC = ");
    crypto_print_amount(&mbtc);
    
    // Test 5: 123.45 μBTC
    crypto_amount_t ubtc;
    crypto_init_bitcoin(&ubtc, BTC_UNIT_MICROBIT);
    crypto_set_value(&ubtc, CRYPTO_BITCOIN, BTC_UNIT_MICROBIT, 123, 45);  // 123.45 μBTC
    printf("123.45 μBTC = ");
    crypto_print_amount(&ubtc);
    
    // Cleanup
    crypto_clear(&btc);
    crypto_clear(&sat);
    crypto_clear(&mbtc);
    crypto_clear(&ubtc);
}

void test_unit_info() {
    printf("\n=== Testing Unit Information ===\n");
    
    const crypto_unit_t* unit;
    
    unit = crypto_get_unit_info(CRYPTO_BITCOIN, BTC_UNIT_BITCOIN);
    printf("Bitcoin unit info:\n");
    printf("  Name: %s\n", unit->name);
    printf("  Symbol: %s\n", unit->symbol);
    printf("  Denominator: %llu\n", unit->denominator);
    printf("  Decimals: %u\n", unit->decimals);
    
    unit = crypto_get_unit_info(CRYPTO_BITCOIN, BTC_UNIT_SATOSHI);
    printf("\nSatoshi unit info:\n");
    printf("  Name: %s\n", unit->name);
    printf("  Symbol: %s\n", unit->symbol);
    printf("  Denominator: %llu\n", unit->denominator);
    printf("  Decimals: %u\n", unit->decimals);
}

void test_invalid_units() {
    printf("\n=== Testing Invalid Units ===\n");
    
    int result;
    
    // Test invalid crypto type
    result = crypto_is_valid_unit(CRYPTO_COUNT, BTC_UNIT_BITCOIN);
    printf("Invalid crypto type test: %s\n", result ? "FAILED" : "PASSED");
    
    // Test invalid unit
    result = crypto_is_valid_unit(CRYPTO_BITCOIN, BTC_UNIT_COUNT);
    printf("Invalid unit test: %s\n", result ? "FAILED" : "PASSED");
}

void test_arithmetic_operations() {
    printf("\n=== Testing Arithmetic Operations ===\n");
    
    // Test 1: Adding different units (1 BTC + 100 mBTC = 1.1 BTC)
    crypto_amount_t btc, mbtc, result;
    crypto_init_bitcoin(&btc, BTC_UNIT_BITCOIN);
    crypto_init_bitcoin(&mbtc, BTC_UNIT_MILLIBIT);
    
    crypto_set_value(&btc, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 1, 0);      // 1.0 BTC
    crypto_set_value(&mbtc, CRYPTO_BITCOIN, BTC_UNIT_MILLIBIT, 100, 0);  // 100 mBTC = 0.1 BTC
    
    crypto_add(&btc, &mbtc, &result);
    verify_result("1 BTC + 100 mBTC", &result, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 1, 10000000, 0);
    
    // Test 2: Subtracting different units (1 BTC - 50000000 SAT = 0.5 BTC)
    crypto_amount_t sat;
    crypto_init_bitcoin(&sat, BTC_UNIT_SATOSHI);
    crypto_set_value(&sat, CRYPTO_BITCOIN, BTC_UNIT_SATOSHI, 50000000, 0);  // 0.5 BTC in satoshis
    
    crypto_sub(&btc, &sat, &result);
    verify_result("1 BTC - 50000000 SAT", &result, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 0, 50000000, 0);
    
    // Test 3: Comparison of different units (50000000 SAT == 500000 μBTC)
    crypto_amount_t ubtc;
    crypto_init_bitcoin(&ubtc, BTC_UNIT_MICROBIT);
    crypto_set_value(&ubtc, CRYPTO_BITCOIN, BTC_UNIT_MICROBIT, 500000, 0);  // 0.5 BTC in μBTC
    
    int cmp_result = crypto_cmp(&sat, &ubtc);
    printf("50000000 SAT %s 500000 μBTC: %s\n", 
           cmp_result == 0 ? "==" : (cmp_result > 0 ? ">" : "<"),
           cmp_result == 0 ? "✓ PASSED" : "✗ FAILED");
    
    // Test 4: Complex arithmetic (1 BTC + 100 mBTC - 50000000 SAT = 0.6 BTC)
    crypto_amount_t temp;
    crypto_add(&btc, &mbtc, &temp);
    crypto_sub(&temp, &sat, &result);
    verify_result("1 BTC + 100 mBTC - 50000000 SAT", &result, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 0, 60000000, 0);
    
    // Test 5: Small value arithmetic (0.001 BTC + 1000 SAT = 0.002 BTC)
    crypto_set_value(&btc, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 0, 100000);  // 0.001 BTC
    crypto_set_value(&sat, CRYPTO_BITCOIN, BTC_UNIT_SATOSHI, 1000, 0);    // 1000 SAT = 0.00001 BTC
    
    crypto_add(&btc, &sat, &result);
    verify_result("0.001 BTC + 1000 SAT", &result, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 0, 101000, 0);
    
    // Test 6: Precision test (0.00000001 BTC + 1 SAT = 0.00000002 BTC)
    crypto_set_value(&btc, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 0, 1);       // 0.00000001 BTC
    crypto_set_value(&sat, CRYPTO_BITCOIN, BTC_UNIT_SATOSHI, 1, 0);       // 1 SAT = 0.00000001 BTC
    
    crypto_add(&btc, &sat, &result);
    verify_result("0.00000001 BTC + 1 SAT", &result, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 0, 2, 0);
    
    // Cleanup
    crypto_clear(&btc);
    crypto_clear(&mbtc);
    crypto_clear(&sat);
    crypto_clear(&ubtc);
    crypto_clear(&result);
    crypto_clear(&temp);
}

void test_decimal_string_parsing() {
    printf("\n=== Testing Decimal String Parsing ===\n");
    
    // Test 1: Basic positive decimal
    crypto_amount_t amount;
    crypto_init_bitcoin(&amount, BTC_UNIT_BITCOIN);
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, "1.23456789");
    verify_result("1.23456789 BTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 1, 23456789, 0);
    
    // Test 2: Negative decimal
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, "-0.00000001");
    verify_result("-0.00000001 BTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 0, 1, 1);
    
    // Test 3: Whole number
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, "42");
    verify_result("42 BTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 42, 0, 0);
    
    // Test 4: Small value in satoshis
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_SATOSHI, "12345");
    verify_result("12345 SAT", &amount, CRYPTO_BITCOIN, BTC_UNIT_SATOSHI, 12345, 0, 0);
    
    // Test 5: Millibitcoin with decimal
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_MILLIBIT, "1.23456");
    verify_result("1.23456 mBTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_MILLIBIT, 1, 23456, 0);
    
    // Test 6: Microbitcoin with decimal
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_MICROBIT, "123.45");
    verify_result("123.45 μBTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_MICROBIT, 123, 45, 0);
    
    // Test 7: Zero with decimal
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, "0.00000000");
    verify_result("0.00000000 BTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 0, 0, 0);
    
    // Test 8: Leading zeros
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, "0001.23456789");
    verify_result("0001.23456789 BTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 1, 23456789, 0);
    
    // Test 9: Negative values in different units
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, "-1.23456789");
    verify_result("-1.23456789 BTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, 1, 23456789, 1);
    
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_SATOSHI, "-12345");
    verify_result("-12345 SAT", &amount, CRYPTO_BITCOIN, BTC_UNIT_SATOSHI, 12345, 0, 1);
    
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_MILLIBIT, "-1.23456");
    verify_result("-1.23456 mBTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_MILLIBIT, 1, 23456, 1);
    
    crypto_set_value_from_decimal(&amount, CRYPTO_BITCOIN, BTC_UNIT_MICROBIT, "-123.45");
    verify_result("-123.45 μBTC", &amount, CRYPTO_BITCOIN, BTC_UNIT_MICROBIT, 123, 45, 1);
    
    // Cleanup
    crypto_clear(&amount);
}

void test_ethereum_conversions() {
    printf("\n=== Testing Ethereum Conversions ===\n");
    
    // Test 1: 1 ETH to WEI
    crypto_amount_t eth, wei;
    crypto_init_ethereum(&eth, ETH_UNIT_ETHER);
    crypto_set_value(&eth, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 1, 0);  // 1.0 ETH
    
    crypto_convert_to_unit(&eth, &wei, ETH_UNIT_WEI);
    printf("1 ETH = ");
    crypto_print_amount(&wei);
    
    // Test 2: 1000000000 GWEI to ETH
    crypto_amount_t gwei;
    crypto_init_ethereum(&gwei, ETH_UNIT_GWEI);
    crypto_set_value(&gwei, CRYPTO_ETHEREUM, ETH_UNIT_GWEI, 1000000000, 0);  // 1000000000 GWEI
    
    crypto_convert_to_unit(&gwei, &eth, ETH_UNIT_ETHER);
    printf("1000000000 GWEI = ");
    crypto_print_amount(&eth);
    
    // Test 3: 0.001 ETH to GWEI
    crypto_init_ethereum(&eth, ETH_UNIT_ETHER);
    crypto_set_value(&eth, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 0, 1000000000000000);  // 0.001 ETH
    
    crypto_convert_to_unit(&eth, &gwei, ETH_UNIT_GWEI);
    printf("0.001 ETH = ");
    crypto_print_amount(&gwei);
    
    // Cleanup
    crypto_clear(&eth);
    crypto_clear(&wei);
    crypto_clear(&gwei);
}

void test_ethereum_unit_info() {
    printf("\n=== Testing Ethereum Unit Information ===\n");
    
    const crypto_unit_t* unit;
    
    unit = crypto_get_unit_info(CRYPTO_ETHEREUM, ETH_UNIT_ETHER);
    printf("Ether unit info:\n");
    printf("  Name: %s\n", unit->name);
    printf("  Symbol: %s\n", unit->symbol);
    printf("  Denominator: %llu\n", unit->denominator);
    printf("  Decimals: %u\n", unit->decimals);
    
    unit = crypto_get_unit_info(CRYPTO_ETHEREUM, ETH_UNIT_GWEI);
    printf("\nGwei unit info:\n");
    printf("  Name: %s\n", unit->name);
    printf("  Symbol: %s\n", unit->symbol);
    printf("  Denominator: %llu\n", unit->denominator);
    printf("  Decimals: %u\n", unit->decimals);
    
    unit = crypto_get_unit_info(CRYPTO_ETHEREUM, ETH_UNIT_WEI);
    printf("\nWei unit info:\n");
    printf("  Name: %s\n", unit->name);
    printf("  Symbol: %s\n", unit->symbol);
    printf("  Denominator: %llu\n", unit->denominator);
    printf("  Decimals: %u\n", unit->decimals);
}

void test_ethereum_arithmetic() {
    printf("\n=== Testing Ethereum Arithmetic Operations ===\n");
    
    // Test 1: Adding different units (1 ETH + 100 GWEI = 1.0000001 ETH)
    crypto_amount_t eth, gwei, result;
    crypto_init_ethereum(&eth, ETH_UNIT_ETHER);
    crypto_init_ethereum(&gwei, ETH_UNIT_GWEI);
    
    crypto_set_value(&eth, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 1, 0);      // 1.0 ETH
    crypto_set_value(&gwei, CRYPTO_ETHEREUM, ETH_UNIT_GWEI, 100, 0);    // 100 GWEI = 0.0000001 ETH
    
    crypto_add(&eth, &gwei, &result);
    verify_result("1 ETH + 100 GWEI", &result, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 1, 100000000000, 0);
    
    // Test 2: Subtracting different units (1 ETH - 500000000 GWEI = 0.5 ETH)
    crypto_set_value(&gwei, CRYPTO_ETHEREUM, ETH_UNIT_GWEI, 500000000, 0);  // 0.5 ETH in gwei
    
    crypto_sub(&eth, &gwei, &result);
    verify_result("1 ETH - 500000000 GWEI", &result, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 0, 500000000000000000, 0);
    
    // Test 3: Comparison of different units (500000000 GWEI == 0.5 ETH)
    crypto_amount_t half_eth;
    crypto_init_ethereum(&half_eth, ETH_UNIT_ETHER);
    crypto_set_value(&half_eth, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 0, 500000000000000000);  // 0.5 ETH
    
    int cmp_result = crypto_cmp(&gwei, &half_eth);
    printf("500000000 GWEI %s 0.5 ETH: %s\n", 
           cmp_result == 0 ? "==" : (cmp_result > 0 ? ">" : "<"),
           cmp_result == 0 ? "✓ PASSED" : "✗ FAILED");
    if (cmp_result == 0) passed_tests++; else failed_tests++;
    total_tests++;
    
    // Test 4: Adding ETH and WEI (1 ETH + 1000000000000000000 WEI = 2 ETH)
    crypto_amount_t wei;
    crypto_init_ethereum(&wei, ETH_UNIT_WEI);
    crypto_set_value(&wei, CRYPTO_ETHEREUM, ETH_UNIT_WEI, 1000000000000000000, 0);  // 1 ETH in wei
    
    crypto_add(&eth, &wei, &result);
    verify_result("1 ETH + 1000000000000000000 WEI", &result, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 2, 0, 0);
    
    // Test 5: Subtracting WEI from GWEI (1 GWEI - 1000000000 WEI = 0.999 GWEI)
    crypto_set_value(&gwei, CRYPTO_ETHEREUM, ETH_UNIT_GWEI, 1000000000, 0);  // 1 GWEI
    crypto_set_value(&wei, CRYPTO_ETHEREUM, ETH_UNIT_WEI, 1000000000, 0);    // 0.001 GWEI in wei
    
    crypto_sub(&gwei, &wei, &result);
    verify_result("1 GWEI - 1000000000 WEI", &result, CRYPTO_ETHEREUM, ETH_UNIT_GWEI, 999999999, 0, 0);
    
    // Test 6: Complex arithmetic with all units (1 ETH + 100 GWEI - 1 ETH = 100 GWEI)
    crypto_set_value(&eth, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 1, 0);      // 1 ETH
    crypto_set_value(&gwei, CRYPTO_ETHEREUM, ETH_UNIT_GWEI, 100, 0);    // 100 GWEI
    
    // First add ETH and GWEI
    crypto_add(&eth, &gwei, &result);
    
    // Then subtract 1 ETH using in-place subtraction
    crypto_sub_inplace(&result, &eth);
    
    verify_result("1 ETH + 100 GWEI - 1 ETH", &result, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 0, 100000000000, 0);
    
    // Cleanup
    crypto_clear(&eth);
    crypto_clear(&gwei);
    crypto_clear(&wei);
    crypto_clear(&result);
    crypto_clear(&half_eth);
}

void test_ethereum_decimal_string_parsing() {
    printf("\n=== Testing Ethereum Decimal String Parsing ===\n");
    
    // Test 1: Basic positive decimal
    crypto_amount_t amount;
    crypto_init_ethereum(&amount, ETH_UNIT_ETHER);
    crypto_set_value_from_decimal(&amount, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, "1.234567890123456789");
    verify_result("1.234567890123456789 ETH", &amount, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 1, 234567890123456789, 0);
    
    // Test 2: Negative decimal
    crypto_set_value_from_decimal(&amount, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, "-0.000000000000000001");
    verify_result("-0.000000000000000001 ETH", &amount, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 0, 1, 1);
    
    // Test 3: Whole number
    crypto_set_value_from_decimal(&amount, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, "42");
    verify_result("42 ETH", &amount, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, 42, 0, 0);
    
    // Test 4: Small value in gwei
    crypto_set_value_from_decimal(&amount, CRYPTO_ETHEREUM, ETH_UNIT_GWEI, "12345");
    verify_result("12345 GWEI", &amount, CRYPTO_ETHEREUM, ETH_UNIT_GWEI, 12345, 0, 0);
    
    // Test 5: Wei with decimal
    crypto_set_value_from_decimal(&amount, CRYPTO_ETHEREUM, ETH_UNIT_WEI, "1234567890123456789");
    verify_result("1234567890123456789 WEI", &amount, CRYPTO_ETHEREUM, ETH_UNIT_WEI, 1234567890123456789, 0, 0);
    
    // Cleanup
    crypto_clear(&amount);
}

int main() {
    printf("Starting Cryptomath Test Suite\n");
    
    test_unit_info();
    test_bitcoin_conversions();
    test_ethereum_unit_info();
    test_ethereum_conversions();
    test_invalid_units();
    test_arithmetic_operations();
    test_ethereum_arithmetic();
    test_decimal_string_parsing();
    test_ethereum_decimal_string_parsing();
    
    printf("\nTest Suite Summary:\n");
    printf("Total Tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success Rate: %.2f%%\n", (float)passed_tests / total_tests * 100);
    
    return failed_tests > 0 ? 1 : 0;
} 