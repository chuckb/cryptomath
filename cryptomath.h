#ifndef CRYPTOMATH_H
#define CRYPTOMATH_H

#include <gmp.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

// Enumeration of supported cryptocurrencies
typedef enum {
    CRYPTO_BITCOIN,
    CRYPTO_ETHEREUM,
    CRYPTO_TETHER,
    CRYPTO_BNB,
    CRYPTO_SOLANA,
    CRYPTO_XRP,
    CRYPTO_USDC,
    CRYPTO_CARDANO,
    CRYPTO_AVALANCHE,
    CRYPTO_DOGECOIN,
    // Add more cryptocurrencies here as needed
    CRYPTO_COUNT  // Keep this as the last entry
} crypto_type_t;

// Enumeration of Bitcoin denominations
typedef enum {
    BTC_UNIT_BITCOIN,
    BTC_UNIT_SATOSHI,
    BTC_UNIT_MILLIBIT,
    BTC_UNIT_MICROBIT,
    BTC_UNIT_COUNT  // Keep this as the last entry
} btc_unit_t;

// Enumeration of Ethereum denominations
typedef enum {
    ETH_UNIT_ETHER,
    ETH_UNIT_GWEI,
    ETH_UNIT_WEI,
    ETH_UNIT_COUNT  // Keep this as the last entry
} eth_unit_t;

// Structure to define a cryptocurrency unit
typedef struct {
    const char* name;      // Human-readable name
    const char* symbol;    // Symbol (e.g., "BTC", "SAT")
    uint64_t denominator;  // Denominator relative to base unit
    uint8_t decimals;      // Number of decimal places
} crypto_unit_t;

// Structure to hold a cryptocurrency amount with arbitrary precision
typedef struct {
    mpq_t value;           // GMP rational number to store the value
    crypto_type_t type;    // Type of cryptocurrency
    uint8_t unit;          // Unit of the amount
    const crypto_unit_t* unit_info;  // Pointer to unit information
} crypto_amount_t;

// Bitcoin unit definitions
static const crypto_unit_t bitcoin_units[] = {
    [BTC_UNIT_BITCOIN] = {
        .name = "Bitcoin",
        .symbol = "BTC",
        .denominator = 1ULL,
        .decimals = 8
    },
    [BTC_UNIT_SATOSHI] = {
        .name = "Satoshi",
        .symbol = "SAT",
        .denominator = 100000000ULL,
        .decimals = 0
    },
    [BTC_UNIT_MILLIBIT] = {
        .name = "Millibit",
        .symbol = "mBTC",
        .denominator = 1000ULL,
        .decimals = 5
    },
    [BTC_UNIT_MICROBIT] = {
        .name = "Microbit",
        .symbol = "μBTC",
        .denominator = 1000000ULL,
        .decimals = 2
    }
};

// Ethereum unit definitions
static const crypto_unit_t ethereum_units[] = {
    [ETH_UNIT_ETHER] = {
        .name = "Ether",
        .symbol = "ETH",
        .denominator = 1ULL,
        .decimals = 18
    },
    [ETH_UNIT_GWEI] = {
        .name = "Gwei",
        .symbol = "GWEI",
        .denominator = 1000000000ULL,
        .decimals = 9
    },
    [ETH_UNIT_WEI] = {
        .name = "Wei",
        .symbol = "WEI",
        .denominator = 1000000000000000000ULL,
        .decimals = 0
    }
};

// Tether unit definitions
static const crypto_unit_t tether_units[] = {
    [USDT_UNIT_USDT] = {
        .name = "Tether",
        .symbol = "USDT",
        .denominator = 1ULL,
        .decimals = 6
    },
    [USDT_UNIT_MICROUSDT] = {
        .name = "Microtether",
        .symbol = "μUSDT",
        .denominator = 1000000ULL,
        .decimals = 0
    }
};

// BNB unit definitions
static const crypto_unit_t bnb_units[] = {
    [BNB_UNIT_BNB] = {
        .name = "BNB",
        .symbol = "BNB",
        .denominator = 1ULL,
        .decimals = 18
    },
    [BNB_UNIT_GWEI] = {
        .name = "Gwei",
        .symbol = "GWEI",
        .denominator = 1000000000ULL,
        .decimals = 9
    },
    [BNB_UNIT_WEI] = {
        .name = "Wei",
        .symbol = "WEI",
        .denominator = 1000000000000000000ULL,
        .decimals = 0
    }
};

