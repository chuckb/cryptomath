# Cryptomath

A C library for handling cryptocurrency amounts with arbitrary precision, currently supporting Bitcoin and its various denominations.

## Features

- Arbitrary precision arithmetic using GMP (GNU Multiple Precision Arithmetic Library)
- Support for Bitcoin and its denominations:
  - Bitcoin (BTC)
  - Satoshi (SAT)
  - Millibitcoin (mBTC)
  - Microbitcoin (μBTC)
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

// Structure to hold a cryptocurrency amount
typedef struct {
    mpq_t value;           // GMP rational number
    crypto_type_t type;    // Type of cryptocurrency
    btc_unit_t unit;       // Unit of the amount
    const crypto_unit_t* unit_info;  // Pointer to unit information
} crypto_amount_t;
```

### Core Functions

```c
// Initialize a cryptocurrency amount
void crypto_init(crypto_amount_t* amount, crypto_type_t type, btc_unit_t unit);

// Initialize a Bitcoin amount
void crypto_init_bitcoin(crypto_amount_t* amount, btc_unit_t unit);

// Set a value with specified unit
void crypto_set_value(crypto_amount_t* amount, btc_unit_t unit, uint64_t whole, uint64_t fraction);

// Convert amount to different unit
void crypto_convert_to_unit(const crypto_amount_t* amount, crypto_amount_t* result, btc_unit_t target_unit);

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
    crypto_amount_t btc, sat, mbtc, result;
    
    // Initialize amounts
    crypto_init_bitcoin(&btc, BTC_UNIT_BITCOIN);
    crypto_init_bitcoin(&sat, BTC_UNIT_SATOSHI);
    crypto_init_bitcoin(&mbtc, BTC_UNIT_MILLIBIT);
    
    // Set values using decimal strings
    crypto_set_value_from_decimal(&btc, BTC_UNIT_BITCOIN, "1.23456789");    // 1.23456789 BTC
    crypto_set_value_from_decimal(&sat, BTC_UNIT_SATOSHI, "50000000");      // 0.5 BTC in satoshis
    crypto_set_value_from_decimal(&mbtc, BTC_UNIT_MILLIBIT, "100.00000");   // 0.1 BTC in millibits
    
    // Print individual amounts
    crypto_print_amount(&btc);   // Prints: 1.23456789 BTC
    crypto_print_amount(&sat);   // Prints: 50000000 SAT
    crypto_print_amount(&mbtc);  // Prints: 100.00000 mBTC
    
    // Add amounts (automatically handles unit conversion)
    crypto_add(&btc, &sat, &result);
    crypto_print_amount(&result);  // Prints: 1.73456789 BTC
    
    // Subtract amounts
    crypto_sub(&result, &mbtc, &result);
    crypto_print_amount(&result);  // Prints: 1.63456789 BTC
    
    // Convert to different unit
    crypto_convert_to_unit(&result, &sat, BTC_UNIT_SATOSHI);
    crypto_print_amount(&sat);  // Prints: 163456789 SAT
    
    // Cleanup
    crypto_clear(&btc);
    crypto_clear(&sat);
    crypto_clear(&mbtc);
    crypto_clear(&result);
    
    return 0;
}
```

## Unit Support

| Unit      | Symbol | Denominator | Decimals | Example        |
|-----------|--------|-------------|----------|----------------|
| Bitcoin   | BTC    | 1           | 8        | 1.00000000     |
| Satoshi   | SAT    | 100000000   | 0        | 100000000      |
| Millibit  | mBTC   | 1000        | 5        | 1000.00000     |
| Microbit  | μBTC   | 1000000     | 2        | 1000000.00     |

## Testing

The test suite verifies:
- Unit conversions
- Arithmetic operations
- Value setting
- Precision handling
- Invalid unit detection

Run tests with:
```bash
make test
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.