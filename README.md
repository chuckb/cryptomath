# Cryptomath

A C library for handling cryptocurrency amounts with arbitrary precision, supporting multiple cryptocurrencies and their various denominations. Available in two forms:
1. Header-only C library for direct integration
2. SQLite extension for database operations

## Features

- Arbitrary precision arithmetic using GMP (GNU Multiple Precision Arithmetic Library)
- Support for multiple cryptocurrencies:
  - Bitcoin (BTC, SAT, mBTC, μBTC)
  - Ethereum (ETH, GWEI, WEI)
  - Binance Coin (BNB, JAGER)
  - Solana (SOL, LAMP)
  - XRP (XRP, DROP)
  - Cardano (ADA, LOVELACE)
  - Avalanche (AVAX, nAVAX)
  - Dogecoin (DOGE, SAT)
  - Polkadot (DOT, PLANCK)
  - Polygon (MATIC, WEI)
  - USD Coin (USDC, μUSDC)
  - Tether (USDT, μUSDT)
  - Flare (FLR, GWEI, WEI)
  - Songbird (SGB, GWEI, WEI)
  - Wrapped Flare (FLR, GWEI, WEI)
  - Wrapped Songbird (FLR, GWEI, WEI)
- Type-safe value setting and conversion
- Arithmetic operations between different units
- Precise decimal representation
- Memory-safe operations with proper cleanup

## Requirements

- C99 or later
- GMP library (libgmp)
- Make (for building)
- SQLite3 (for SQLite extension)

## Installation

### Installing Dependencies

```bash
# macOS
brew install gmp sqlite3

# Ubuntu/Debian
sudo apt-get install libgmp-dev libsqlite3-dev
```

### Building

The project has two main components that can be built separately:

1. Header-only library:
```bash
# Build just the library
make lib

# Build and run library tests
make test-lib
```

2. SQLite extension:
```bash
# Build the SQLite extension
make sqlite

# Build and run SQLite extension tests
make test-sqlite
```

For debugging:
```bash
# Build with debug symbols and no optimization
make debug

# Run all tests with debug build
make test
```

## Header-only Library Usage

### API

```c
// Supported cryptocurrencies
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
    CRYPTO_COUNT
} crypto_type_t;

// Structure to hold a cryptocurrency amount
typedef struct {
    crypto_type_t crypto_type;
    mpz_t value;
} crypto_val_t;

// Structure defining cryptocurrency denominations
typedef struct {
    const char* name;          // Human-readable name
    const char* symbol;        // Symbol (e.g., "BTC", "SAT")
    crypto_type_t crypto_type; // Type of cryptocurrency
    uint8_t decimals;          // Number of decimal places
} crypto_def_t;
```

### Core Functions

```c
// Initialize a cryptocurrency amount
void crypto_init(crypto_val_t* val, crypto_type_t type);

// Set a value from decimal string
void crypto_set_from_decimal(crypto_val_t* val, crypto_def_t denom, const char* decimal_str);

// Convert amount to decimal string
char* crypto_to_decimal_str(crypto_val_t* val, crypto_def_t denom);

// Set one crypto amount to another
void crypto_set(crypto_val_t* to, const crypto_val_t* from)

// Arithmetic operations
void crypto_add(crypto_val_t* r, const crypto_val_t* a, const crypto_val_t* b);
void crypto_sub(crypto_val_t* r, const crypto_val_t* a, const crypto_val_t* b);
void crypto_mul(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b);
void crypto_div_truncate(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b);
void crypto_div_floor(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b);
void crypto_div_ceil(crypto_val_t* r, const crypto_val_t* a, const mpz_t *b);

// Comparison operations
int crypto_cmp(const crypto_val_t* a, const crypto_val_t* b);
int crypto_gt_zero(const crypto_val_t* a);
int crypto_lt_zero(const crypto_val_t* a);
int crypto_eq_zero(const crypto_val_t* a);

// Cleanup
void crypto_clear(crypto_val_t* val);
```

### Example Usage

