//
// Created by Hofstätter, Matthias on 08.07.24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qe2ed.h"
#include "qe2ed_client.h"

#include <assert.h>
#include <inttypes.h>

#include "qe2ed_internal.h"

qe2ed_client_context_t *qe2ed_create_client_context(size_t buffer_size) {
    qe2ed_client_context_t *client_ctx = malloc(sizeof(qe2ed_client_context_t));
    client_ctx->buffer = malloc(buffer_size);
    return client_ctx;
}

void qe2ed_free_client_context(qe2ed_client_context_t *ctx) {
    free(ctx->buffer);
    free(ctx);
}

int qe2ed_client_callback(picoquic_cnx_t* cnx,
                          uint64_t stream_id, uint8_t* bytes, size_t length,
                          picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx) {
    int ret = 0;

    qe2ed_t* qe2ed = (qe2ed_t*)callback_ctx;
    qe2ed_client_context_t* ctx = (qe2ed_client_context_t*)v_stream_ctx;

    switch (fin_or_event) {
        case picoquic_callback_stream_data:
        case picoquic_callback_stream_fin: {
                //fprintf(stdout, "picoquic_callback_stream_data length=%" PRIu64 "\n", length);
                uint64_t current_time = picoquic_current_time();
                fprintf(stdout, "-> [%" PRIu64 "][%" PRIu64 "][%" PRIu64 "][%" PRIu64 "] %.*s\n", current_time,
                    *(uint64_t *)bytes, *(uint64_t *)(bytes + sizeof(uint64_t)), *(uint64_t *)(bytes + 2 * sizeof(uint64_t)),
                    length - 2 * sizeof(uint64_t), bytes + 3 * sizeof(uint64_t));

                picoquic_mark_active_stream(cnx, stream_id, 1, NULL);
            }
            break;
        case picoquic_callback_stop_sending:
            break;
        case picoquic_callback_stream_reset:
            break;
        case picoquic_callback_stateless_reset:
        case picoquic_callback_application_close:
        case picoquic_callback_close: {
                uint64_t local_error, remote_error, local_application_error, remote_application_error = 0;
                picoquic_get_close_reasons(cnx, &local_error, &remote_error, &local_application_error,
                                           &remote_application_error);
                printf(
                    "local_error=%" PRIu64 ", remote_error=%" PRIu64 ", local_application_error=%" PRIu64
                    ", remote_application_error=%" PRIu64 "\n", local_error, remote_error, local_application_error,
                    remote_application_error);

                picoquic_set_callback(cnx, NULL, NULL);
            }
        case picoquic_callback_version_negotiation:
            break;
        case picoquic_callback_stream_gap:
            break;
        case picoquic_callback_prepare_to_send: {
                assert(length > sizeof(uint64_t));

                char *input = malloc(length - sizeof(uint64_t));
                fgets(input, length - sizeof(uint64_t), stdin);

                if (strcmp(input, "exit\n") == 0) {
                    ret = picoquic_close(cnx, 0);
                    break;
                }

                uint8_t* buffer = picoquic_provide_stream_data_buffer(bytes, sizeof(uint64_t) + strlen(input) + 1, 0, 0);
                if (buffer != NULL) {
                    uint64_t current_time = picoquic_current_time();
                    memcpy(buffer, &current_time, sizeof(uint64_t));
                    memcpy(buffer + sizeof(uint64_t), input, strlen(input) + 1);

                    //fprintf(stdout, "picoquic_callback_prepare_to_send length=%" PRIu64 "\n", strlen(input) + 1 + sizeof(uint64_t));
                    fprintf(stdout, "<- [%" PRIu64 "] %s", *(uint64_t *)buffer, input);
                }

                free(input);
            }
            break;
        case picoquic_callback_almost_ready:
            break;
        case picoquic_callback_ready:
            fprintf(stdout, "Client ready.\n");
            picoquic_mark_active_stream(cnx, stream_id, 1, ctx);
            break;
        case picoquic_callback_path_available:
            break;
        case picoquic_callback_path_suspended:
            break;
        case picoquic_callback_path_deleted:
            break;
        case picoquic_callback_path_quality_changed:
            break;
        default:
            /* just ignore. */
            break;
    }

    return ret;
}

int qe2ed_client_loop_callback(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode,
    void* callback_ctx, void * callback_arg) {
    int ret = 0;

    qe2ed_t* qe2ed = (qe2ed_t*)callback_ctx;

    switch (cb_mode) {
        case picoquic_packet_loop_ready:
            //printf("picoquic_packet_loop_ready\n");
                break;
        case picoquic_packet_loop_after_receive:
            if (picoquic_get_cnx_state(qe2ed->cnx) == picoquic_state_disconnected) {
                ret = PICOQUIC_NO_ERROR_TERMINATE_PACKET_LOOP;
            }
        break;
        case picoquic_packet_loop_after_send:
            if (picoquic_get_cnx_state(qe2ed->cnx) == picoquic_state_disconnected) {
                ret = PICOQUIC_NO_ERROR_TERMINATE_PACKET_LOOP;
            }
        break;
        case picoquic_packet_loop_port_update:
            //printf("picoquic_packet_loop_port_update\n");
                break;
        case picoquic_packet_loop_time_check:
            //printf("picoquic_packet_loop_time_check\n");
                break;
        case picoquic_packet_loop_wake_up:
            //printf("picoquic_packet_loop_wake_up\n");
                break;
        default:
            break;
    }

    return ret;
}
