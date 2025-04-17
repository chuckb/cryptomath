/*
 * Copyright (c) 2025 Charles Benedict, Jr.
 * See LICENSE.md for licensing information.
 * This copyright notice must be retained in its entirety.
 * The LICENSE.md file must be retained and must be included with any distribution of this file.
 */

// Usage:
//
// #define CRYPTO_IMPLEMENTATION
// #include "cryptomath2.h"
//
// crypto_val_t val1;
// crypto_init(&val1, CRYPTO_BITCOIN);
// crypto_set_from_decimal(&val1, BTC_DENOM_BITCOIN, "1.23456788");
//
// crypto_val_t val2;
// crypto_init(&val2, CRYPTO_BITCOIN);
// crypto_set_from_decimal(&val2, BTC_DENOM_BITCOIN, "2.46913576");
//
// crypto_val_t result;
// crypto_init(&result, CRYPTO_BITCOIN);
// crypto_add(&result, &val1, &val2);
// printf("result = %s\n", crypto_to_decimal_str(&result, BTC_DENOM_SATOSHI));
// crypto_clear(&val1);
// crypto_clear(&val2);
// crypto_clear(&result);

#ifndef CRYPTOMATH2_H
#define CRYPTOMATH2_H

#include <gmp.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// TODO: Perhaps this metadata could be stored in a database or file?
// If so, how could we make it type safe?
// Perhaps codegen from a schema?
// Load into a hash table?
// Enumeration of supported cryptocurrencies
typedef enum {
    CRYPTO_BITCOIN,
    CRYPTO_ETHEREUM,
    CRYPTO_BINANCE_COIN,
    CRYPTO_SOLANA,
    CRYPTO_XRP,
    CRYPTO_CARDANO,
    CRYPTO_AVALANCHE,
    CRYPTO_DOGECOIN,
    CRYPTO_POLKADOT,
    CRYPTO_POLYGON,
    CRYPTO_USDC,
    CRYPTO_USDT,
    CRYPTO_FLARE,
    CRYPTO_SONGBIRD,
    CRYPTO_WFLR,
    CRYPTO_WSGB,
    // Add more cryptocurrencies here as needed
    CRYPTO_COUNT  // Keep this as the last entry
} crypto_type_t;

typedef enum {
    BTC_DENOM_BITCOIN,
    BTC_DENOM_SATOSHI,
    BTC_DENOM_MILLIBIT,
    BTC_DENOM_MICROBIT,
    ETH_DENOM_ETHER,
    ETH_DENOM_GWEI,
    ETH_DENOM_WEI,
    BNB_DENOM_BNB,
    BNB_DENOM_JAGER,
    SOL_DENOM_SOL,
    SOL_DENOM_LAMPORT,
    XRP_DENOM_XRP,
    XRP_DENOM_DROP,
    ADA_DENOM_ADA,
    ADA_DENOM_LOVELACE,
    AVAX_DENOM_AVAX,
    AVAX_DENOM_NAVAX,
    DOGE_DENOM_DOGE,
    DOGE_DENOM_SATOSHI,
    DOT_DENOM_DOT,
    DOT_DENOM_PLANCK,
    MATIC_DENOM_MATIC,
    MATIC_DENOM_WEI,
    USDC_DENOM_USDC,
    USDC_DENOM_MICROUSDC,
    USDT_DENOM_USDT,
    USDT_DENOM_MICROUSDT,
    FLR_DENOM_FLR,
    FLR_DENOM_GWEI,
    FLR_DENOM_WEI,
    SGB_DENOM_SGB,
    SGB_DENOM_GWEI,
    SGB_DENOM_WEI,
    WFLR_DENOM_WFLR,
    WFLR_DENOM_GWEI,
    WFLR_DENOM_WEI,
    WSGB_DENOM_WSGB,
    WSGB_DENOM_GWEI,
    WSGB_DENOM_WEI,
    DENOM_COUNT  // Keep this as the last entry
} crypto_denom_t;

typedef struct {
    crypto_type_t crypto_type;
    mpz_t value;
} crypto_val_t;

