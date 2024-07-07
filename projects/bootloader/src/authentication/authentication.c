/**
 * @file authentication.c
 * @brief This module is about the authentication of the bootloader. It provides the necessary functions to authenticate
 * the application image. The Elliptic Curve Digital Signature Algorithm (ECDSA) is used to sign the application.
 * The public key is stored in the bootloader, and the signature is stored in the application image. The bootloader
 * will verify the signature of the application image before jumping to it.
 *
 * The implementation of the ECDSA is not hardware accelerated.
 * @version 0.1
 * @date 2024-07-06
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include "authentication.h"

#include "ecdsa_pub_key.h"
#include "uECC.h"
#include "sha256.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// --- external variables ----------------------------------------------------------------------------------------------
extern const uint8_t ecdsa_public_key[];

// --- function definitions --------------------------------------------------------------------------------------------
/**
 * @brief Function to verify the signature of the application image. The signature is verified using the ECDSA
 * algorithm.
 *
 * @param app_image_start_addr The start address of the application image.
 * @param app_image_size_bytes The size of the application image in bytes.
 * @param signature The signature of the application image.
 *
 * @return true The signature is valid.
 * @return false The signature is invalid.
 */
bool
authenticate_application(uint32_t app_image_start_addr, uint32_t app_image_size_bytes, const uint8_t *signature)
{

    // Calculate the SHA-256 hash of the application image
    SHA256_CTX ctx;
    BYTE       hash[SHA256_BLOCK_SIZE];

    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE *)app_image_start_addr, app_image_size_bytes);
    sha256_final(&ctx, hash);

    // First check if public key is valid
    if (!uECC_valid_public_key(ecdsa_public_key, uECC_secp256r1()))
    {
        return false;
    }

    // Verify the signature using the ECDSA algorithm
    int ret = uECC_verify(ecdsa_public_key, hash, sizeof(hash), signature, uECC_secp256r1());

    // True if the signature is valid
    return ret == 1;
}

/**
 * @brief Function to get the public key used for verifying the signature of the application image. The public key is
 * stored in the bootloader.
 *
 * @return uint8_t* The public key used for verifying the signature.
 */
const uint8_t *
get_public_key(void)
{
    return ecdsa_public_key;
}