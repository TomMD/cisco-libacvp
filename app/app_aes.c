/*
 * Copyright (c) 2019, Cisco Systems, Inc.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/cisco/libacvp/LICENSE
 */


#include <openssl/evp.h>
#include "acvp/acvp.h"
#include "app_lcl.h"

static EVP_CIPHER_CTX *glb_cipher_ctx = NULL; /* need to maintain across calls for MCT */

void app_aes_cleanup(void) {
    if (glb_cipher_ctx) EVP_CIPHER_CTX_free(glb_cipher_ctx);
    glb_cipher_ctx = NULL;
}

int app_aes_handler(ACVP_TEST_CASE *test_case) {
    ACVP_SYM_CIPHER_TC      *tc;
    EVP_CIPHER_CTX *cipher_ctx;
    const EVP_CIPHER        *cipher;
    int ct_len, pt_len;
    unsigned char *iv = 0;
    /* assume fail at first */
    int rv = 1;

    if (!test_case) {
        return rv;
    }

    tc = test_case->tc.symmetric;

    if (glb_cipher_ctx == NULL) {
        glb_cipher_ctx = EVP_CIPHER_CTX_new();
        if (glb_cipher_ctx == NULL) {
            printf("Failed to allocate global cipher_ctx");
            return 1;
        }
    }

    /* Begin encrypt code section */
    cipher_ctx = glb_cipher_ctx;
    if ((tc->test_type != ACVP_SYM_TEST_TYPE_MCT)) {
        EVP_CIPHER_CTX_init(cipher_ctx);
    }

    switch (tc->cipher) {
    case ACVP_AES_ECB:
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_ecb();
            break;
        case 192:
            cipher = EVP_aes_192_ecb();
            break;
        case 256:
            cipher = EVP_aes_256_ecb();
            break;
        default:
            printf("Unsupported AES key length\n");
            return rv;

            break;
        }
        break;
    case ACVP_AES_CTR:
        iv = tc->iv;
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_ctr();
            break;
        case 192:
            cipher = EVP_aes_192_ctr();
            break;
        case 256:
            cipher = EVP_aes_256_ctr();
            break;
        default:
            printf("Unsupported AES key length\n");
            return rv;

            break;
        }
        break;
    case ACVP_AES_CFB1:
        iv = tc->iv;
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_cfb1();
            break;
        case 192:
            cipher = EVP_aes_192_cfb1();
            break;
        case 256:
            cipher = EVP_aes_256_cfb1();
            break;
        default:
            printf("Unsupported AES key length\n");
            return rv;

            break;
        }
        break;
    case ACVP_AES_CFB8:
        iv = tc->iv;
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_cfb8();
            break;
        case 192:
            cipher = EVP_aes_192_cfb8();
            break;
        case 256:
            cipher = EVP_aes_256_cfb8();
            break;
        default:
            printf("Unsupported AES key length\n");
            return rv;

            break;
        }
        break;
    case ACVP_AES_CFB128:
        iv = tc->iv;
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_cfb128();
            break;
        case 192:
            cipher = EVP_aes_192_cfb128();
            break;
        case 256:
            cipher = EVP_aes_256_cfb128();
            break;
        default:
            printf("Unsupported AES key length\n");
            return rv;

            break;
        }
        break;
    case ACVP_AES_OFB:
        iv = tc->iv;
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_ofb();
            break;
        case 192:
            cipher = EVP_aes_192_ofb();
            break;
        case 256:
            cipher = EVP_aes_256_ofb();
            break;
        default:
            printf("Unsupported AES key length\n");
            return rv;

            break;
        }
        break;
    case ACVP_AES_CBC:
        iv = tc->iv;
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_cbc();
            break;
        case 192:
            cipher = EVP_aes_192_cbc();
            break;
        case 256:
            cipher = EVP_aes_256_cbc();
            break;
        default:
            printf("Unsupported AES key length\n");
            return rv;

            break;
        }
        break;
    case ACVP_AES_XTS:
        iv = tc->iv;
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_xts();
            break;
        case 256:
            cipher = EVP_aes_256_xts();
            break;
        default:
            printf("Unsupported AES key length\n");
            return rv;

            break;
        }
        break;
    default:
        printf("Error: Unsupported AES mode requested by ACVP server\n");
        return rv;

        break;
    }

    /* If Monte Carlo we need to be able to init and then update
     * one thousand times before we complete each iteration.
     */
    if (tc->test_type == ACVP_SYM_TEST_TYPE_MCT) {
        if (tc->direction == ACVP_SYM_CIPH_DIR_ENCRYPT) {
            if (tc->mct_index == 0) {
                EVP_EncryptInit_ex(cipher_ctx, cipher, NULL, tc->key, iv);
                EVP_CIPHER_CTX_set_padding(cipher_ctx, 0);
                if (tc->cipher == ACVP_AES_CFB1) {
                    EVP_CIPHER_CTX_set_flags(cipher_ctx, EVP_CIPH_FLAG_LENGTH_BITS);
                }
            }
            EVP_EncryptUpdate(cipher_ctx, tc->ct, &ct_len, tc->pt, tc->pt_len);
            tc->ct_len = ct_len;
        } else if (tc->direction == ACVP_SYM_CIPH_DIR_DECRYPT) {
            if (tc->mct_index == 0) {
                EVP_DecryptInit_ex(cipher_ctx, cipher, NULL, tc->key, iv);
                EVP_CIPHER_CTX_set_padding(cipher_ctx, 0);
                if (tc->cipher == ACVP_AES_CFB1) {
                    EVP_CIPHER_CTX_set_flags(cipher_ctx, EVP_CIPH_FLAG_LENGTH_BITS);
                }
            }
            EVP_DecryptUpdate(cipher_ctx, tc->pt, &pt_len, tc->ct, tc->ct_len);
            tc->pt_len = pt_len;
        } else {
            printf("Unsupported direction\n");
            return rv;
        }
        if (tc->mct_index == 999) {
            EVP_CIPHER_CTX_cleanup(cipher_ctx);
        }
    } else {
        if (tc->direction == ACVP_SYM_CIPH_DIR_ENCRYPT) {
            EVP_EncryptInit_ex(cipher_ctx, cipher, NULL, tc->key, iv);
            EVP_CIPHER_CTX_set_padding(cipher_ctx, 0);
            if (tc->cipher == ACVP_AES_CFB1) {
                EVP_CIPHER_CTX_set_flags(cipher_ctx, EVP_CIPH_FLAG_LENGTH_BITS);
            }
            EVP_EncryptUpdate(cipher_ctx, tc->ct, &ct_len, tc->pt, tc->pt_len);
            tc->ct_len = ct_len;
            EVP_EncryptFinal_ex(cipher_ctx, tc->ct + ct_len, &ct_len);
            tc->ct_len += ct_len;
        } else if (tc->direction == ACVP_SYM_CIPH_DIR_DECRYPT) {
            EVP_DecryptInit_ex(cipher_ctx, cipher, NULL, tc->key, iv);
            EVP_CIPHER_CTX_set_padding(cipher_ctx, 0);
            if (tc->cipher == ACVP_AES_CFB1) {
                EVP_CIPHER_CTX_set_flags(cipher_ctx, EVP_CIPH_FLAG_LENGTH_BITS);
            }
            EVP_DecryptUpdate(cipher_ctx, tc->pt, &pt_len, tc->ct, tc->ct_len);
            tc->pt_len = pt_len;
            EVP_DecryptFinal_ex(cipher_ctx, tc->pt + pt_len, &pt_len);
            tc->pt_len += pt_len;
        } else {
            printf("Unsupported direction\n");
            return rv;
        }
        EVP_CIPHER_CTX_cleanup(cipher_ctx);
    }

    return 0;
}