typedef struct {
    crypto_type_t crypto_type;   // Type of cryptocurrency
    const char* name;            // Human-readable name of cryptocurrency
    const char* symbol;          // Symbol of cryptocurrency
} crypto_def_t;

typedef struct {
    const char* name;          // Human-readable name of denomination
    const char* symbol;        // Denomination symbol (e.g., "BTC", "SAT")
    crypto_type_t crypto_type; // Type of cryptocurrency
    uint8_t decimals;          // Number of decimal places
} crypto_denom_def_t;

// Array of cryptocurrency definitions
static const crypto_def_t crypto_defs[] = {
    [CRYPTO_BITCOIN] = {
        .name = "Bitcoin",
        .symbol = "BTC"
    },
    [CRYPTO_ETHEREUM] = {
        .name = "Ethereum",
        .symbol = "ETH"
    },
    [CRYPTO_BINANCE_COIN] = {
        .name = "Binance Coin",
        .symbol = "BNB"
    },
    [CRYPTO_SOLANA] = {
        .name = "Solana",
        .symbol = "SOL"
    },
    [CRYPTO_XRP] = {
        .name = "XRP",
        .symbol = "XRP"
    },
    [CRYPTO_CARDANO] = {
        .name = "Cardano",
        .symbol = "ADA"
    },
    [CRYPTO_AVALANCHE] = {
        .name = "Avalanche",
        .symbol = "AVAX"
    },
    [CRYPTO_DOGECOIN] = {
        .name = "Dogecoin",
        .symbol = "DOGE"
    },
    [CRYPTO_POLKADOT] = {
        .name = "Polkadot",
        .symbol = "DOT"
    },
    [CRYPTO_POLYGON] = {
        .name = "Polygon",
        .symbol = "MATIC"
    },
    [CRYPTO_USDC] = {
        .name = "USD Coin",
        .symbol = "USDC"
    },
    [CRYPTO_USDT] = {
        .name = "Tether",
        .symbol = "USDT"
    },
    [CRYPTO_FLARE] = {
        .name = "Flare",
        .symbol = "FLR"
    },
    [CRYPTO_SONGBIRD] = {
        .name = "Songbird",
        .symbol = "SGB"
    },
    [CRYPTO_WFLR] = {
        .name = "Wrapped Flare",
        .symbol = "WFLR"
    },
    [CRYPTO_WSGB] = {
        .name = "Wrapped Songbird",
        .symbol = "WSGB"
    }
};