```c
#define CRYPTOMATH_IMPLEMENTATION
#include "cryptomath.h"

int main() {
    // Bitcoin example
    crypto_val_t btc, sat, mbtc, result;
    
    // Initialize Bitcoin amounts
    crypto_init(&btc, CRYPTO_BITCOIN);
    crypto_init(&sat, CRYPTO_BITCOIN);
    crypto_init(&mbtc, CRYPTO_BITCOIN);
    
    // Set Bitcoin values
    crypto_set_from_decimal(&btc, BTC_DENOM_BITCOIN, "1.23456789");    // 1.23456789 BTC
    crypto_set_from_decimal(&sat, BTC_DENOM_SATOSHI, "-50000000");     // -0.5 BTC in satoshis
    crypto_set_from_decimal(&mbtc, BTC_DENOM_MILLIBIT, "100.00000");   // 0.1 BTC in millibits
    
    // Ethereum example
    crypto_val_t eth, gwei, wei;
    
    // Initialize Ethereum amounts
    crypto_init(&eth, CRYPTO_ETHEREUM);
    crypto_init(&gwei, CRYPTO_ETHEREUM);
    crypto_init(&wei, CRYPTO_ETHEREUM);
    
    // Set Ethereum values
    crypto_set_from_decimal(&eth, ETH_DENOM_ETHER, "1.234567890123456789");  // 1.234567890123456789 ETH
    crypto_set_from_decimal(&gwei, ETH_DENOM_GWEI, "1000000000");            // 1 ETH in gwei
    crypto_set_from_decimal(&wei, ETH_DENOM_WEI, "1234567890123456789");     // 1.234567890123456789 ETH in wei
    
    // Print amounts
    printf("%s\n", crypto_to_decimal_str(&btc, BTC_DENOM_BITCOIN));   // Prints: 1.23456789
    printf("%s\n", crypto_to_decimal_str(&sat, BTC_DENOM_SATOSHI));   // Prints: -50000000
    printf("%s\n", crypto_to_decimal_str(&mbtc, BTC_DENOM_MILLIBIT)); // Prints: 100.00000
    printf("%s\n", crypto_to_decimal_str(&eth, ETH_DENOM_ETHER));     // Prints: 1.234567890123456789
    printf("%s\n", crypto_to_decimal_str(&gwei, ETH_DENOM_GWEI));     // Prints: 1000000000
    printf("%s\n", crypto_to_decimal_str(&wei, ETH_DENOM_WEI));       // Prints: 1234567890123456789
    
    // Arithmetic operations (within same crypto type)
    crypto_add(&result, &btc, &sat);
    printf("%s\n", crypto_to_decimal_str(&result, BTC_DENOM_BITCOIN));  // Prints: 1.73456789
    
    crypto_add(&result, &eth, &gwei);
    printf("%s\n", crypto_to_decimal_str(&result, ETH_DENOM_ETHER));    // Prints: 2.234567890123456789
    
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

## SQLite Extension Usage

### Available Functions

The SQLite extension provides the following functions:

```sql
-- Scale a given crypto amount
crypto_scale(crypto, from_denom, to_denom, operand) -> TEXT

-- Arithmetic operations
crypto_add(crypto, denomination, operand1, operand2) -> TEXT
crypto_sub(crypto, denomination, operand1, operand2) -> TEXT
crypto_mul(crypto, denomination, operand1, operand2) -> TEXT
crypto_div_trunc(crypto, denomination, operand1, operand2) -> TEXT
crypto_div_floor(crypto, denomination, operand1, operand2) -> TEXT
crypto_div_ceil(crypto, denomination, operand1, operand2) -> TEXT

-- Comparison operations
crypto_cmp(crypto, denomination, operand1, operand2) -> INTEGER
-- Returns:
--   0 if operand1 == operand2
--  -1 if operand1 < operand2
--   1 if operand1 > operand2

-- Aggregate operations
crypto_sum(crypto, operand_denomination, final_denomination, operand) -> TEXT
crypto_max(crypto, operand_denomination, final_denomination, operand) -> TEXT
crypto_min(crypto, operand_denomination, final_denomination, operand) -> TEXT