/* NOTE - openssl does not support inverse option */
int app_aes_keywrap_handler(ACVP_TEST_CASE *test_case) {
    ACVP_SYM_CIPHER_TC      *tc;
    EVP_CIPHER_CTX *cipher_ctx = NULL;
    const EVP_CIPHER        *cipher;
    int c_len;
    int rc = 1;

    if (!test_case) {
        return rc;
    }

    tc = test_case->tc.symmetric;

    if (tc->kwcipher != ACVP_SYM_KW_CIPHER) {
        return rc;
    }

    /* Begin encrypt code section */
    cipher_ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(cipher_ctx);

    switch (tc->cipher) {
    case ACVP_AES_KW:
    case ACVP_AES_KWP:
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_wrap();
            break;
        case 192:
            cipher = EVP_aes_192_wrap();
            break;
        case 256:
            cipher = EVP_aes_256_wrap();
            break;
        default:
            printf("Unsupported AES keywrap key length\n");
            goto end;
        }
        break;
    default:
        printf("Error: Unsupported AES keywrap mode requested by ACVP server\n");
        goto end;
    }

    if (tc->direction == ACVP_SYM_CIPH_DIR_ENCRYPT) {
        EVP_CIPHER_CTX_set_flags(cipher_ctx, EVP_CIPHER_CTX_FLAG_WRAP_ALLOW);
        EVP_CipherInit_ex(cipher_ctx, cipher, NULL, tc->key, NULL, 1);
        c_len = EVP_Cipher(cipher_ctx, tc->ct, tc->pt, tc->pt_len);
        if (c_len <= 0) {
            printf("Error: key wrap operation failed (%d)\n", c_len);
            goto end;
        } else {
            tc->ct_len = c_len;
        }
    } else if (tc->direction == ACVP_SYM_CIPH_DIR_DECRYPT) {
        EVP_CIPHER_CTX_set_flags(cipher_ctx, EVP_CIPHER_CTX_FLAG_WRAP_ALLOW);
        EVP_CipherInit_ex(cipher_ctx, cipher, NULL, tc->key, NULL, 0);

#ifdef OPENSSL_KWP
        if (tc->cipher == ACVP_AES_KWP) {
            EVP_CIPHER_CTX_set_flags(cipher_ctx, EVP_CIPHER_CTX_FLAG_UNWRAP_WITHPAD);
        }
#endif
        c_len = EVP_Cipher(cipher_ctx, tc->pt, tc->ct, tc->ct_len);
        if (c_len <= 0) {
            goto end;
        } else {
            tc->pt_len = c_len;
        }
    } else {
        printf("Unsupported direction\n");
        goto end;
    }
    rc = 0;