// Solana unit definitions
static const crypto_unit_t solana_units[] = {
    [SOL_UNIT_SOL] = {
        .name = "Solana",
        .symbol = "SOL",
        .denominator = 1ULL,
        .decimals = 9
    },
    [SOL_UNIT_LAMPORT] = {
        .name = "Lamport",
        .symbol = "LAMP",
        .denominator = 1000000000ULL,
        .decimals = 0
    }
};

// XRP unit definitions
static const crypto_unit_t xrp_units[] = {
    [XRP_UNIT_XRP] = {
        .name = "XRP",
        .symbol = "XRP",
        .denominator = 1ULL,
        .decimals = 6
    },
    [XRP_UNIT_DROP] = {
        .name = "Drop",
        .symbol = "DROP",
        .denominator = 1000000ULL,
        .decimals = 0
    }
};

// USDC unit definitions
static const crypto_unit_t usdc_units[] = {
    [USDC_UNIT_USDC] = {
        .name = "USDC",
        .symbol = "USDC",
        .denominator = 1ULL,
        .decimals = 6
    },
    [USDC_UNIT_MICROUSDC] = {
        .name = "MicroUSDC",
        .symbol = "μUSDC",
        .denominator = 1000000ULL,
        .decimals = 0
    }
};

// Cardano unit definitions
static const crypto_unit_t cardano_units[] = {
    [ADA_UNIT_ADA] = {
        .name = "Cardano",
        .symbol = "ADA",
        .denominator = 1ULL,
        .decimals = 6
    },
    [ADA_UNIT_LOVELACE] = {
        .name = "Lovelace",
        .symbol = "LOVELACE",
        .denominator = 1000000ULL,
        .decimals = 0
    }
};

// Avalanche unit definitions
static const crypto_unit_t avalanche_units[] = {
    [AVAX_UNIT_AVAX] = {
        .name = "Avalanche",
        .symbol = "AVAX",
        .denominator = 1ULL,
        .decimals = 18
    },
    [AVAX_UNIT_GWEI] = {
        .name = "Gwei",
        .symbol = "GWEI",
        .denominator = 1000000000ULL,
        .decimals = 9
    },
    [AVAX_UNIT_WEI] = {
        .name = "Wei",
        .symbol = "WEI",
        .denominator = 1000000000000000000ULL,
        .decimals = 0
    }
};

// Dogecoin unit definitions
static const crypto_unit_t dogecoin_units[] = {
    [DOGE_UNIT_DOGE] = {
        .name = "Dogecoin",
        .symbol = "DOGE",
        .denominator = 1ULL,
        .decimals = 8
    },
    [DOGE_UNIT_SATOSHI] = {
        .name = "Satoshi",
        .symbol = "SAT",
        .denominator = 100000000ULL,
        .decimals = 0
    }
};

// Function declarations
int crypto_is_valid_unit(crypto_type_t type, uint8_t unit);
int crypto_is_valid_amount(const crypto_amount_t* amount);
void crypto_init(crypto_amount_t* amount, crypto_type_t type, uint8_t unit);
void crypto_clear(crypto_amount_t* amount);
void crypto_init_bitcoin(crypto_amount_t* amount, btc_unit_t unit);
void crypto_init_ethereum(crypto_amount_t* amount, eth_unit_t unit);
const crypto_unit_t* crypto_get_unit_info(crypto_type_t type, uint8_t unit);
void crypto_convert_to_unit(const crypto_amount_t* amount, crypto_amount_t* result, uint8_t target_unit);
void crypto_print_amount(const crypto_amount_t* amount);

// New function declarations for type-safe value setting
void crypto_set_value(crypto_amount_t* amount, crypto_type_t type, uint8_t unit, uint64_t whole, uint64_t fraction);
void crypto_set_value_from_decimal(crypto_amount_t* amount, crypto_type_t type, uint8_t unit, const char* decimal_str);

// Arithmetic operation declarations
void crypto_add(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result);
void crypto_sub(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result);
void crypto_add_inplace(crypto_amount_t* a, const crypto_amount_t* b);
void crypto_sub_inplace(crypto_amount_t* a, const crypto_amount_t* b);
int crypto_cmp(const crypto_amount_t* a, const crypto_amount_t* b);

// Multiplication and division operations
void crypto_mul(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result);
void crypto_div(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result);
void crypto_mul_inplace(crypto_amount_t* a, const crypto_amount_t* b);
void crypto_div_inplace(crypto_amount_t* a, const crypto_amount_t* b);

// Implementation section
#ifdef CRYPTOMATH_IMPLEMENTATION

