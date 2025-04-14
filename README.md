# Cryptomath

A C library for handling cryptocurrency amounts with arbitrary precision, supporting multiple cryptocurrencies and their various denominations.

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

4. For debugging:
   ```bash
   # Build with debug symbols and no optimization
   make debug
   
   # Run tests with debug build
   make test
   ```

   The debug build includes:
   - Debug symbols (-ggdb)
   - No optimization (-O0)
   - Additional warnings (-Wall -Wextra)
   - Automatic dependency generation (-MMD -MP)

## API

### Types

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

// Arithmetic operations
void crypto_add(crypto_val_t* r, const crypto_val_t* a, const crypto_val_t* b);
void crypto_sub(crypto_val_t* r, const crypto_val_t* a, const crypto_val_t* b);
void crypto_mul_i64(crypto_val_t* r, const crypto_val_t* a, int64_t s);
void crypto_divt_ui64(crypto_val_t* r, const crypto_val_t* a, uint64_t s);

// Comparison operations
int crypto_cmp(const crypto_val_t* a, const crypto_val_t* b);
int crypto_gt_zero(const crypto_val_t* a);
int crypto_lt_zero(const crypto_val_t* a);
int crypto_eq_zero(const crypto_val_t* a);

// Cleanup
void crypto_clear(crypto_val_t* val);
```

## Usage Example

```c
#include "cryptomath2.h"

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
The library supports many other cryptocurrencies with their respective denominations. Each cryptocurrency has a base unit and a smallest unit (usually with 0 decimal places).

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
make test
```

## License

See the LICENSE file for details.