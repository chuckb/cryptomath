# Cryptomath

A C library for handling cryptocurrency amounts with arbitrary precision, currently supporting Bitcoin and Ethereum with their various denominations.

## Features

- Arbitrary precision arithmetic using GMP (GNU Multiple Precision Arithmetic Library)
- Support for multiple cryptocurrencies:
  - Bitcoin and its denominations:
    - Bitcoin (BTC)
    - Satoshi (SAT)
    - Millibitcoin (mBTC)
    - Microbitcoin (μBTC)
  - Ethereum and its denominations:
    - Ether (ETH)
    - Gwei (GWEI)
    - Wei (WEI)
- Type-safe value setting and conversion
- Arithmetic operations between different units
- Precise decimal representation
- Memory-safe operations with proper cleanup

## Requirements

- C99 or later
- GMP library (libgmp)
- Make (for building)

## Installation

1. Install GMP:
   ```bash
   # macOS
   brew install gmp
   
   # Ubuntu/Debian
   sudo apt-get install libgmp-dev
   ```

2. Build the library and tests:
   ```bash
   make
   ```

3. Run the tests:
   ```bash
   make test
   ```

## API

### Types

```c
// Supported cryptocurrencies
typedef enum {
    CRYPTO_BITCOIN,
    CRYPTO_ETHEREUM,
    CRYPTO_COUNT
} crypto_type_t;

// Bitcoin denominations
typedef enum {
    BTC_UNIT_BITCOIN,
    BTC_UNIT_SATOSHI,
    BTC_UNIT_MILLIBIT,
    BTC_UNIT_MICROBIT,
    BTC_UNIT_COUNT
} btc_unit_t;

// Ethereum denominations
typedef enum {
    ETH_UNIT_ETHER,
    ETH_UNIT_GWEI,
    ETH_UNIT_WEI,
    ETH_UNIT_COUNT
} eth_unit_t;

// Structure to hold a cryptocurrency amount
typedef struct {
    mpq_t value;           // GMP rational number
    crypto_type_t type;    // Type of cryptocurrency
    uint8_t unit;          // Unit of the amount
    const crypto_unit_t* unit_info;  // Pointer to unit information
} crypto_amount_t;
```

### Core Functions

```c
// Initialize a cryptocurrency amount
void crypto_init(crypto_amount_t* amount, crypto_type_t type, uint8_t unit);

// Initialize a Bitcoin amount
void crypto_init_bitcoin(crypto_amount_t* amount, btc_unit_t unit);

// Initialize an Ethereum amount
void crypto_init_ethereum(crypto_amount_t* amount, eth_unit_t unit);

// Set a value with specified unit
void crypto_set_value(crypto_amount_t* amount, crypto_type_t type, uint8_t unit, uint64_t whole, uint64_t fraction);

// Set a value from decimal string
void crypto_set_value_from_decimal(crypto_amount_t* amount, crypto_type_t type, uint8_t unit, const char* decimal_str);

// Convert amount to different unit
void crypto_convert_to_unit(const crypto_amount_t* amount, crypto_amount_t* result, uint8_t target_unit);

// Print amount in decimal form
void crypto_print_amount(const crypto_amount_t* amount);

// Arithmetic operations
void crypto_add(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result);
void crypto_sub(const crypto_amount_t* a, const crypto_amount_t* b, crypto_amount_t* result);
int crypto_cmp(const crypto_amount_t* a, const crypto_amount_t* b);

// Cleanup
void crypto_clear(crypto_amount_t* amount);
```

## Usage Example