int crypto_is_valid_unit(crypto_type_t type, uint8_t unit) {
    switch (type) {
        case CRYPTO_BITCOIN:
            return (unit < BTC_UNIT_COUNT);
        case CRYPTO_ETHEREUM:
            return (unit < ETH_UNIT_COUNT);
        case CRYPTO_TETHER:
            return (unit < USDT_UNIT_COUNT);
        case CRYPTO_BNB:
            return (unit < BNB_UNIT_COUNT);
        case CRYPTO_SOLANA:
            return (unit < SOL_UNIT_COUNT);
        case CRYPTO_XRP:
            return (unit < XRP_UNIT_COUNT);
        case CRYPTO_USDC:
            return (unit < USDC_UNIT_COUNT);
        case CRYPTO_CARDANO:
            return (unit < ADA_UNIT_COUNT);
        case CRYPTO_AVALANCHE:
            return (unit < AVAX_UNIT_COUNT);
        case CRYPTO_DOGECOIN:
            return (unit < DOGE_UNIT_COUNT);
        default:
            return 0;
    }
}

int crypto_is_valid_amount(const crypto_amount_t* amount) {
    if (!amount || !amount->unit_info) {
        return 0;
    }
    return crypto_is_valid_unit(amount->type, amount->unit);
}

void crypto_init(crypto_amount_t* amount, crypto_type_t type, uint8_t unit) {
    assert(amount != NULL);
    assert(crypto_is_valid_unit(type, unit));
    
    mpq_init(amount->value);
    amount->type = type;
    amount->unit = unit;
    switch (type) {
        case CRYPTO_BITCOIN:
            amount->unit_info = &bitcoin_units[unit];
            break;
        case CRYPTO_ETHEREUM:
            amount->unit_info = &ethereum_units[unit];
            break;
        case CRYPTO_TETHER:
            amount->unit_info = &tether_units[unit];
            break;
        case CRYPTO_BNB:
            amount->unit_info = &bnb_units[unit];
            break;
        case CRYPTO_SOLANA:
            amount->unit_info = &solana_units[unit];
            break;
        case CRYPTO_XRP:
            amount->unit_info = &xrp_units[unit];
            break;
        case CRYPTO_USDC:
            amount->unit_info = &usdc_units[unit];
            break;
        case CRYPTO_CARDANO:
            amount->unit_info = &cardano_units[unit];
            break;
        case CRYPTO_AVALANCHE:
            amount->unit_info = &avalanche_units[unit];
            break;
        case CRYPTO_DOGECOIN:
            amount->unit_info = &dogecoin_units[unit];
            break;
        default:
            assert(0);  // Invalid crypto type
    }
}

void crypto_clear(crypto_amount_t* amount) {
    assert(amount != NULL);
    mpq_clear(amount->value);
}

void crypto_init_bitcoin(crypto_amount_t* amount, btc_unit_t unit) {
    assert(amount != NULL);
    assert(crypto_is_valid_unit(CRYPTO_BITCOIN, unit));
    crypto_init(amount, CRYPTO_BITCOIN, unit);
}

void crypto_init_ethereum(crypto_amount_t* amount, eth_unit_t unit) {
    assert(amount != NULL);
    assert(crypto_is_valid_unit(CRYPTO_ETHEREUM, unit));
    crypto_init(amount, CRYPTO_ETHEREUM, unit);
}

const crypto_unit_t* crypto_get_unit_info(crypto_type_t type, uint8_t unit) {
    assert(crypto_is_valid_unit(type, unit));
    switch (type) {
        case CRYPTO_BITCOIN:
            return &bitcoin_units[unit];
        case CRYPTO_ETHEREUM:
            return &ethereum_units[unit];
        case CRYPTO_TETHER:
            return &tether_units[unit];
        case CRYPTO_BNB:
            return &bnb_units[unit];
        case CRYPTO_SOLANA:
            return &solana_units[unit];
        case CRYPTO_XRP:
            return &xrp_units[unit];
        case CRYPTO_USDC:
            return &usdc_units[unit];
        case CRYPTO_CARDANO:
            return &cardano_units[unit];
        case CRYPTO_AVALANCHE:
            return &avalanche_units[unit];
        case CRYPTO_DOGECOIN:
            return &dogecoin_units[unit];
        default:
            return NULL;
    }
}