// Array of cryptocurrency denomination definitions
static const crypto_denom_def_t crypto_denoms[] = {
    [BTC_DENOM_BITCOIN] = {
        .name = "Bitcoin",
        .symbol = "BTC",
        .crypto_type = CRYPTO_BITCOIN,
        .decimals = 8
    },
    [BTC_DENOM_SATOSHI] = {
        .name = "Satoshi",
        .symbol = "SAT",
        .crypto_type = CRYPTO_BITCOIN,
        .decimals = 0
    },
    [BTC_DENOM_MILLIBIT] = {
        .name = "Millibit",
        .symbol = "mBTC",
        .crypto_type = CRYPTO_BITCOIN,
        .decimals = 5
    },
    [BTC_DENOM_MICROBIT] = {
        .name = "Microbit",
        .symbol = "μBTC",
        .crypto_type = CRYPTO_BITCOIN,
        .decimals = 2
    },
    [ETH_DENOM_ETHER] = {
        .name = "Ether",
        .symbol = "ETH",
        .crypto_type = CRYPTO_ETHEREUM,
        .decimals = 18
    },
    [ETH_DENOM_GWEI] = {
        .name = "Gwei",
        .symbol = "GWEI",
        .crypto_type = CRYPTO_ETHEREUM,
        .decimals = 9
    },
    [ETH_DENOM_WEI] = {
        .name = "Wei",
        .symbol = "WEI",
        .crypto_type = CRYPTO_ETHEREUM,
        .decimals = 0
    },
    [BNB_DENOM_BNB] = {
        .name = "Binance Coin",
        .symbol = "BNB",
        .crypto_type = CRYPTO_BINANCE_COIN,
        .decimals = 18
    },
    [BNB_DENOM_JAGER] = {
        .name = "Jager",
        .symbol = "JAGER",
        .crypto_type = CRYPTO_BINANCE_COIN,
        .decimals = 0
    },
    [SOL_DENOM_SOL] = {
        .name = "Solana",
        .symbol = "SOL",
        .crypto_type = CRYPTO_SOLANA,
        .decimals = 9
    },
    [SOL_DENOM_LAMPORT] = {
        .name = "Lamport",
        .symbol = "LAMP",
        .crypto_type = CRYPTO_SOLANA,
        .decimals = 0
    },
    [XRP_DENOM_XRP] = {
        .name = "XRP",
        .symbol = "XRP",
        .crypto_type = CRYPTO_XRP,
        .decimals = 6
    },
    [XRP_DENOM_DROP] = {
        .name = "Drop",
        .symbol = "DROP",
        .crypto_type = CRYPTO_XRP,
        .decimals = 0
    },
    [ADA_DENOM_ADA] = {
        .name = "Cardano",
        .symbol = "ADA",
        .crypto_type = CRYPTO_CARDANO,
        .decimals = 6
    },
    [ADA_DENOM_LOVELACE] = {
        .name = "Lovelace",
        .symbol = "LOVELACE",
        .crypto_type = CRYPTO_CARDANO,
        .decimals = 0
    },
    [AVAX_DENOM_AVAX] = {
        .name = "Avalanche",
        .symbol = "AVAX",
        .crypto_type = CRYPTO_AVALANCHE,
        .decimals = 18
    },
    [AVAX_DENOM_NAVAX] = {
        .name = "nAVAX",
        .symbol = "nAVAX",
        .crypto_type = CRYPTO_AVALANCHE,
        .decimals = 0
    },
    [DOGE_DENOM_DOGE] = {
        .name = "Dogecoin",
        .symbol = "DOGE",
        .crypto_type = CRYPTO_DOGECOIN,
        .decimals = 8
    },
    [DOGE_DENOM_SATOSHI] = {
        .name = "Satoshi",
        .symbol = "SAT",
        .crypto_type = CRYPTO_DOGECOIN,
        .decimals = 0
    },
    [DOT_DENOM_DOT] = {
        .name = "Polkadot",
        .symbol = "DOT",
        .crypto_type = CRYPTO_POLKADOT,
        .decimals = 10
    },
    [DOT_DENOM_PLANCK] = {
        .name = "Planck",
        .symbol = "PLANCK",
        .crypto_type = CRYPTO_POLKADOT,
        .decimals = 0
    },
    [MATIC_DENOM_MATIC] = {
        .name = "Polygon",
        .symbol = "MATIC",
        .crypto_type = CRYPTO_POLYGON,
        .decimals = 18
    },
    [MATIC_DENOM_WEI] = {
        .name = "Wei",
        .symbol = "WEI",
        .crypto_type = CRYPTO_POLYGON,
        .decimals = 0
    },
    [USDC_DENOM_USDC] = {
        .name = "USD Coin",
        .symbol = "USDC",
        .crypto_type = CRYPTO_USDC,
        .decimals = 6
    },
    [USDC_DENOM_MICROUSDC] = {
        .name = "Micro USD Coin",
        .symbol = "μUSDC",
        .crypto_type = CRYPTO_USDC,
        .decimals = 0
    },
    [USDT_DENOM_USDT] = {
        .name = "Tether",
        .symbol = "USDT",
        .crypto_type = CRYPTO_USDT,
        .decimals = 6
    },
    [USDT_DENOM_MICROUSDT] = {
        .name = "Micro Tether",
        .symbol = "μUSDT",
        .crypto_type = CRYPTO_USDT,
        .decimals = 0
    },
    [FLR_DENOM_FLR] = {
        .name = "Flare",
        .symbol = "FLR",
        .crypto_type = CRYPTO_FLARE,
        .decimals = 18
    },
    [FLR_DENOM_GWEI] = {
        .name = "Gwei",
        .symbol = "GWEI",
        .crypto_type = CRYPTO_FLARE,
        .decimals = 9
    },
    [FLR_DENOM_WEI] = {
        .name = "Wei",
        .symbol = "WEI",
        .crypto_type = CRYPTO_FLARE,
        .decimals = 0
    },
    [SGB_DENOM_SGB] = {
        .name = "Songbird",
        .symbol = "SGB",
        .crypto_type = CRYPTO_SONGBIRD,
        .decimals = 18
    },
    [SGB_DENOM_GWEI] = {
        .name = "Gwei",
        .symbol = "GWEI",
        .crypto_type = CRYPTO_SONGBIRD,
        .decimals = 9
    },
    [SGB_DENOM_WEI] = {
        .name = "Wei",
        .symbol = "WEI",
        .crypto_type = CRYPTO_SONGBIRD,
        .decimals = 0
    },
    [WFLR_DENOM_WFLR] = {
        .name = "Wrapped Flare",
        .symbol = "WFLR",
        .crypto_type = CRYPTO_WFLR,
        .decimals = 18
    },
    [WFLR_DENOM_GWEI] = {
        .name = "Gwei",
        .symbol = "GWEI",
        .crypto_type = CRYPTO_WFLR,
        .decimals = 9
    },
    [WFLR_DENOM_WEI] = {
        .name = "Wei",
        .symbol = "WEI",
        .crypto_type = CRYPTO_WFLR,
        .decimals = 0
    },
    [WSGB_DENOM_WSGB] = {
        .name = "Wrapped Songbird",
        .symbol = "WSGB",
        .crypto_type = CRYPTO_WSGB,
        .decimals = 18
    },
    [WSGB_DENOM_GWEI] = {
        .name = "Gwei",
        .symbol = "GWEI",
        .crypto_type = CRYPTO_WSGB,
        .decimals = 9
    },
    [WSGB_DENOM_WEI] = {
        .name = "Wei",
        .symbol = "WEI",
        .crypto_type = CRYPTO_WSGB,
        .decimals = 0
    }
};