```c
#include "cryptomath.h"

int main() {
    // Bitcoin example
    crypto_amount_t btc, sat, mbtc, result;
    
    // Initialize Bitcoin amounts
    crypto_init_bitcoin(&btc, BTC_UNIT_BITCOIN);
    crypto_init_bitcoin(&sat, BTC_UNIT_SATOSHI);
    crypto_init_bitcoin(&mbtc, BTC_UNIT_MILLIBIT);
    
    // Set Bitcoin values
    crypto_set_value_from_decimal(&btc, CRYPTO_BITCOIN, BTC_UNIT_BITCOIN, "1.23456789");    // 1.23456789 BTC
    crypto_set_value_from_decimal(&sat, CRYPTO_BITCOIN, BTC_UNIT_SATOSHI, "50000000");      // 0.5 BTC in satoshis
    crypto_set_value_from_decimal(&mbtc, CRYPTO_BITCOIN, BTC_UNIT_MILLIBIT, "100.00000");   // 0.1 BTC in millibits
    
    // Ethereum example
    crypto_amount_t eth, gwei, wei;
    
    // Initialize Ethereum amounts
    crypto_init_ethereum(&eth, ETH_UNIT_ETHER);
    crypto_init_ethereum(&gwei, ETH_UNIT_GWEI);
    crypto_init_ethereum(&wei, ETH_UNIT_WEI);
    
    // Set Ethereum values
    crypto_set_value_from_decimal(&eth, CRYPTO_ETHEREUM, ETH_UNIT_ETHER, "1.234567890123456789");  // 1.234567890123456789 ETH
    crypto_set_value_from_decimal(&gwei, CRYPTO_ETHEREUM, ETH_UNIT_GWEI, "1000000000");            // 1 ETH in gwei
    crypto_set_value_from_decimal(&wei, CRYPTO_ETHEREUM, ETH_UNIT_WEI, "1234567890123456789");     // 1.234567890123456789 ETH in wei
    
    // Print amounts
    crypto_print_amount(&btc);   // Prints: 1.23456789 BTC
    crypto_print_amount(&sat);   // Prints: 50000000 SAT
    crypto_print_amount(&mbtc);  // Prints: 100.00000 mBTC
    crypto_print_amount(&eth);   // Prints: 1.234567890123456789 ETH
    crypto_print_amount(&gwei);  // Prints: 1000000000 GWEI
    crypto_print_amount(&wei);   // Prints: 1234567890123456789 WEI
    
    // Arithmetic operations (within same crypto type)
    crypto_add(&btc, &sat, &result);
    crypto_print_amount(&result);  // Prints: 1.73456789 BTC
    
    crypto_add(&eth, &gwei, &result);
    crypto_print_amount(&result);  // Prints: 2.234567890123456789 ETH
    
    // Cleanup
    crypto_clear(&btc);
    crypto_clear(&sat);
    crypto_clear(&mbtc);
    crypto_clear(&eth);
    crypto_clear(&gwei);
    crypto_clear(&wei);
    crypto_clear(&result);
    
    return 0;
}
```

## Unit Support

### Bitcoin Units
| Unit      | Symbol | Denominator | Decimals | Example        |
|-----------|--------|-------------|----------|----------------|
| Bitcoin   | BTC    | 1           | 8        | 1.00000000     |
| Satoshi   | SAT    | 100000000   | 0        | 100000000      |
| Millibit  | mBTC   | 1000        | 5        | 1000.00000     |
| Microbit  | μBTC   | 1000000     | 2        | 1000000.00     |

### Ethereum Units
| Unit      | Symbol | Denominator | Decimals | Example                    |
|-----------|--------|-------------|----------|----------------------------|
| Ether     | ETH    | 1           | 18       | 1.000000000000000000      |
| Gwei      | GWEI   | 1000000000  | 9        | 1000000000.000000000      |
| Wei       | WEI    | 1000000000000000000 | 0 | 1000000000000000000      |

## Testing

The test suite verifies:
- Unit conversions for both Bitcoin and Ethereum
- Arithmetic operations within each cryptocurrency type
- Value setting and parsing
- Precision handling
- Invalid unit detection
- Type safety between different cryptocurrencies

Run tests with:
```bash
make test
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.