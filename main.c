#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <picoquic.h>
#include <picoquic_config.h>
#include <picoquic_packet_loop.h>
#include <performance_log.h>
#include <stdlib.h>
#include <stdio.h>

#include "cmake-build-debug/_deps/picoquic-src/loglib/autoqlog.h"
#include "qe2ed/qe2ed.h"
#include "qe2ed/qe2ed_client.h"
#include "qe2ed/qe2ed_internal.h"
#include "qe2ed/qe2ed_server.h"

static const int default_server_port = 4443;
static const char* default_server_name = "::";
static const char* ticket_store_filename = "qe2ed_ticket_store.bin";
static const char* token_store_filename = "qe2ed_token_store.bin";
#define SERVER_CERT_FILE "certs/cert.pem"
#define SERVER_KEY_FILE "certs/key.pem"

void usage() {
    fprintf(stdout, "qe2ed v0.0.0.1\n");
    fprintf(stdout, "\t-p\tRun server.\n");
}

int main(int argc, char** argv)
{
    int ret = 0;
    qe2ed_t* qe2ed = qe2ed_create();

    picoquic_quic_config_t config;
    char option_string[256];
    const char* server_name = default_server_name;
    char default_server_cert_file[512];
    char default_server_key_file[512];
    int server_port = default_server_port;
    struct sockaddr_storage server_sockaddr;
    int is_client = 0;

    /* Get opts */
    int opt;

    picoquic_config_init(&config);
    memcpy(option_string, "", 0);
    ret = picoquic_config_option_letters(option_string + 0, sizeof(option_string) - 0, NULL);

    if (ret == 0) {
        /* Get the parameters */
        while ((opt = getopt(argc, argv, option_string)) != -1) {
            switch (opt) {
            default:
                if (picoquic_config_command_line(opt, &optind, argc, (char const **)argv, optarg, &config) != 0) {
                    usage();
                }
                break;
            }
        }
    }

    /* Simplified style params */
    if (optind < argc) {
        server_name = argv[optind++];
        is_client = 1;
    }

    if (optind < argc) {
        if ((server_port = atoi(argv[optind++])) <= 0) {
            fprintf(stderr, "Invalid port: %s\n", optarg);
            usage();
        }
    }

    if (optind < argc) {
        usage();
    }

    int is_name = 0;
    if (is_client) {
        ret = picoquic_get_server_address(server_name, server_port, &server_sockaddr, &is_name);
    }

    picoquic_config_set_option(&config, picoquic_option_ALPN, "qe2ed");

    /* Create picoquic quic. */
    picoquic_quic_t* quic = NULL;
    uint64_t current_time = picoquic_current_time();
    if (is_client) {
        /* client mode */
        quic = picoquic_create_and_configure(&config, NULL, NULL, current_time, NULL);
        //quic = picoquic_create(1, NULL, NULL, NULL, "qe2ed", NULL, NULL,
        //    NULL, NULL, NULL, current_time, NULL, ticket_store_filename, NULL, 0);
    } else {
        /* server mode */

        if (config.server_cert_file == NULL &&
            picoquic_get_input_path(default_server_cert_file, sizeof(default_server_cert_file), config.solution_dir, SERVER_CERT_FILE) == 0) {
            /* Using set option call to ensure proper memory management*/
            picoquic_config_set_option(&config, picoquic_option_CERT, default_server_cert_file);
            }

        if (config.server_key_file == NULL &&
            picoquic_get_input_path(default_server_key_file, sizeof(default_server_key_file), config.solution_dir, SERVER_KEY_FILE) == 0) {
            /* Using set option call to ensure proper memory management*/
            picoquic_config_set_option(&config, picoquic_option_KEY, default_server_key_file);
            }

        quic = picoquic_create_and_configure(&config, qe2ed_server_callback, qe2ed, current_time, NULL);
        //quic = picoquic_create(256, "./certs/cert.pem", "./certs/key.pem", NULL, "qe2ed", qe2ed_server_callback,
        //    qe2ed, NULL, NULL, NULL, current_time, NULL, NULL, NULL, 0);
    }

    picoquic_set_key_log_file_from_env(quic);

    if (config.qlog_dir != NULL)
    {
        picoquic_set_qlog(quic, config.qlog_dir);
    }

    if (config.performance_log != NULL)
    {
        picoquic_perflog_setup(quic, config.performance_log);
    }

    /* Configure picoquic. */
    if (quic == NULL) {
        fprintf(stderr, "Could not create context.");
        ret = -1;
    } else {
        /* both */
        picoquic_set_default_idle_timeout(quic, 0); /* infinite timeout. */

        if (is_client) {
            /* client mode */
            qe2ed->cnx = picoquic_create_cnx(quic, picoquic_null_connection_id, picoquic_null_connection_id,
                (struct sockaddr *) &server_sockaddr, current_time, 0, "qe2ed.lan", "qe2ed", 1);
            /* Set the client callback context. */
            picoquic_set_callback(qe2ed->cnx, qe2ed_client_callback, qe2ed);
            ret = picoquic_start_client_cnx(qe2ed->cnx);
        } else {
            /* server mode */
        }
    }

    /* Start network thread. */
    if (ret == 0) {
        picoquic_packet_loop_param_t param = { 0 };
        param.local_port = (uint16_t)config.server_port;
        param.local_af = 0;
        param.dest_if = 0;
        param.socket_buffer_size = 0;
        param.do_not_use_gso = 0;

        if (is_client) {
            qe2ed->network_thread_ctx = picoquic_start_network_thread(quic, &param, qe2ed_client_loop_callback, qe2ed, &ret);
        } else {
            qe2ed->network_thread_ctx = picoquic_start_network_thread(quic, &param, qe2ed_server_loop_callback, qe2ed, &ret);
        }
        if (ret == 0) {
            /* Wait until picoquic network thread exits. */
            picoquic_wait_thread((picoquic_thread_t)qe2ed->network_thread_ctx->pthread);
            //picoquic_delete_thread(qe2ed->network_thread_ctx->pthread);
        }

        printf("qe2ed exit, ret = %d\n", ret);
    }

    /* Clean up */
    if (quic != NULL) {
        picoquic_free(quic);
    }

    if (qe2ed != NULL) {
        qe2ed_free(qe2ed);
    }

    return ret;
}