int crypto_is_valid_type(crypto_type_t type);
int crypto_is_valid_denom(crypto_denom_t denom);
void crypto_init(crypto_val_t* val, crypto_type_t type);
void crypto_clear(crypto_val_t* val);
void crypto_set_from_decimal(crypto_val_t* val, crypto_denom_t denom, const char* decimal_str);
char* crypto_to_decimal_str(crypto_val_t* val, crypto_denom_t denom);
void crypto_add(crypto_val_t* r, const crypto_val_t* a, const crypto_val_t* b);
void crypto_sub(crypto_val_t* r, const crypto_val_t* a, const crypto_val_t* b);
void crypto_mul(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b);
void crypto_div_truncate(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b);
void crypto_div_floor(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b);
void crypto_div_ceil(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b);
int crypto_cmp(const crypto_val_t* a, const crypto_val_t* b);
int crypto_gt_zero(const crypto_val_t* a);
int crypto_lt_zero(const crypto_val_t* a);
int crypto_eq_zero(const crypto_val_t* a);
crypto_denom_t crypto_get_denom_for_symbol(crypto_type_t type, const char* symbol);
crypto_type_t crypto_get_type_for_symbol(const char* symbol);
bool crypto_is_valid_decimal(const char* str);
uint8_t crypto_scale_by_precision(const char* str, mpz_t* result);
bool crypto_has_nonzero_fraction(const char* str);

// Implementation section
#ifdef CRYPTOMATH2_IMPLEMENTATION

// Note: We assume that decimals will not exceed 18 places.
// No token I can find has more than 18 decimal places.
// If you need more, you can change the type of denominator to a mpz_t.
uint64_t power(uint64_t base, uint64_t exp)
{
    uint64_t result = 1;
    while(exp) 
    {   
        result = result * base; 
        exp--; 
    }
    return result;
}

int crypto_is_valid_type(crypto_type_t type) {
    return (type >= 0 && type < CRYPTO_COUNT);
}

