#define LIBRARY_LOG_LEVEL LOG_INFO

#include <mbedtls/x509_csr.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/platform.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/pk.h>
#include <mbedtls/error.h>
#include <mbedtls/entropy_poll.h>>
#include <mbedtls/oid.h>>

#include "aknano_priv.h"
#include "aknano_secret.h"
#include "aknano_provisioning_secret.h"

static char subject[200];
static char buf[1024];

#if 0
/* Function to be used if the PK pair is generated in a previous step (e.g., using PKCS#11) */
int aknano_gen_device_certificate_from_key(
                    CK_BYTE * pxDerPublicKey,
                    CK_ULONG ulDerPublicKeyLength,
                    const char* uuid, const char* factory_name, 
                    const char* serial_string, char* cert_buf)
{
    LogInfo(("aknano_gen_device_certificate_from_key")); vTaskDelay(50 / portTICK_PERIOD_MS);
    
    const char* pers = uuid;
    const char* not_before = "20210101000000";
    const char* not_after =  "20301231235959";
    char issuer_subject[200];

    mbedtls_mpi serial;
#if 0
    mbedtls_entropy_context entropy;
#endif
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_pk_context device_key;

    mbedtls_pk_context factory_key; 
    mbedtls_x509write_cert new_device_crt;
    mbedtls_x509_crt factory_crt;

    int ret;

    sprintf(subject, "CN=%s,OU=%s", uuid, factory_name);


    mbedtls_mpi_init(&serial);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_pk_init(&device_key);

//    mbedtls_entropy_init(&entropy);
//    mbedtls_pk_init(&new_device_key);
    mbedtls_pk_init(&factory_key);
    mbedtls_x509write_crt_init(&new_device_crt);
    mbedtls_x509_crt_init(&factory_crt);


    LogInfo(("ulDerPublicKeyLength=%lu", ulDerPublicKeyLength)); vTaskDelay(50 / portTICK_PERIOD_MS);

    ret = mbedtls_pk_parse_public_key(&device_key, pxDerPublicKey, ulDerPublicKeyLength);
    if( ret != 0 )    {
        LogInfo(("mbedtls_pk_parse_key error")); vTaskDelay(50 / portTICK_PERIOD_MS);
        mbedtls_printf("mbedtls_pk_parse_key error: %d [0x%04X]", ret, -ret);
        goto exit;
    }
    LogInfo(("mbedtls_pk_parse_key OK")); vTaskDelay(50 / portTICK_PERIOD_MS);

#if 0

    ret = mbedtls_pk_setup(&new_device_key, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if( ret != 0 )    {
        LOG_WRN("mbedtls_pk_setup error: %d", ret);
        goto exit;
    }

    ret = mbedtls_entropy_add_source(&entropy, aknano_entropy_source,
            entropy_device, 0, MBEDTLS_ENTROPY_SOURCE_STRONG);

    if( ret != 0 )    {
        LOG_WRN("mbedtls_entropy_add_source error: %d [0x%04X]", ret, -ret);
        goto exit;
    }

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func,
                               &entropy,
                               (const unsigned char*)pers,
                               strlen(pers));
    if( ret != 0 )    {
        LOG_WRN("mbedtls_ctr_drbg_seed error: %d [0x%04X]", ret, -ret);
        goto exit;
    }
    ret = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1,
                              mbedtls_pk_ec(new_device_key),
                              mbedtls_ctr_drbg_random,
                              &ctr_drbg);
    if( ret != 0 )    {
        LOG_WRN("mbedtls_ecp_gen_key error: %d [0x%04X]", ret, -ret);
        goto exit;
    }

    memset(key_buf, 0, AKNANO_CERT_BUF_SIZE);
    ret = mbedtls_pk_write_key_pem(&new_device_key, key_buf, AKNANO_CERT_BUF_SIZE);
    if( ret != 0 )    {
        LOG_INF("Error writting key to buffer");
        goto exit;
    }

    /* Key created */