end:
    /* Cleanup */
    if (cipher_ctx) EVP_CIPHER_CTX_free(cipher_ctx);

    return rc;
}

/*
 * This fuction is invoked by libacvp when an AES crypto
 * operation is needed from the crypto module being
 * validated.  This is a callback provided to libacvp when
 * acvp_enable_capability() is invoked to register the
 * AES-GCM capabilitiy with libacvp.  libacvp will in turn
 * invoke this function when it needs to process an AES-GCM
 * test case.
 */
int app_aes_handler_aead(ACVP_TEST_CASE *test_case) {
    ACVP_SYM_CIPHER_TC      *tc;
    EVP_CIPHER_CTX *cipher_ctx = NULL;
    const EVP_CIPHER        *cipher;
    unsigned char iv_fixed[4] = { 1, 2, 3, 4 };
    int rc = 0;
    int ret = 0;

    if (!test_case) {
        return 1;
    }

    tc = test_case->tc.symmetric;

    if (tc->direction != ACVP_SYM_CIPH_DIR_ENCRYPT && tc->direction != ACVP_SYM_CIPH_DIR_DECRYPT) {
        printf("Unsupported direction\n");
        return 1;
    }

    /* Begin encrypt code section */
    cipher_ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(cipher_ctx);

    /* Validate key length and assign OpenSSL EVP cipher */
    switch (tc->cipher) {
    case ACVP_AES_GCM:
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_gcm();
            break;
        case 192:
            cipher = EVP_aes_192_gcm();
            break;
        case 256:
            cipher = EVP_aes_256_gcm();
            break;
        default:
            printf("Unsupported AES-GCM key length\n");
            rc = 1;
            goto end;
        }
        if (tc->direction == ACVP_SYM_CIPH_DIR_ENCRYPT) {
            EVP_CIPHER_CTX_set_flags(cipher_ctx, EVP_CIPH_FLAG_NON_FIPS_ALLOW);
            EVP_CipherInit(cipher_ctx, cipher, NULL, NULL, 1);
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_GCM_SET_IVLEN, tc->iv_len, 0);
            EVP_CipherInit(cipher_ctx, NULL, tc->key, NULL, 1);

            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_GCM_SET_IV_FIXED, 4, iv_fixed);
            if (!EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_GCM_IV_GEN, tc->iv_len, tc->iv)) {
                printf("acvp_aes_encrypt: iv gen error\n");
                rc = 1;
                goto end;
            }
            if (tc->aad_len) {
                EVP_Cipher(cipher_ctx, NULL, tc->aad, tc->aad_len);
            }
            EVP_Cipher(cipher_ctx, tc->ct, tc->pt, tc->pt_len);
            EVP_Cipher(cipher_ctx, NULL, NULL, 0);
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_GCM_GET_TAG, tc->tag_len, tc->tag);
        } else if (tc->direction == ACVP_SYM_CIPH_DIR_DECRYPT) {
            EVP_CIPHER_CTX_set_flags(cipher_ctx, EVP_CIPH_FLAG_NON_FIPS_ALLOW);
            EVP_CipherInit_ex(cipher_ctx, cipher, NULL, tc->key, NULL, 0);
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_GCM_SET_IVLEN, tc->iv_len, 0);
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_GCM_SET_IV_FIXED, -1, tc->iv);
            if (!EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_GCM_IV_GEN, tc->iv_len, tc->iv)) {
                printf("\nFailed to set IV");
                rc = 1;
                goto end;
            }
            if (tc->aad_len) {
                /*
                 * Set dummy tag before processing AAD.  Otherwise the AAD can
                 * not be processed.
                 */
                EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_GCM_SET_TAG, tc->tag_len, tc->tag);
                EVP_Cipher(cipher_ctx, NULL, tc->aad, tc->aad_len);
            }
            /*
             * Set the tag when decrypting
             */
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_GCM_SET_TAG, tc->tag_len, tc->tag);

            /*
             * Decrypt the CT
             */
            EVP_Cipher(cipher_ctx, tc->pt, tc->ct, tc->ct_len);
            /*
             * Check the tag
             */
            ret = EVP_Cipher(cipher_ctx, NULL, NULL, 0);
            if (ret) {
                rc = 1;
                goto end;
            }
        }
        break;
    case ACVP_AES_CCM:
        switch (tc->key_len) {
        case 128:
            cipher = EVP_aes_128_ccm();
            break;
        case 192:
            cipher = EVP_aes_192_ccm();
            break;
        case 256:
            cipher = EVP_aes_256_ccm();
            break;
        default:
            printf("Unsupported AES-CCM key length\n");
            rc = 1;
            goto end;
        }
        if (tc->direction == ACVP_SYM_CIPH_DIR_ENCRYPT) {
            EVP_CipherInit(cipher_ctx, cipher, NULL, NULL, 1);
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_CCM_SET_IVLEN, tc->iv_len, 0);
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_CCM_SET_TAG, tc->tag_len, 0);
            EVP_CipherInit(cipher_ctx, NULL, tc->key, tc->iv, 1);
            EVP_Cipher(cipher_ctx, NULL, NULL, tc->pt_len);
            EVP_Cipher(cipher_ctx, NULL, tc->aad, tc->aad_len);
            EVP_Cipher(cipher_ctx, tc->ct, tc->pt, tc->pt_len);
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_CCM_GET_TAG, tc->tag_len, tc->ct + tc->ct_len);
            tc->ct_len += tc->tag_len;
        } else if (tc->direction == ACVP_SYM_CIPH_DIR_DECRYPT) {
            EVP_CipherInit(cipher_ctx, cipher, NULL, NULL, 0);
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_CCM_SET_IVLEN, tc->iv_len, 0);
            EVP_CIPHER_CTX_ctrl(cipher_ctx, EVP_CTRL_CCM_SET_TAG, tc->tag_len, tc->ct + tc->pt_len);
            EVP_CipherInit(cipher_ctx, NULL, tc->key, tc->iv, 0);
            EVP_Cipher(cipher_ctx, NULL, NULL, tc->pt_len);
            EVP_Cipher(cipher_ctx, NULL, tc->aad, tc->aad_len);
            /*
             * Decrypt and check the tag
             */
            ret = EVP_Cipher(cipher_ctx, tc->pt, tc->ct, tc->ct_len);
            if (ret < 0) {
                rc = 1;
                goto end;
            }
        }
        break;
    default:
        printf("Error: Unsupported AES AEAD mode requested by ACVP server\n");
        rc = 1;
        goto end;
    }

end:
    /* Cleanup */
    if (cipher_ctx) EVP_CIPHER_CTX_free(cipher_ctx);

    return rc;
}