int crypto_is_valid_denom(crypto_denom_t denom) {
    return (denom >= 0 && denom < DENOM_COUNT);
}

void crypto_init(crypto_val_t* val, crypto_type_t type) {
    assert(val != NULL);
    assert(crypto_is_valid_type(type));
    val->crypto_type = type;
    mpz_init(val->value);
}

void crypto_clear(crypto_val_t* val) {
    assert(val != NULL);
    mpz_clear(val->value);
}

// Set the value of a crypto_val_t from a decimal string.
// Note that val will be stored in the smallest unit of the crypto type.
// For example, if the decimal string is "1.23456789" and the denom is BTC_DENOM_BITCOIN,
// val will be set to 123456789.
void crypto_set_from_decimal(crypto_val_t* val, crypto_denom_t denom, const char* decimal_str) {
    assert(val != NULL);
    assert(crypto_is_valid_denom(denom));
    assert(decimal_str != NULL);
    assert(val->crypto_type == crypto_denoms[denom].crypto_type);

    // 1. Truncate spaces from decimal_str
    while (*decimal_str == ' ') {
        decimal_str++;
    }

    // 2. Check for optional sign
    int sign = 1;
    if (*decimal_str == '-') {
        sign = -1;
        decimal_str++;
    } else if (*decimal_str == '+') {
        decimal_str++;
    }
    
    // 3. Parse the decimal string
    const char *dot = strchr(decimal_str, '.');
    if (dot == NULL) {
        // No decimal point, just set the value
        mpz_set_str(val->value, decimal_str, 10);
        // Scale the whole number by the number of decimal places
        // TODO: Pre-calculate the power of 10 for the denom
        mpz_mul_ui(val->value, val->value, power(10, crypto_denoms[denom].decimals));
    } else {
        // Parse whole number and fraction separately
        mpz_t whole_part;
        mpz_t fraction_part;
        mpz_init(whole_part);
        mpz_init(fraction_part);
        
        // Parse the whole number part
        char* whole_str = strdup(decimal_str);
        whole_str[dot - decimal_str] = '\0';
        mpz_set_str(whole_part, whole_str, 10);
        // TODO: Pre-calculate the power of 10 for the denom
        mpz_mul_ui(whole_part, whole_part, power(10, crypto_denoms[denom].decimals));
        free(whole_str);

        // Parse the fraction part up to the last expected digit for the denom
        char* fraction_str = malloc(crypto_denoms[denom].decimals + 1);
        // Pad fraction_str with zeros
        memset(fraction_str, '0', crypto_denoms[denom].decimals);
        // Null-terminate the string
        fraction_str[crypto_denoms[denom].decimals] = '\0';
        // Copy the fraction part into the string
        memcpy(fraction_str, dot + 1, 
            strlen(dot + 1) < crypto_denoms[denom].decimals ? strlen(dot + 1) : crypto_denoms[denom].decimals);
        mpz_set_str(fraction_part, fraction_str, 10);
        free(fraction_str);

        // Add the whole and fraction parts
        mpz_add(val->value, whole_part, fraction_part);
        mpz_clear(whole_part);
        mpz_clear(fraction_part);
    }

    // 4. Apply sign
    mpz_mul_si(val->value, val->value, sign);
}

