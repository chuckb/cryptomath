/*
  Compile on Linux (example):
    gcc -fPIC -shared -o crypto_decimal_extension.so crypto_decimal_extension.c \
        -lgmp -lsqlite3

  Then in SQLite:
    .load ./crypto_decimal_extension
    SELECT crypto_scale('ETH', 'GWEI', crypto_add('ETH', '1.234567891', '0.765432109')); 
    -- Should yield 2000000000
*/

#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1
#include <gmp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#define CRYPTOMATH2_IMPLEMENTATION
#include "cryptomath2.h"

//-----------------------------
// crypto_add_sqlite
//
// A SQLite function that adds two crypto-decimal strings exactly.
static void crypto_add_sqlite(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
){
    // Expect 3 args
    if (argc != 3) {
        sqlite3_result_error(context, "crypto_add requires three arguments (denomination, operand1, operand2)", -1);
        return;
    }

    // Get text from args
    const unsigned char *arg0 = sqlite3_value_text(argv[0]);
    const unsigned char *arg1 = sqlite3_value_text(argv[1]);
    const unsigned char *arg2 = sqlite3_value_text(argv[2]);
    if (!arg0 || !arg1 || !arg2) {
        sqlite3_result_null(context);
        return;
    }

    // Get the denom for the first arg
    crypto_denom_t denom = crypto_get_denom_for_symbol((const char*)arg0);
    if (denom == DENOM_COUNT) {
        sqlite3_result_error(context, "crypto_add: Invalid denomination", -1);
        return;
    }
    // Get crypto_type for the first arg
    crypto_type_t crypto_type = crypto_denoms[denom].crypto_type;

    // Parse both operands into crypto_val_t
    crypto_val_t a, b;
    crypto_init(&a, crypto_type);
    crypto_init(&b, crypto_type);

    // Validate the first operand
    if (!crypto_is_valid_decimal((const char*)arg1)) {
        crypto_clear(&a); 
        crypto_clear(&b);
        sqlite3_result_error(context, "crypto_add: Invalid decimal format for first operand", -1);
        return;
    }
    // Parse the first operand into crypto_val_t
    crypto_set_from_decimal(&a, denom, (const char*)arg1);

    // Validate the second operand
    if (!crypto_is_valid_decimal((const char*)arg2)) {
        crypto_clear(&a); 
        crypto_clear(&b);
        sqlite3_result_error(context, "crypto_add: Invalid decimal format for second operand", -1);
        return;
    }
    // Parse the second operand into crypto_val_t
    crypto_set_from_decimal(&b, denom, (const char*)arg2);

    // Add
    crypto_add(&a, &b, &a); // store result in a
    crypto_clear(&b);

    // Convert result to decimal string
    char *resultStr = crypto_to_decimal_str(&a, denom);
    crypto_clear(&a);

    if (!resultStr) {
        sqlite3_result_error(context, "crypto_add: Could not convert result to string", -1);
        return;
    }

    // Return to SQLite
    sqlite3_result_text(context, resultStr, -1, SQLITE_TRANSIENT);
    free(resultStr);
}

//-----------------------------
// crypto_scale_sqlite
//
// A SQLite function that scales a crypto-decimal according to a denom given.
static void crypto_scale_sqlite(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
){
    // Expect 3 args
    if (argc != 3) {
        sqlite3_result_error(context, "crypto_scale requires three arguments (from_denom, to_denom, operand)", -1);
        return;
    }

    // Get text from args
    const unsigned char *arg0 = sqlite3_value_text(argv[0]);
    const unsigned char *arg1 = sqlite3_value_text(argv[1]);
    const unsigned char *arg2 = sqlite3_value_text(argv[2]);
    if (!arg0 || !arg1 || !arg2) {
        sqlite3_result_null(context);
        return;
    }

    // Get the denom for the first arg
    crypto_denom_t from_denom = crypto_get_denom_for_symbol((const char*)arg0);
    if (from_denom == DENOM_COUNT) {
        sqlite3_result_error(context, "crypto_scale: Invalid from denomination", -1);
        return;
    }
    // Get crypto_type for the first arg
    crypto_type_t from_type = crypto_denoms[from_denom].crypto_type;

    // Get the denom for the second arg
    crypto_denom_t to_denom = crypto_get_denom_for_symbol((const char*)arg1);
    if (to_denom == DENOM_COUNT) {
        sqlite3_result_error(context, "crypto_scale: Invalid to denomination", -1);
        return;
    }
    // Get crypto_type for the second arg
    crypto_type_t to_type = crypto_denoms[to_denom].crypto_type;

    // Types must match
    if (from_type != to_type) {
        sqlite3_result_error(context, "crypto_scale: From and to crypto types must match", -1);
        return;
    }

    // Parse the third operand into crypto_val_t
    crypto_val_t a;
    crypto_init(&a, from_type);
    crypto_set_from_decimal(&a, from_denom, (const char*)arg2);

    // Convert result to decimal string
    char *resultStr = crypto_to_decimal_str(&a, to_denom);
    crypto_clear(&a);

    if (!resultStr) {
        sqlite3_result_error(context, "crypto_scale: Could not convert result to string", -1);
        return;
    }

    // Return to SQLite
    sqlite3_result_text(context, resultStr, -1, SQLITE_TRANSIENT);
    free(resultStr);
}

//-----------------------------
// Entry point for the extension
#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_cryptodecimalextension_init(
    sqlite3 *db,
    char **pzErrMsg,
    const sqlite3_api_routines *pApi
){
    SQLITE_EXTENSION_INIT2(pApi);
    // Create or register the function crypto_add
    if (sqlite3_create_function(db, "crypto_add", 3, SQLITE_UTF8, NULL,
                                crypto_add_sqlite, NULL, NULL) != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_add function");
        return SQLITE_ERROR;
    }
    // Create or register the function crypto_scale
    if (sqlite3_create_function(db, "crypto_scale", 3, SQLITE_UTF8, NULL,
                                crypto_scale_sqlite, NULL, NULL) != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_scale function");
        return SQLITE_ERROR;
    }
    return SQLITE_OK;
}
