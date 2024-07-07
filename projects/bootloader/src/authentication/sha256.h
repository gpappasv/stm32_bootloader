/**
 * @file sha256.h
 * @brief Standard Secure Hash Algorithm (SHA-256) in C
 * @version 0.1
 * @date 2024-07-07
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SHA256_H
#define SHA256_H

// --- includes --------------------------------------------------------------------------------------------------------
#include <stddef.h>

// --- defines ---------------------------------------------------------------------------------------------------------
#define SHA256_BLOCK_SIZE 32 // SHA256 outputs a 32 byte digest

// --- typedefs --------------------------------------------------------------------------------------------------------
typedef unsigned char BYTE; // 8-bit byte
typedef unsigned int  WORD; // 32-bit word, change to "long" for 16-bit machines

typedef struct
{
    BYTE               data[64];
    WORD               datalen;
    unsigned long long bitlen;
    WORD               state[8];
} SHA256_CTX;

// --- function declarations -------------------------------------------------------------------------------------------
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len);
void sha256_final(SHA256_CTX *ctx, BYTE hash[]);

#endif // SHA256_H