char* crypto_to_decimal_str(crypto_val_t* val, crypto_denom_t denom) {
    assert(val != NULL);
    assert(crypto_is_valid_denom(denom));
    assert(val->crypto_type == crypto_denoms[denom].crypto_type);

    // Determine the sign
    int sign = mpz_sgn(val->value);

    // Determine the whole part scaled to the denom
    mpz_t whole_part;
    mpz_init(whole_part);
    mpz_t fraction_part;
    mpz_init(fraction_part);
    mpz_tdiv_qr_ui(whole_part, fraction_part, val->value, power(10, crypto_denoms[denom].decimals));

    // Convert the whole and fraction parts to strings
    mpz_abs(whole_part, whole_part);
    mpz_abs(fraction_part, fraction_part);
    char* whole_str = mpz_get_str(NULL, 10, whole_part);
    char* fraction_str = mpz_get_str(NULL, 10, fraction_part);

    // Format the whole and fraction parts
    char* formatted_str = malloc(strlen(whole_str) + strlen(fraction_str) + (sign == -1 ? 2 : 1));
    // Add the sign if it's negative; otherwise, just copy the whole_str
    if (sign == -1) {
        formatted_str[0] = '-';
        strcpy(formatted_str + 1, whole_str);
    } else {
        strcpy(formatted_str, whole_str);
    }
    // If the fraction part is not zero, add the decimal point and pad the fraction part with leading zeros if needed
    if (mpz_cmp_ui(fraction_part, 0) != 0) {
        strcat(formatted_str, ".");
        // Pad the fraction part with leading zeros if not enough digits
        if (strlen(fraction_str) < crypto_denoms[denom].decimals) {
            formatted_str = realloc(formatted_str, strlen(formatted_str) + crypto_denoms[denom].decimals - strlen(fraction_str) + 1);
            memset(formatted_str + strlen(formatted_str), '0', crypto_denoms[denom].decimals - strlen(fraction_str));
            strcat(formatted_str, fraction_str);
        } else {
            strcat(formatted_str, fraction_str);
        }
    }

    // Free the temporary strings
    free(whole_str);
    free(fraction_str);
    mpz_clear(whole_part);
    mpz_clear(fraction_part);
    return formatted_str;
}

void crypto_add(crypto_val_t* r, const crypto_val_t* a, const crypto_val_t* b) {
    assert(r != NULL);
    assert(a != NULL);
    assert(b != NULL);
    assert(a->crypto_type == b->crypto_type);
    assert(r->crypto_type == a->crypto_type);
    mpz_add(r->value, a->value, b->value);
}

void crypto_sub(crypto_val_t* r, const crypto_val_t* a, const crypto_val_t* b) {
    assert(r != NULL);
    assert(a != NULL);
    assert(b != NULL);
    assert(a->crypto_type == b->crypto_type);
    assert(r->crypto_type == a->crypto_type);
    mpz_sub(r->value, a->value, b->value);
}

void crypto_mul(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b) {
    assert(r != NULL);
    assert(a != NULL);
    assert(b != NULL);
    assert(r->crypto_type == a->crypto_type);
    mpz_mul(r->value, a->value, *b);
}

void crypto_div_truncate(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b) {
    assert(r != NULL);
    assert(a != NULL);
    assert(b != NULL);
    assert(r->crypto_type == a->crypto_type);
    mpz_tdiv_q(r->value, a->value, *b);
}

void crypto_div_floor(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b) {
    assert(r != NULL);
    assert(a != NULL);
    assert(b != NULL);
    assert(r->crypto_type == a->crypto_type);
    mpz_fdiv_q(r->value, a->value, *b);
}

void crypto_div_ceil(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b) {
    assert(r != NULL);
    assert(a != NULL);
    assert(b != NULL);
    assert(r->crypto_type == a->crypto_type);
    mpz_cdiv_q(r->value, a->value, *b);
}

int crypto_cmp(const crypto_val_t* a, const crypto_val_t* b) {
    assert(a != NULL);
    assert(b != NULL);
    assert(a->crypto_type == b->crypto_type);
    return mpz_cmp(a->value, b->value);
}

int crypto_gt_zero(const crypto_val_t* a) {
    assert(a != NULL);
    return mpz_cmp_ui(a->value, 0) > 0;
}

int crypto_lt_zero(const crypto_val_t* a) {
    assert(a != NULL);
    return mpz_cmp_ui(a->value, 0) < 0;
}

int crypto_eq_zero(const crypto_val_t* a) {
    assert(a != NULL);
    return mpz_cmp_ui(a->value, 0) == 0;
}

// Get the denom for a given symbol.
// Returns DENOM_COUNT if the symbol is not found.
crypto_denom_t crypto_get_denom_for_symbol(crypto_type_t type, const char* symbol) {
    for (int i = 0; i < DENOM_COUNT; i++) {
        if (crypto_denoms[i].crypto_type == type && strcmp(symbol, crypto_denoms[i].symbol) == 0) {
            return i;
        }
    }
    return DENOM_COUNT;
}