#endif
    /* Sign CSR with CA key */

    ret = mbedtls_pk_parse_key(&factory_key, (const unsigned char *)AKNANO_ISSUER_KEY,
                strlen(AKNANO_ISSUER_KEY)+1,NULL, 0);
    if (ret < 0) {
        mbedtls_printf("Error loading KEY_ISSUER (%d) [0x%04X]\n", ret, ret & 0xFFFF);
        goto exit;
    }

    ret = mbedtls_x509_crt_parse(&factory_crt, (const unsigned char*)AKNANO_ISSUER_CERTIFICATE, 
                                strlen(AKNANO_ISSUER_CERTIFICATE) + 1);
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_x509_crt_parse error: %d [0x%04X]", ret, -ret);
        goto exit;
    }

    mbedtls_x509write_crt_set_subject_key(&new_device_crt, &device_key);

    mbedtls_x509write_crt_set_issuer_key(&new_device_crt, &factory_key);
    ret = mbedtls_x509_dn_gets( issuer_subject, sizeof(issuer_subject), &factory_crt.subject);
    if (ret < 0) {
         mbedtls_printf("Error geting issuer subject [0x%04X]\n", (-ret) & 0xFFFF);
         goto exit;
    }

    mbedtls_x509write_crt_set_issuer_name(&new_device_crt, issuer_subject);
    mbedtls_x509write_crt_set_md_alg( &new_device_crt, MBEDTLS_MD_SHA256);
    ret = mbedtls_x509write_crt_set_subject_name(&new_device_crt, subject);
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_x509write_crt_set_subject_name error: %d [0x%04X]", ret, -ret);
        goto exit;
    }

    if( ( ret = mbedtls_mpi_read_string( &serial, 16, serial_string ) ) != 0 )
    {
        mbedtls_printf("mbedtls_mpi_read_string returned -0x%04x - %s", 
                (unsigned int) -ret, buf);
        goto exit;
    }

    ret = mbedtls_x509write_crt_set_serial( &new_device_crt, &serial );
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_x509write_crt_set_serial returned -0x%04x - %s",
                (unsigned int) -ret, buf );
        goto exit;
    }
    ret = mbedtls_x509write_crt_set_validity( &new_device_crt, not_before, not_after );
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_x509write_crt_set_validity returned -0x%04x - %s", 
                (unsigned int) -ret, buf );
        goto exit;
    }

    memset(cert_buf, 0, AKNANO_CERT_BUF_SIZE);
    ret = mbedtls_x509write_crt_pem(&new_device_crt, cert_buf, AKNANO_CERT_BUF_SIZE,
                            mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 ) {
        mbedtls_printf("Error writting certificate to buffer ret=%d 0x%X \n", ret, (-ret) & 0xFFFF );
        char s[500];
        mbedtls_strerror(ret, s, 500);
        mbedtls_printf("[%s]\n", s);

        goto exit;
    }

exit:
    mbedtls_mpi_free(&serial);
    // mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_pk_free(&device_key);
    mbedtls_pk_free(&factory_key);
    mbedtls_x509write_crt_free(&new_device_crt);
    mbedtls_x509_crt_free(&factory_crt);

    return ret;
}

#endif