void crypto_convert_to_unit(const crypto_amount_t* amount, crypto_amount_t* result, uint8_t target_unit) {
    assert(amount != NULL);
    assert(result != NULL);
    assert(crypto_is_valid_amount(amount));
    assert(crypto_is_valid_unit(amount->type, target_unit));
    
    crypto_init(result, amount->type, target_unit);
    
    // Calculate conversion factor
    mpq_t conversion_factor;
    mpq_init(conversion_factor);
    
    // Convert from source unit to base unit
    mpq_set_ui(conversion_factor, amount->unit_info->denominator, 1);
    mpq_mul(result->value, amount->value, conversion_factor);
    
    // Convert from base unit to target unit
    const crypto_unit_t* target_unit_info = crypto_get_unit_info(amount->type, target_unit);
    mpq_set_ui(conversion_factor, 1, target_unit_info->denominator);
    mpq_mul(result->value, result->value, conversion_factor);
    
    mpq_clear(conversion_factor);
}

void crypto_print_amount(const crypto_amount_t* amount) {
    assert(amount != NULL);
    assert(crypto_is_valid_amount(amount));

    // Convert to string with full precision
    char* str = mpq_get_str(NULL, 10, amount->value);
    if (!str) {
        return;
    }

    // Find the decimal point
    char* decimal_point = strchr(str, '.');
    if (!decimal_point) {
        // If no decimal point, add it
        printf("%s.0 %s\n", str, amount->unit_info->symbol);
    } else {
        // Print with proper decimal places
        printf("%.*s %s\n", 
               (int)(strlen(decimal_point + 1) + 1),  // +1 for the decimal point
               str,
               amount->unit_info->symbol);
    }

    free(str);
}

void crypto_set_value(crypto_amount_t* amount, crypto_type_t type, uint8_t unit, uint64_t whole, uint64_t fraction) {
    assert(amount != NULL);
    assert(crypto_is_valid_unit(type, unit));
    
    // Initialize the amount with the correct type and unit
    crypto_init(amount, type, unit);
    
    // Get the unit info for the target unit
    const crypto_unit_t* unit_info = crypto_get_unit_info(type, unit);
    
    // Set the whole part
    mpq_set_ui(amount->value, whole, 1);
    
    // Add the fraction part if provided
    if (fraction > 0) {
        mpq_t fraction_part;
        mpq_init(fraction_part);
        
        // Calculate the denominator for the fraction based on the unit's decimals
        uint64_t denominator = 1;
        for (uint8_t i = 0; i < unit_info->decimals; i++) {
            denominator *= 10;
        }
        
        mpq_set_ui(fraction_part, fraction, denominator);
        mpq_add(amount->value, amount->value, fraction_part);
        mpq_clear(fraction_part);
    }
    
    // Convert to base unit
    mpq_t conversion_factor;
    mpq_init(conversion_factor);
    mpq_set_ui(conversion_factor, 1, unit_info->denominator);
    mpq_mul(amount->value, amount->value, conversion_factor);
    mpq_clear(conversion_factor);
}

void crypto_set_value_from_decimal(crypto_amount_t* amount, crypto_type_t type, uint8_t unit, const char* decimal_str) {
    assert(amount != NULL);
    assert(decimal_str != NULL);
    assert(crypto_is_valid_unit(type, unit));
    
    // Initialize the amount structure
    crypto_init(amount, type, unit);
    
    // Parse the decimal string
    const char* p = decimal_str;
    int is_negative = 0;
    
    // Handle sign
    if (*p == '-') {
        is_negative = 1;
        p++;
    }
    
    // Parse whole part
    mpz_t whole;
    mpz_init(whole);
    mpz_set_ui(whole, 0);
    
    while (*p >= '0' && *p <= '9') {
        mpz_mul_ui(whole, whole, 10);
        mpz_add_ui(whole, whole, *p - '0');
        p++;
    }
    
    // Parse fractional part
    mpz_t fraction;
    mpz_init(fraction);
    mpz_set_ui(fraction, 0);
    mpz_t denominator;
    mpz_init(denominator);
    mpz_set_ui(denominator, 1);
    
    if (*p == '.') {
        p++;
        while (*p >= '0' && *p <= '9') {
            mpz_mul_ui(fraction, fraction, 10);
            mpz_add_ui(fraction, fraction, *p - '0');
            mpz_mul_ui(denominator, denominator, 10);
            p++;
        }
    }
    
    // Combine whole and fraction
    mpz_mul(whole, whole, denominator);
    mpz_add(whole, whole, fraction);
    
    // Set the value
    mpq_set_z(amount->value, whole);
    mpq_set_den(amount->value, denominator);
    
    // Apply sign
    if (is_negative) {
        mpq_neg(amount->value, amount->value);
    }
    
    // Clean up
    mpz_clear(whole);
    mpz_clear(fraction);
    mpz_clear(denominator);
    
    // Convert to base unit if needed
    const crypto_unit_t* unit_info = crypto_get_unit_info(type, unit);
    if (unit_info->denominator != 1) {
        mpq_t conversion_factor;
        mpq_init(conversion_factor);
        mpq_set_ui(conversion_factor, 1, unit_info->denominator);
        mpq_mul(amount->value, amount->value, conversion_factor);
        mpq_clear(conversion_factor);
    }
}

