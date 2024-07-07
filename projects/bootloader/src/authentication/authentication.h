/**
 * @file authentication.h
 * @brief This module is about the authentication of the bootloader. It provides the necessary functions to authenticate
 *       the application image. The Elliptic Curve Digital Signature Algorithm (ECDSA) is used to sign the application.
 *       The public key is stored in the bootloader, and the signature is stored in the application image. The
 * bootloader will verify the signature of the application image before jumping to it.
 *
 *       The implementation of the ECDSA is not hardware accelerated.
 * @version 0.1
 * @date 2024-07-06
 *
 * @copyright Copyright (c) 2024
 *
 */

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>

// --- function declarations -------------------------------------------------------------------------------------------
/**
 * @brief Function to verify the signature of the application image. The signature is verified using the ECDSA
 * algorithm.
 *
 * @param app_image_start_addr: The start address of the application image.
 * @param app_image_size_bytes: The size of the application image in bytes.
 * @param signature: The signature of the application image.
 *
 * @return true: The signature is valid.
 * @return false: The signature is invalid.
 */
bool authenticate_application(uint32_t       app_image_start_addr,
                              uint32_t       app_image_size_bytes,
                              const uint8_t *der_signature);

/**
 * @brief Function to get the public key used for verifying the signature of the application image. The public key is
 *        stored in the bootloader.
 *
 * @return uint8_t*: The public key used for verifying the signature.
 */
const uint8_t *get_public_key(void);