int aknano_gen_device_certificate_and_key(
                    const char* uuid, const char* factory_name, 
                    const char* serial_string, char* cert_buf, char* key_buf)
{
    LogInfo(("aknano_gen_device_certificate_and_key")); vTaskDelay(50 / portTICK_PERIOD_MS);
    
    // struct device *entropy_device = device_get_binding(DT_CHOSEN_ZEPHYR_ENTROPY_LABEL);
    const char* pers = uuid;
    const char* not_before = "20210101000000";
    const char* not_after =  "20301231235959";
    char issuer_subject[200];

    mbedtls_mpi serial;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_pk_context new_device_key;

    mbedtls_pk_context factory_key; 
    mbedtls_x509write_cert new_device_crt;
    mbedtls_x509_crt factory_crt;

    int ret;

    sprintf(subject, "CN=%s,OU=%s", uuid, factory_name);


    mbedtls_mpi_init(&serial);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    mbedtls_entropy_init(&entropy);
    mbedtls_pk_init(&new_device_key);
    mbedtls_pk_init(&factory_key);
    mbedtls_x509write_crt_init(&new_device_crt);
    mbedtls_x509_crt_init(&factory_crt);

    LogInfo(("aknano_gen_device_certificate_and_key 1")); vTaskDelay(50 / portTICK_PERIOD_MS);

#if 0
    ret = mbedtls_pk_parse_public_key(&device_key, pxDerPublicKey, ulDerPublicKeyLength);
    if( ret != 0 )    {
        LogInfo(("mbedtls_pk_parse_key error")); vTaskDelay(50 / portTICK_PERIOD_MS);
        mbedtls_printf("mbedtls_pk_parse_key error: %d [0x%04X]", ret, -ret);
        goto exit;
    }
    LogInfo(("mbedtls_pk_parse_key OK")); vTaskDelay(50 / portTICK_PERIOD_MS);
#endif
    LogInfo(("aknano_gen_device_certificate_and_key 2")); vTaskDelay(50 / portTICK_PERIOD_MS);

    ret = mbedtls_pk_setup(&new_device_key, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_pk_setup error: %d", ret);
        goto exit;
    }
    LogInfo(("aknano_gen_device_certificate_and_key 3")); vTaskDelay(50 / portTICK_PERIOD_MS);

    ret = mbedtls_entropy_add_source(&entropy, mbedtls_hardware_poll, NULL,
                            MBEDTLS_ENTROPY_MIN_HARDWARE,
                            MBEDTLS_ENTROPY_SOURCE_STRONG );
    // ret = mbedtls_entropy_add_source(&entropy, mbedtls_platform_entropy_poll,
    //             NULL, 0, MBEDTLS_ENTROPY_SOURCE_STRONG);
    LogInfo(("aknano_gen_device_certificate_and_key 4")); vTaskDelay(50 / portTICK_PERIOD_MS);

    if( ret != 0 )    {
        mbedtls_printf("mbedtls_entropy_add_source error: %d [0x%04X]", ret, -ret);
        goto exit;
    }

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func,
                               &entropy,
                               (const unsigned char*)pers,
                               strlen(pers));
    LogInfo(("aknano_gen_device_certificate_and_key 5")); vTaskDelay(50 / portTICK_PERIOD_MS);
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_ctr_drbg_seed error: %d [0x%04X]", ret, -ret);
        goto exit;
    }
    ret = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1,
                              mbedtls_pk_ec(new_device_key),
                              mbedtls_ctr_drbg_random,
                              &ctr_drbg);
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_ecp_gen_key error: %d [0x%04X]", ret, -ret);
        goto exit;
    }
    LogInfo(("aknano_gen_device_certificate_and_key 6")); vTaskDelay(50 / portTICK_PERIOD_MS);

    memset(key_buf, 0, AKNANO_CERT_BUF_SIZE);
    ret = mbedtls_pk_write_key_pem(&new_device_key, key_buf, AKNANO_CERT_BUF_SIZE);
    if( ret != 0 )    {
        mbedtls_printf("Error writting key to buffer");
        goto exit;
    }

    /* Key created */
    LogInfo(("aknano_gen_device_certificate_and_key key created\r\n%s", key_buf)); vTaskDelay(50 / portTICK_PERIOD_MS);

    /* Sign CSR with CA key */

    ret = mbedtls_pk_parse_key(&factory_key, (const unsigned char *)AKNANO_ISSUER_KEY,
                strlen(AKNANO_ISSUER_KEY)+1,NULL, 0);
    if (ret < 0) {
        mbedtls_printf("Error loading KEY_ISSUER (%d) [0x%04X]\n", ret, ret & 0xFFFF);
        goto exit;
    }

    ret = mbedtls_x509_crt_parse(&factory_crt, (const unsigned char*)AKNANO_ISSUER_CERTIFICATE, 
                                strlen(AKNANO_ISSUER_CERTIFICATE) + 1);
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_x509_crt_parse error: %d [0x%04X]", ret, -ret);
        goto exit;
    }

    mbedtls_x509write_crt_set_subject_key(&new_device_crt, &new_device_key);

    mbedtls_x509write_crt_set_issuer_key(&new_device_crt, &factory_key);
    ret = mbedtls_x509_dn_gets( issuer_subject, sizeof(issuer_subject), &factory_crt.subject);
    if (ret < 0) {
         mbedtls_printf("Error geting issuer subject [0x%04X]\n", (-ret) & 0xFFFF);
         goto exit;
    }

    mbedtls_x509write_crt_set_issuer_name(&new_device_crt, issuer_subject);
    mbedtls_x509write_crt_set_md_alg( &new_device_crt, MBEDTLS_MD_SHA256);
    ret = mbedtls_x509write_crt_set_subject_name(&new_device_crt, subject);
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_x509write_crt_set_subject_name error: %d [0x%04X]", ret, -ret);
        goto exit;
    }

    if( ( ret = mbedtls_mpi_read_string( &serial, 16, serial_string ) ) != 0 )
    {
        mbedtls_printf("mbedtls_mpi_read_string returned -0x%04x - %s", 
                (unsigned int) -ret, buf);
        goto exit;
    }

    ret = mbedtls_x509write_crt_set_serial( &new_device_crt, &serial );
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_x509write_crt_set_serial returned -0x%04x - %s",
                (unsigned int) -ret, buf );
        goto exit;
    }
    ret = mbedtls_x509write_crt_set_validity( &new_device_crt, not_before, not_after );
    if( ret != 0 )    {
        mbedtls_printf("mbedtls_x509write_crt_set_validity returned -0x%04x - %s", 
                (unsigned int) -ret, buf );
        goto exit;
    }

