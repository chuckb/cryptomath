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
#include "cypto_get_types.h"
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
    // Expect 4 args
    if (argc != 4) {
        sqlite3_result_error(context, "crypto_add requires four arguments (crypto, denomination, operand1, operand2)", -1);
        return;
    }

    // Get text from args
    const unsigned char *crypto_type_str = sqlite3_value_text(argv[0]);
    const unsigned char *denom_str = sqlite3_value_text(argv[1]);
    const unsigned char *op_1_str = sqlite3_value_text(argv[2]);
    const unsigned char *op_2_str = sqlite3_value_text(argv[3]);
    if (!crypto_type_str || !denom_str || !op_1_str || !op_2_str) {
        sqlite3_result_null(context);
        return;
    }

    // Get crypto_type for the first arg
    crypto_type_t crypto_type = crypto_get_type_for_symbol((const char*)crypto_type_str);
    if (crypto_type == CRYPTO_COUNT) {
        sqlite3_result_error(context, "crypto_add: Invalid crypto type", -1);
        return;
    }

    // Get the denom for the first arg
    crypto_denom_t denom = crypto_get_denom_for_symbol(crypto_type, (const char*)denom_str);
    if (denom == DENOM_COUNT) {
        sqlite3_result_error(context, "crypto_add: Invalid denomination", -1);
        return;
    }

    // Parse both operands into crypto_val_t
    crypto_val_t a, b;
    crypto_init(&a, crypto_type);
    crypto_init(&b, crypto_type);

    // Validate the first operand
    if (!crypto_is_valid_decimal((const char*)op_1_str)) {
        crypto_clear(&a); 
        crypto_clear(&b);
        sqlite3_result_error(context, "crypto_add: Invalid decimal format for first operand", -1);
        return;
    }
    // Parse the first operand into crypto_val_t
    crypto_set_from_decimal(&a, denom, (const char*)op_1_str);

    // Validate the second operand
    if (!crypto_is_valid_decimal((const char*)op_2_str)) {
        crypto_clear(&a); 
        crypto_clear(&b);
        sqlite3_result_error(context, "crypto_add: Invalid decimal format for second operand", -1);
        return;
    }
    // Parse the second operand into crypto_val_t
    crypto_set_from_decimal(&b, denom, (const char*)op_2_str);

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
    // Expect 4 args
    if (argc != 4) {
        sqlite3_result_error(context, "crypto_scale requires four arguments (crypto, from_denom, to_denom, operand)", -1);
        return;
    }

    // Get text from args
    const unsigned char *crypto_type_str = sqlite3_value_text(argv[0]);
    const unsigned char *from_denom_str = sqlite3_value_text(argv[1]);
    const unsigned char *to_denom_str = sqlite3_value_text(argv[2]);
    const unsigned char *operand_str = sqlite3_value_text(argv[3]);
    if (!crypto_type_str || !from_denom_str || !to_denom_str || !operand_str) {
        sqlite3_result_null(context);
        return;
    }

    // Get crypto_type
    crypto_type_t crypto_type = crypto_get_type_for_symbol((const char*)crypto_type_str);
    if (crypto_type == CRYPTO_COUNT) {
        sqlite3_result_error(context, "crypto_scale: Invalid crypto type", -1);
        return;
    }

    // Get the denom for the first arg
    crypto_denom_t from_denom = crypto_get_denom_for_symbol(crypto_type, (const char*)from_denom_str);
    if (from_denom == DENOM_COUNT) {
        sqlite3_result_error(context, "crypto_scale: Invalid from denomination", -1);
        return;
    }

    // Get the denom for the second arg
    crypto_denom_t to_denom = crypto_get_denom_for_symbol(crypto_type, (const char*)to_denom_str);
    if (to_denom == DENOM_COUNT) {
        sqlite3_result_error(context, "crypto_scale: Invalid to denomination", -1);
        return;
    }

    // Parse the third operand into crypto_val_t
    crypto_val_t a;
    crypto_init(&a, crypto_type);
    crypto_set_from_decimal(&a, from_denom, (const char*)operand_str);

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

// ----------------------------------------------------------------------
// Aggregation context for crypto_sum
//
// This is used to sum up a series of crypto-decimal values.
// It is used to implement the crypto_sum function.
//
// Example:
//   SELECT crypto_sum(denom, operand) FROM table;

typedef struct {
    crypto_val_t sum;            // running sum
    crypto_denom_t final_denom;  // final denomination
    int initialized;             // 0 if not yet initialized with any value
} crypto_sum_ctx_t;

