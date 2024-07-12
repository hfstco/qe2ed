//
// Created by Hofstätter, Matthias on 08.07.24.
//

#include "qe2ed.h"
#include "qe2ed_server.h"

#include <assert.h>

int qe2ed_server_callback(picoquic_cnx_t *cnx,
                          uint64_t stream_id, uint8_t *bytes, size_t length,
                          picoquic_call_back_event_t fin_or_event, void *callback_ctx, void *v_stream_ctx) {
    int ret = 0;

    qe2ed_t *qe2ed = (qe2ed_t *) callback_ctx;
    qe2ed_server_context_t *ctx = (qe2ed_server_context_t *) v_stream_ctx;

    switch (fin_or_event) {
        case picoquic_callback_stream_data:
        case picoquic_callback_stream_fin: {
            //fprintf(stdout, "picoquic_callback_stream_data length=%" PRIu64 "\n", length);
            if (ctx == NULL) {
                ctx = malloc(sizeof(qe2ed_server_context_t));
                ctx->buffer = malloc(length + sizeof(uint64_t));
                ctx->size = length + sizeof(uint64_t);
            }

            uint64_t current_time = picoquic_current_time();
            memcpy(ctx->buffer, &current_time, sizeof(uint64_t));
            memcpy(ctx->buffer + sizeof(uint64_t), bytes, length);

            fprintf(stdout, "-> [%" PRIu64 "][%" PRIu64 "] %s", *(uint64_t *) ctx->buffer,
                    *(uint64_t *) (ctx->buffer + sizeof(uint64_t)),
                    ctx->buffer + 2 * sizeof(uint64_t));

            picoquic_mark_active_stream(cnx, stream_id, 1, ctx);
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
        }
        break;
        case picoquic_callback_version_negotiation:
            break;
        case picoquic_callback_stream_gap:
            break;
        case picoquic_callback_prepare_to_send: {
            assert(length > ctx->size + sizeof(uint64_t));

            uint8_t *buffer = picoquic_provide_stream_data_buffer(bytes, ctx->size + sizeof(uint64_t), 0, 0);
            if (buffer != NULL) {
                uint64_t current_time = picoquic_current_time();
                memcpy(buffer, &current_time, sizeof(uint64_t));
                memcpy(buffer + sizeof(uint64_t), ctx->buffer, ctx->size);
            }

            //fprintf(stdout, "picoquic_callback_prepare_to_send length=%" PRIu64 "\n", ctx->size + sizeof(uint64_t));
            fprintf(stdout, "<- [%" PRIu64 "][%" PRIu64 "][%" PRIu64 "] %s\n", *(uint64_t *) buffer,
                    *(uint64_t *) (buffer + sizeof(uint64_t)),
                    *(uint64_t *) (buffer + 2 * sizeof(uint64_t)), buffer + 3 * sizeof(uint64_t));

            free(ctx->buffer);
            free(ctx);
            picoquic_set_app_stream_ctx(cnx, stream_id, NULL);
        }
        break;
        case picoquic_callback_almost_ready:
            break;
        case picoquic_callback_ready:
            fprintf(stdout, "New connection.\n");
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

int qe2ed_server_loop_callback(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode,
    void* callback_ctx, void * callback_arg) {
    int ret = 0;

    qe2ed_t* qe2ed = (qe2ed_t*)callback_ctx;

    switch (cb_mode) {
        case picoquic_packet_loop_ready:
            //printf("picoquic_packet_loop_ready\n");
                break;
        case picoquic_packet_loop_after_receive:
        break;
        case picoquic_packet_loop_after_send:
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
