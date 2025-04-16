/*
 * Copyright (c) 2025 Charles Benedict, Jr.
 * See LICENSE.md for licensing information.
 * This copyright notice must be retained in its entirety.
 * The LICENSE.md file must be retained and must be included with any distribution of this file.
 */

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
#include "cypto_get_denoms.h"
#include <gmp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#define CRYPTOMATH2_IMPLEMENTATION
#include "cryptomath2.h"

typedef enum {
    ARITHMETIC_ADD,
    ARITHMETIC_SUB,
    ARITHMETIC_MUL,
    ARITHMETIC_DIV
} crypto_arithmetic_op_t;

static const char *crypto_arithmetic_op_str[] = {
    "crypto_add",
    "crypto_sub",
    "crypto_mul",
    "crypto_div"
};

/*
 * Report a formatted error back to the SQL caller.
 * Internally formats via sqlite3_vsnprintf, then calls sqlite3_result_error().
 */
static void result_error_fmt(
  sqlite3_context *ctx,    /* The SQLite function context */
  const char      *fmt,    /* printf-style format string */
  ...
){
  char buf[256];           /* adjust size as needed */
  va_list ap;
  va_start(ap, fmt);
  /* SQLite’s own safe snprintf: will NUL-terminate */
  sqlite3_vsnprintf(sizeof(buf), buf, fmt, ap);
  va_end(ap);

  /* Pass -1 so SQLite copies the entire NUL‑terminated buffer */
  sqlite3_result_error(ctx, buf, -1);
}

//-----------------------------
// crypto_arithmetic_sqlite
//
// A SQLite function that performs an arithmetic operation on two crypto-decimal strings.
static void crypto_arithmetic_sqlite(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
){
    /* Pull out the arithmetic operation from the user data */
    crypto_arithmetic_op_t op = (crypto_arithmetic_op_t)(intptr_t)sqlite3_user_data(context);    

    // Expect 4 args
    if (argc != 4) {
        result_error_fmt(context, "%s: requires four arguments (crypto, denomination, operand1, operand2)", crypto_arithmetic_op_str[op]);
        return;
    }

    // Get text from args
    const unsigned char *crypto_type_str = sqlite3_value_text(argv[0]);
    const unsigned char *denom_str = sqlite3_value_text(argv[1]);
    const unsigned char *op_1_str = sqlite3_value_text(argv[2]);
    const unsigned char *op_2_str = sqlite3_value_text(argv[3]);
    if (!crypto_type_str || !denom_str || !op_1_str || !op_2_str) {
        result_error_fmt(context, "%s: Invalid arguments", crypto_arithmetic_op_str[op]);
        return;
    }

    // Get crypto_type for the first arg
    crypto_type_t crypto_type = crypto_get_type_for_symbol((const char*)crypto_type_str);
    if (crypto_type == CRYPTO_COUNT) {
        result_error_fmt(context, "%s: Invalid crypto type", crypto_arithmetic_op_str[op]);
        return;
    }

    // Get the denom for the first arg
    crypto_denom_t denom = crypto_get_denom_for_symbol(crypto_type, (const char*)denom_str);
    if (denom == DENOM_COUNT) {
        result_error_fmt(context, "%s: Invalid denomination", crypto_arithmetic_op_str[op]);
        return;
    }

    // Parse both operands into crypto_val_t
    crypto_val_t op_1, op_2;
    crypto_init(&op_1, crypto_type);
    crypto_init(&op_2, crypto_type);

    // Validate the first operand
    if (!crypto_is_valid_decimal((const char*)op_1_str)) {
        crypto_clear(&op_1); 
        crypto_clear(&op_2);
        result_error_fmt(context, "%s: Invalid decimal format for first operand", crypto_arithmetic_op_str[op]);
        return;
    }
    // Parse the first operand into crypto_val_t
    crypto_set_from_decimal(&op_1, denom, (const char*)op_1_str);

    // Validate the second operand
    if (!crypto_is_valid_decimal((const char*)op_2_str)) {
        crypto_clear(&op_1); 
        crypto_clear(&op_2);
        result_error_fmt(context, "%s: Invalid decimal format for second operand", crypto_arithmetic_op_str[op]);
        return;
    }
    // Parse the second operand into crypto_val_t
    crypto_set_from_decimal(&op_2, denom, (const char*)op_2_str);
    int64_t op_2_int = atoi((const char*)op_2_str);
    uint64_t op_2_uint = atoi((const char*)op_2_str);

    // Perform the arithmetic operation
    switch (op) {
        case ARITHMETIC_ADD:
            crypto_add(&op_1, &op_1, &op_2);
            break;
        case ARITHMETIC_SUB:
            crypto_sub(&op_1, &op_1, &op_2);
            break;
        case ARITHMETIC_MUL:
            crypto_mul_i64(&op_1, &op_1, op_2_int);
            break;
        case ARITHMETIC_DIV:
            crypto_divt_ui64(&op_1, &op_1, op_2_uint);
            break;
    }
    crypto_clear(&op_2);

    // Convert result to decimal string
    char *resultStr = crypto_to_decimal_str(&op_1, denom);
    crypto_clear(&op_1);

    if (!resultStr) {
        result_error_fmt(context, "%s: Could not convert result to string", crypto_arithmetic_op_str[op]);
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
    void *pOp = (void*)(intptr_t)ARITHMETIC_ADD;
    if (sqlite3_create_function(db, "crypto_add", 4, SQLITE_UTF8, pOp,
                                crypto_arithmetic_sqlite, NULL, NULL) != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_add function");
        return SQLITE_ERROR;
    }
    // Create or register the function crypto_sub
    pOp = (void*)(intptr_t)ARITHMETIC_SUB;
    if (sqlite3_create_function(db, "crypto_sub", 4, SQLITE_UTF8, pOp,
                                crypto_arithmetic_sqlite, NULL, NULL) != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_sub function");
        return SQLITE_ERROR;
    }
    // Create or register the function crypto_mul
    pOp = (void*)(intptr_t)ARITHMETIC_MUL;
    if (sqlite3_create_function(db, "crypto_mul", 4, SQLITE_UTF8, pOp,
                                crypto_arithmetic_sqlite, NULL, NULL) != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_mul function");
        return SQLITE_ERROR;
    }
    // Create or register the function crypto_div
    pOp = (void*)(intptr_t)ARITHMETIC_DIV;
    if (sqlite3_create_function(db, "crypto_div", 4, SQLITE_UTF8, pOp,
                                crypto_arithmetic_sqlite, NULL, NULL) != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_div function");
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

    // Register "crypto_denoms" virtual table.
    rc = sqlite3_create_module(db, "crypto_denoms", &cryptoDenomsModule, 0);
    if (rc == SQLITE_OK) {
        /* Eponymous virtual table: "crypto_types" is both the module & table name.
           This CREATE VIRTUAL TABLE statement is run in the temp schema so that
           the user can do "SELECT * FROM crypto_denoms()" with no extra steps.
         */
        rc = sqlite3_exec(db,
            "CREATE VIRTUAL TABLE temp.crypto_denoms USING crypto_denoms",
            NULL, NULL, NULL
        );
    }

    if (rc != SQLITE_OK) {
        *pzErrMsg = sqlite3_mprintf("Error registering crypto_denoms virtual table");
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}