-- Metadata as virtual tables; list supported crypto types and denominations
crypto_types()
crypto_denoms()
```

### Example Usage

```sql
-- Create a table with decimal columns stored as TEXT
CREATE TABLE transactions (
    id INTEGER PRIMARY KEY,
    amount TEXT,
    fee TEXT
);

-- Insert BITCOIN values as TEXT
INSERT INTO transactions (amount, fee) VALUES
    ('1.23456789', '0.0001');
INSERT INTO transactions (amount, fee) VALUES
    ('9.87654321', '0.0010');

-- Add BITCOIN together in BTC units and scale to SATS
SELECT crypto_scale('BTC', 'BTC', 'SAT', crypto_add('BTC', 'BTC', amount, fee)) as total FROM transactions;

-- Sum BITCOIN across rows and scale output to SATS
SELECT crypto_sum('BTC', 'BTC', 'SAT', amount) FROM transactions;

-- Find maximum and minimum BITCOIN amounts
SELECT crypto_max('BTC', 'BTC', 'SAT', amount) as max_amount,
       crypto_min('BTC', 'BTC', 'SAT', amount) as min_amount
FROM transactions;

-- Compare cryptocurrency amounts
SELECT 
    CASE crypto_cmp('BTC', 'BTC', amount, '1.0')
        WHEN 1 THEN 'Greater than 1 BTC'
        WHEN 0 THEN 'Equal to 1 BTC'
        WHEN -1 THEN 'Less than 1 BTC'
    END as comparison
FROM transactions;

-- Find transactions with fees greater than 0.0005 BTC
SELECT * FROM transactions 
WHERE crypto_cmp('BTC', 'BTC', fee, '0.0005') > 0;
```

### Loading the Extension

```bash
# Load extension in SQLite CLI
sqlite3
.load ./build/crypto_decimal_extension.dylib

# Or in your application
sqlite3_enable_load_extension(db, 1);
sqlite3_load_extension(db, "./build/crypto_decimal_extension.dylib", 0, NULL);
```
Note that you must have a sqlite version installed with extension support enabled for CLI use.
Mac OS does not come with a native build that supports extensions. The homebrew version
DOES have this enabled. You will have to point header directories at that new location. Also note
that sqlite is a cask only install, so you will have to alter your path to pick up the fresher version.

## Unit Support

### Bitcoin Units
| Unit      | Symbol | Decimals | Example        |
|-----------|--------|----------|----------------|
| Bitcoin   | BTC    | 8        | 1.00000000     |
| Satoshi   | SAT    | 0        | 100000000      |
| Millibit  | mBTC   | 5        | 1000.00000     |
| Microbit  | μBTC   | 2        | 1000000.00     |

### Ethereum Units
| Unit      | Symbol | Decimals | Example                    |
|-----------|--------|----------|----------------------------|
| Ether     | ETH    | 18       | 1.000000000000000000      |
| Gwei      | GWEI   | 9        | 1000000000.000000000      |
| Wei       | WEI    | 0        | 1000000000000000000      |

### Other Supported Cryptocurrencies
The library supports many other cryptocurrencies with their respective denominations. Each cryptocurrency has a base unit and a smallest unit (with 0 decimal places).

To add additional currency types and denominations, see the cryptomath.h file and the typedefs
and static arrays at the top of the file.

## Testing

The test suite verifies:
- Unit conversions for all supported cryptocurrencies
- Arithmetic operations within each cryptocurrency type
- Value setting and parsing
- Precision handling
- Invalid unit detection
- Type safety between different cryptocurrencies

Run tests with:
```bash
# Test header-only library
make test-lib

# Test SQLite extension
make test-sqlite

# Test everything
make test
```

## Distribution

```bash
# Tarball for Mac OS
make dist

# Tarball for Ubuntu 24.04
make dist-linux LINUX_DISTRO=ubuntu:24.04 LINUX_DEPS="build-essential autoconf libssl-dev libsqlite3-dev libgmp-dev"
```

A generic dockerfile exists that should be able to build for many Linux distros.

## License

See the LICENSE.md file for details.

## Copyright

Copyright (c) 2025 Charles Benedict, Jr.

This copyright notice must be retained in its entirety.
The LICENSE.md file must be retained and must be included with any distribution of this file.

