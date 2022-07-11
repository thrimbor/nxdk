#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <nxdk/net.h>

#include <assert.h>
#include <stdio.h>
#define _CRT_RAND_S
#include <stdlib.h>

#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"

#include <string.h>

#define SERVER_PORT "443"
#define SERVER_NAME "google.de"
#define GET_REQUEST "GET / HTTP/1.0\r\n\r\n"

#define DEBUG_LEVEL 1


static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    debugPrint("%s:%d: %s\n", file, line, str );
}

int custom_mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len )
{
    int fd = ((mbedtls_net_context *) ctx)->fd;
    return send(fd, buf, len, 0);
}

int custom_mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len )
{
    int fd = ((mbedtls_net_context *) ctx)->fd;
    return recv(fd, buf, len, 0);
}

int custom_mbedtls_net_connect( mbedtls_net_context *ctx, const char *host,
                         const char *port, int proto )
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    struct addrinfo hints, *addr_list, *cur;

    /* Do name resolution with IPv4 */
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = proto == MBEDTLS_NET_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = proto == MBEDTLS_NET_PROTO_UDP ? IPPROTO_UDP : IPPROTO_TCP;

    if( getaddrinfo( host, port, &hints, &addr_list ) != 0 )
        return( MBEDTLS_ERR_NET_UNKNOWN_HOST );

    /* Try the sockaddrs until a connection succeeds */
    ret = MBEDTLS_ERR_NET_UNKNOWN_HOST;
    for( cur = addr_list; cur != NULL; cur = cur->ai_next )
    {
        ctx->fd = (int) socket( cur->ai_family, cur->ai_socktype,
                            cur->ai_protocol );
        if( ctx->fd < 0 )
        {
            ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
            continue;
        }

        if( connect( ctx->fd, cur->ai_addr, cur->ai_addrlen ) == 0 )
        {
            ret = 0;
            break;
        }

        close( ctx->fd );
        ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
    }

    freeaddrinfo( addr_list );

    return( ret );
}

int entropy_function (void *data, unsigned char *output, size_t len, size_t *olen)
{
    if (len < 4) return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;

    size_t written = 0;
    while (written < len-4) {
        rand_s((unsigned int *)output);
        output += 4;
        written += 4;
    }

    *olen = written;
    return 0;
}

extern struct netif *g_pnetif;

int main( void )
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    nxNetInit(NULL);

    debugPrint("\n");
    debugPrint("IP address.. %s\n", ip4addr_ntoa(netif_ip4_addr(g_pnetif)));
    debugPrint("Mask........ %s\n", ip4addr_ntoa(netif_ip4_netmask(g_pnetif)));
    debugPrint("Gateway..... %s\n", ip4addr_ntoa(netif_ip4_gw(g_pnetif)));
    debugPrint("\n");

    int ret = 1, len;
    int exit_code = EXIT_FAILURE;
    mbedtls_net_context server_fd;
    uint32_t flags;
    unsigned char buf[1024];
    const char *pers = "ssl_client1";

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold( DEBUG_LEVEL );
#endif

    /*
     * 0. Initialize the RNG and the session data
     */
    //mbedtls_net_init( &server_fd );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    mbedtls_x509_crt_init( &cacert );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    debugPrint( "\n  . Seeding the random number generator..." );


    mbedtls_entropy_init( &entropy );
    mbedtls_entropy_add_source(&entropy, entropy_function, NULL, 0, MBEDTLS_ENTROPY_SOURCE_STRONG);
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        debugPrint( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        goto exit;
    }

    debugPrint( " ok\n" );

    /*
     * 0. Initialize certificates
     */
    debugPrint( "  . Loading the CA root certificate ..." );
    // We're just loading the GlobalSign Root CA used by google here

    FILE *f = fopen("D:\\google-de.crt", "rb");
    assert(f);
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // We're technically leaking this allocation
    char *string = malloc(fsize);
    fread(string, fsize, 1, f);
    fclose(f);

    ret = mbedtls_x509_crt_parse( &cacert, (const unsigned char *) string,
                          fsize );
    if( ret < 0 )
    {
        debugPrint( " failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", (unsigned int) -ret );
        goto exit;
    }

    debugPrint( " ok (%d skipped)\n", ret );

    /*
     * 1. Start the connection
     */
    debugPrint( "  . Connecting to tcp/%s/%s...", SERVER_NAME, SERVER_PORT );


    if( ( ret = custom_mbedtls_net_connect( &server_fd, SERVER_NAME,
                                         SERVER_PORT, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        debugPrint( " failed\n  ! mbedtls_net_connect returned %d\n\n", ret );
        goto exit;
    }

    debugPrint( " ok\n" );

    /*
     * 2. Setup stuff
     */
    debugPrint( "  . Setting up the SSL/TLS structure..." );


    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        debugPrint( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
        goto exit;
    }

    debugPrint( " ok\n" );

    /* OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );

    if( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
        debugPrint( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
        goto exit;
    }

    if( ( ret = mbedtls_ssl_set_hostname( &ssl, SERVER_NAME ) ) != 0 )
    {
        debugPrint( " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_ssl_set_bio( &ssl, &server_fd, custom_mbedtls_net_send, custom_mbedtls_net_recv, NULL );

    /*
     * 4. Handshake
     */
    debugPrint( "  . Performing the SSL/TLS handshake..." );


    while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            debugPrint( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", (unsigned int) -ret );
            goto exit;
        }
    }

    debugPrint( " ok\n" );

    /*
     * 5. Verify the server certificate
     */
    debugPrint( "  . Verifying peer X.509 certificate..." );

    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
    {
#if !defined(MBEDTLS_X509_REMOVE_INFO)
        char vrfy_buf[512];
#endif

        debugPrint( " failed\n" );

#if !defined(MBEDTLS_X509_REMOVE_INFO)
        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        debugPrint( "%s\n", vrfy_buf );
#endif
    }
    else
        debugPrint( " ok\n" );

    /*
     * 3. Write the GET request
     */
    debugPrint( "  > Write to server:" );


    len = sprintf( (char *) buf, GET_REQUEST );

    while( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            debugPrint( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
            goto exit;
        }
    }

    len = ret;
    debugPrint( " %d bytes written\n\n%s", len, (char *) buf );

    /*
     * 7. Read the HTTP response
     */
    debugPrint( "  < Read from server:" );


    do
    {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            debugPrint( "failed\n  ! mbedtls_ssl_read returned %d\n\n", ret );
            break;
        }

        if( ret == 0 )
        {
            debugPrint( "\n\nEOF\n\n" );
            break;
        }

        len = ret;
        debugPrint( " %d bytes read\n\n%s", len, (char *) buf );
    }
    while( 1 );

    mbedtls_ssl_close_notify( &ssl );

    exit_code = EXIT_SUCCESS;

exit:

#ifdef MBEDTLS_ERROR_C
    if( exit_code != EXIT_SUCCESS )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        debugPrint("Last error was: %d - %s\n\n", ret, error_buf );
    }
#endif

    // mbedtls_net_free does the same, but we probably shouldn't rely on that as we don't use the rest of their implementation
    //mbedtls_net_free( &server_fd );
    shutdown(server_fd.fd, 2);
    close(server_fd.fd);

    mbedtls_x509_crt_free( &cacert );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    // Give the user a chance to read what we printed ;)
    for(;;) Sleep(1000);

    return 0;
}