// Step function: called for each row
static void crypto_sum_step(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
){
    // Expect 4 args
    if (argc != 4) {
        sqlite3_result_error(context, "crypto_sum requires 4 arguments (crypto, operand_denomination, final_denomination, operand)", -1);
        return;
    }

    // Parse the command arguments
    const unsigned char *crypto_type_str = sqlite3_value_text(argv[0]);
    const unsigned char *operand_denom_str = sqlite3_value_text(argv[1]);
    const unsigned char *final_denom_str = sqlite3_value_text(argv[2]);
    const unsigned char *operand_str = sqlite3_value_text(argv[3]);
    if (!crypto_type_str || !operand_denom_str || !final_denom_str) {
        sqlite3_result_null(context);
        return;
    }

    if (!operand_str || !crypto_is_valid_decimal((const char*)operand_str)) {
        // treat as 0
        return;
    }

    // Get crypto_type for the final denom
    crypto_type_t crypto_type = crypto_get_type_for_symbol((const char*)crypto_type_str);
    if (crypto_type == CRYPTO_COUNT) {
        sqlite3_result_error(context, "crypto_sum: Invalid crypto type", -1);
        return;
    }

    // Get the final denom
    crypto_denom_t final_denom = crypto_get_denom_for_symbol(crypto_type, (const char*)final_denom_str);
    if (final_denom == DENOM_COUNT) {
        sqlite3_result_error(context, "crypto_sum: Invalid final denomination", -1);
        return;
    }

    // Get the operand denom
    crypto_denom_t operand_denom = crypto_get_denom_for_symbol(crypto_type, (const char*)operand_denom_str);
    if (operand_denom == DENOM_COUNT) {
        sqlite3_result_error(context, "crypto_sum: Invalid operand denomination", -1);
        return;
    }

    // Access aggregator context
    crypto_sum_ctx_t *p = (crypto_sum_ctx_t *)sqlite3_aggregate_context(context, sizeof(*p));
    if (!p) {
        // Out of memory
        sqlite3_result_error_nomem(context);
        return;
    }

    // If first time, initialize crypto_val_t
    // Note: crypto_init() will initialize the value to 0
    if (!p->initialized) {
        crypto_init(&p->sum, crypto_type);
        p->final_denom = final_denom;
        p->initialized = 1;
    }

    // Parse the operand into crypto_val_t
    crypto_val_t operand;
    crypto_init(&operand, crypto_type);
    crypto_set_from_decimal(&operand, operand_denom, (const char*)operand_str);

    // Add to sum
    crypto_add(&p->sum, &operand, &p->sum);
    crypto_clear(&operand);
}

// Final function: called after all rows processed
static void crypto_sum_final(sqlite3_context *context) {
    crypto_sum_ctx_t *p = (crypto_sum_ctx_t *)sqlite3_aggregate_context(context, 0);

    // If no rows were processed or aggregator not created, result = NULL
    if (!p || !p->initialized) {
        sqlite3_result_null(context);
        return;
    }

    // Convert p->sum to decimal string
    char *result_str = crypto_to_decimal_str(&p->sum, p->final_denom);
    // Clear aggregator memory
    crypto_clear(&p->sum);
    p->initialized = 0;

    if (!result_str) {
        sqlite3_result_error(context, "crypto_sum:Memory error converting sum to string", -1);
        return;
    }

    // Return as TEXT
    sqlite3_result_text(context, result_str, -1, SQLITE_TRANSIENT);
    free(result_str);
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
    if (sqlite3_create_function(db, "crypto_add", 4, SQLITE_UTF8, NULL,
                                crypto_add_sqlite, NULL, NULL) != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_add function");
        return SQLITE_ERROR;
    }
    // Create or register the function crypto_scale
    if (sqlite3_create_function(db, "crypto_scale", 4, SQLITE_UTF8, NULL,
                                crypto_scale_sqlite, NULL, NULL) != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_scale function");
        return SQLITE_ERROR;
    }
    // create aggregate function: crypto_sum
    // The last parameter to sqlite3_create_function_v2 is a destructor for the app data, if needed.
    // We'll pass NULL here.
    int rc = sqlite3_create_function_v2(
        db,
        "crypto_sum",         // function name
        4,                 // number of arguments
        SQLITE_UTF8,       // preferred text encoding
        NULL,              // application data
        NULL,              // xFunc (scalar)
        crypto_sum_step,      // xStep (aggregate step)
        crypto_sum_final,     // xFinal (aggregate final)
        NULL               // xDestroy
    );

    if (rc != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_sum function");
        return SQLITE_ERROR;
    }

    // Register "crypto_types" virtual table.
    rc = sqlite3_create_module(db, "crypto_types", &cryptoTypesModule, 0);
    if (rc == SQLITE_OK) {
        /* Eponymous virtual table: "crypto_types" is both the module & table name.
           This CREATE VIRTUAL TABLE statement is run in the temp schema so that
           the user can do "SELECT * FROM crypto_types()" with no extra steps.
         */
        rc = sqlite3_exec(db,
            "CREATE VIRTUAL TABLE temp.crypto_types USING crypto_types",
            NULL, NULL, NULL
        );
    }

    if (rc != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_types virtual table");
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}