#if 0
     ret = mbedtls_x509_set_extension(&new_device_crt.extensions, 
         MBEDTLS_OID_KEY_USAGE,
         MBEDTLS_OID_SIZE(MBEDTLS_OID_KEY_USAGE),
         1, "keyCertSign", strlen("keyCertSign")
         );

    ret = mbedtls_x509_set_extension(&new_device_crt.extensions, 
        MBEDTLS_OID_BASIC_CONSTRAINTS,
        MBEDTLS_OID_SIZE(MBEDTLS_OID_BASIC_CONSTRAINTS),
        1, "CA:TRUE, pathlen:0", strlen("CA:TRUE, pathlen:0")
        );
#endif
#if 0
     ret = mbedtls_x509_set_extension(&new_device_crt.extensions, 
         MBEDTLS_OID_KEY_USAGE,
         MBEDTLS_OID_SIZE(MBEDTLS_OID_KEY_USAGE),
         1, "Digital Signature", strlen("Digital Signature")
         );

     ret = mbedtls_x509_set_extension(&new_device_crt.extensions, 
         MBEDTLS_OID_EXTENDED_KEY_USAGE,
         MBEDTLS_OID_SIZE(MBEDTLS_OID_EXTENDED_KEY_USAGE),
         1, "TLS Web Client Authentication", strlen("TLS Web Client Authentication")
         );
#endif

    memset(cert_buf, 0, AKNANO_CERT_BUF_SIZE);
    ret = mbedtls_x509write_crt_pem(&new_device_crt, cert_buf, AKNANO_CERT_BUF_SIZE,
                            mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 ) {
        mbedtls_printf("Error writting certificate to buffer ret=%d 0x%X \n", ret, (-ret) & 0xFFFF );
        char s[500];
        mbedtls_strerror(ret, s, 500);
        mbedtls_printf("[%s]\n", s);

        goto exit;
    }

exit:
    mbedtls_mpi_free(&serial);
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_pk_free(&new_device_key);
    mbedtls_pk_free(&factory_key);
    mbedtls_x509write_crt_free(&new_device_crt);
    mbedtls_x509_crt_free(&factory_crt);

    return ret;
}
