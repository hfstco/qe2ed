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
#include <picoquic_internal.h>

#include "qe2ed_internal.h"

qe2ed_client_context_t *qe2ed_create_client_context(int nb_pp_frames) {
    qe2ed_client_context_t *client_ctx = malloc(sizeof(qe2ed_client_context_t));
    client_ctx->nb_pp_packets_left = nb_pp_frames;
}

void qe2ed_free_client_context(qe2ed_client_context_t *ctx) {
    free(ctx);
}

int qe2ed_client_callback(picoquic_cnx_t* cnx,
                          uint64_t stream_id, uint8_t* bytes, size_t length,
                          picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx) {
    int ret = 0;

    qe2ed_t* qe2ed = (qe2ed_t*)callback_ctx;
    qe2ed_client_context_t* ctx = (qe2ed_client_context_t*)v_stream_ctx;

    switch (fin_or_event) {
        /*
         * stream_data and stream_fin is called if new incoming data received. stream_fin is called if the fin flag is
         * set.
         */
        case picoquic_callback_stream_data:
        case picoquic_callback_stream_fin: {
                //fprintf(stdout, "picoquic_callback_stream_data length=%" PRIu64 "\n", length);
                uint64_t current_time = picoquic_current_time();
                fprintf(stdout, "-> [%" PRIu64 "][%" PRIu64 "][%" PRIu64 "][%" PRIu64 "]\n",
                    picoquic_current_time(), *(uint64_t *)bytes, *(uint64_t *)(bytes + sizeof(uint64_t)), *(uint64_t *)(bytes + 2 * sizeof(uint64_t)));

                /* Log */
                FILE *file;
                ret = qe2ed_open_csv_log(cnx, &file);
                if (ret == 0) {
                    fprintf(file, "%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 "\n", picoquic_current_time(),
                    *(uint64_t *)bytes, *(uint64_t *)(bytes + sizeof(uint64_t)), *(uint64_t *)(bytes + 2 * sizeof(uint64_t)));
                    fflush(file);
                    fclose(file);
                }

                ctx->nb_pp_packets_left--;
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
                /* Free context. */
                qe2ed_free_client_context(ctx);

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
        /*
         * prepare_to_send is called if picoquic is ready to send new data.
         */
        case picoquic_callback_prepare_to_send:
            if (ctx->nb_pp_packets_left <= 0) {
                picoquic_close(cnx, 0);
            } else {
                uint8_t* buffer = picoquic_provide_stream_data_buffer(bytes, 4 * sizeof(uint64_t), 0, 0);
                if (buffer != NULL) {
                    uint64_t current_time = picoquic_current_time();
                    //uint64_t diff_time = current_time - cnx->start_time;
                    memcpy(buffer, &current_time, sizeof(uint64_t));
                    //memcpy(buffer + sizeof(uint64_t), input, strlen(input));

                    //fprintf(stdout, "picoquic_callback_prepare_to_send length=%" PRIu64 "\n", strlen(input) + 1 + sizeof(uint64_t));
                    fprintf(stdout, "<- [%" PRIu64 "]\n", *(uint64_t *)buffer);
                    fflush(stdout);
                }
            }
            break;
        case picoquic_callback_almost_ready:
            break;
        /*
         * ready is called if connection is ready to use.
         */
        case picoquic_callback_ready:
            fprintf(stdout, "Client ready. Type \"exit\" to close connection and shutdown client.\n");
            fprintf(stdout, "[CLIENT_RECV_TIME][SERVER_SENT_TIME][SERVER_RECV_TIME][CLIENT_SENT_TIME] <DATA>\n");

            /* Log */
            FILE *file;
            ret = qe2ed_open_csv_log(cnx, &file);
            if (ret == 0) {
                fprintf(file, "CLIENT_RECV_TIME,SERVER_SENT_TIME,SERVER_RECV_TIME,CLIENT_SENT_TIME\n");
                fflush(file);
                fclose(file);
            }

            /* Create client context. */
            ctx = qe2ed_create_client_context(qe2ed->nb_pp_packets);

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