bool crypto_is_valid_decimal(const char* str) {
    if (!str) return false;
    
    // Skip leading whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    // Check for empty string after whitespace
    if (*str == '\0') return false;
    
    // Check for optional sign
    if (*str == '+' || *str == '-') {
        str++;
    }
    
    // Must have at least one digit or decimal point
    if (!isdigit(*str) && *str != '.') return false;
    
    bool has_decimal = *str == '.' ? true : false;
    bool has_digit = isdigit(*str) ? true : false;
    
    str++;
    
    // Process the rest of the string
    while (*str != '\0') {
        if (*str == '.') {
            if (has_decimal) return false; // More than one decimal point
            has_decimal = true;
        } else if (isdigit(*str)) {
            has_digit = true;
        } else if (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
            // Skip trailing whitespace
            str++;
            while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
                str++;
            }
            // If we hit non-whitespace after trailing whitespace, invalid
            if (*str != '\0') return false;
            break;
        } else {
            return false; // Invalid character
        }
        str++;
    }
    
    return has_digit; // Must have at least one digit
}

bool crypto_has_nonzero_fraction(const char* str) {
    if (!str) return false;
    
    // Skip leading whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    // Skip optional sign
    if (*str == '+' || *str == '-') {
        str++;
    }
    
    // Find the decimal point
    const char* decimal_point = strchr(str, '.');
    if (!decimal_point) {
        return false; // No decimal point means no fraction
    }
    
    // Check if there are any non-zero digits after the decimal point
    decimal_point++; // Move past the decimal point
    while (*decimal_point != '\0' && *decimal_point != ' ' && *decimal_point != '\t' && *decimal_point != '\n' && *decimal_point != '\r') {
        if (*decimal_point != '0') {
            return true; // Found a non-zero digit in the fraction
        }
        decimal_point++;
    }
    
    return false; // All digits after decimal point were zero
}

// Take an assumed valid decimal string, determine the precision past the decimal point,
// multiple the whole number by 10^precision, add the fraction to the whole number,
// and return the precision. The result will be passed in as a pointer to mpz_t.
uint8_t crypto_scale_by_precision(const char* str, mpz_t* result) {
    assert(str != NULL);
    assert(result != NULL);

    // Get the whole number part
    mpz_set_str(*result, str, 10);

    // Find the decimal point
    const char* decimal_point = strchr(str, '.');
    if (!decimal_point) {
        return 0; // No decimal point means no precision
    }

    // Get the precision
    uint8_t precision = 0;
    while (decimal_point[precision + 1] != '\0') {
        precision++;
    }

    // Get the fraction part into a mpz_t
    mpz_t fraction;
    mpz_init(fraction);
    mpz_set_str(fraction, decimal_point + 1, 10);

    // Only scale the whole number if the fraction is not zero
    if (mpz_cmp_ui(fraction, 0) != 0) {
        mpz_t scale;
        mpz_init(scale);
        mpz_ui_pow_ui(scale, 10, precision);
        // Scale the whole number by the precision; assume the precision can be large
        mpz_mul(*result, *result, scale);
        // Add the fraction to the whole number
        mpz_add(*result, *result, fraction);
        // Clear the fraction
        mpz_clear(fraction);
        mpz_clear(scale);
        // Return the precision
        return precision;
    } else {
        // Clear the fraction
        mpz_clear(fraction);
        return 0;
    }
}

// Get the type for a given symbol.
// Returns CRYPTO_COUNT if the symbol is not found.
crypto_type_t crypto_get_type_for_symbol(const char* symbol) {
    for (int i = 0; i < CRYPTO_COUNT; i++) {
        if (strcmp(symbol, crypto_defs[i].symbol) == 0) {
            return i;
        }
    }
    return CRYPTO_COUNT;
}

#endif // CRYPTOMATH2_IMPLEMENTATION

#endif // CRYPTOMATH2_H 