void crypto_add(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result) {
    assert(a != NULL && b != NULL && result != NULL);
    assert(crypto_is_valid_amount(a) && crypto_is_valid_amount(b));
    assert(a->type == b->type);  // Can only add same crypto types
    
    // Initialize result with the same type and unit as a
    crypto_init(result, a->type, a->unit);
    
    // Add the values
    mpq_add(result->value, a->value, b->value);
}

void crypto_sub(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result) {
    assert(a != NULL && b != NULL && result != NULL);
    assert(crypto_is_valid_amount(a) && crypto_is_valid_amount(b));
    assert(a->type == b->type);  // Can only subtract same crypto types
    
    // Initialize result with the same type and unit as a
    crypto_init(result, a->type, a->unit);
    
    // Subtract the values
    mpq_sub(result->value, a->value, b->value);
}

void crypto_add_inplace(crypto_amount_t* a, const crypto_amount_t* b) {
    assert(a != NULL && b != NULL);
    assert(crypto_is_valid_amount(a) && crypto_is_valid_amount(b));
    assert(a->type == b->type);
    
    // Create a temporary result to avoid issues when a == b
    mpq_t temp;
    mpq_init(temp);
    // Add the values
    mpq_add(temp, a->value, b->value);
    
    // Copy the result back to a
    mpq_set(a->value, temp);
    
    // Clean up the temporary
    mpq_clear(temp);
}
    
void crypto_sub_inplace(crypto_amount_t* a, const crypto_amount_t* b) {
    assert(a != NULL && b != NULL);
    assert(crypto_is_valid_amount(a) && crypto_is_valid_amount(b));
    assert(a->type == b->type);
    
    // Create a temporary result to avoid issues when a == b
    mpq_t temp;
    mpq_init(temp);
    // Subtract the values
    mpq_sub(temp, a->value, b->value);
    
    // Copy the result back to a
    mpq_set(a->value, temp);
    
    // Clean up the temporary
    mpq_clear(temp);
}

int crypto_cmp(const crypto_amount_t* a, const crypto_amount_t* b) {
    assert(a != NULL && b != NULL);
    assert(crypto_is_valid_amount(a) && crypto_is_valid_amount(b));
    assert(a->type == b->type);  // Can only compare same crypto types
    
    return mpq_cmp(a->value, b->value);
}

void crypto_mul(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result) {
    assert(a != NULL && b != NULL && result != NULL);
    assert(crypto_is_valid_amount(a) && crypto_is_valid_amount(b));
    assert(a->type == b->type);  // Can only multiply same crypto types
    
    // Initialize result with the same type and unit as a
    crypto_init(result, a->type, a->unit);
    
    // Multiply the values
    mpq_mul(result->value, a->value, b->value);
}

void crypto_div(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result) {
    assert(a != NULL && b != NULL && result != NULL);
    assert(crypto_is_valid_amount(a) && crypto_is_valid_amount(b));
    assert(a->type == b->type);  // Can only divide same crypto types
    
    // Initialize result with the same type and unit as a
    crypto_init(result, a->type, a->unit);
    
    // Divide the values
    mpq_div(result->value, a->value, b->value);
}

void crypto_mul_inplace(crypto_amount_t* a, const crypto_amount_t* b) {
    assert(a != NULL && b != NULL);
    assert(crypto_is_valid_amount(a) && crypto_is_valid_amount(b));
    assert(a->type == b->type);
    
    // Create a temporary result to avoid issues when a == b
    mpq_t temp;
    mpq_init(temp);
    // Multiply the values
    mpq_mul(temp, a->value, b->value);
    
    // Copy the result back to a
    mpq_set(a->value, temp);
    
    // Clean up the temporary
    mpq_clear(temp);
}

void crypto_div_inplace(crypto_amount_t* a, const crypto_amount_t* b) {
    assert(a != NULL && b != NULL);
    assert(crypto_is_valid_amount(a) && crypto_is_valid_amount(b));
    assert(a->type == b->type);
    
    // Create a temporary result to avoid issues when a == b
    mpq_t temp;
    mpq_init(temp);
    // Divide the values
    mpq_div(temp, a->value, b->value);
    
    // Copy the result back to a
    mpq_set(a->value, temp);
    
    // Clean up the temporary
    mpq_clear(temp);
}

#endif // CRYPTOMATH_IMPLEMENTATION

#endif // CRYPTOMATH_H 