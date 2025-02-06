//
// Created by Hofstätter, Matthias on 08.07.24.
//

#ifndef QE2ED_CLIENT_H
#define QE2ED_CLIENT_H

#include <picoquic.h>
#include <picoquic_packet_loop.h>
#include "qe2ed_utils.h"

/*
 * Client context
 */
typedef struct st_qe2ed_client_context_t {
    /* # of ping pong packets left. */
    int nb_pp_packets_left;
} qe2ed_client_context_t;

/*
 * Create client context.
 */
qe2ed_client_context_t *qe2ed_create_client_context(int nb_pp_frames);

/*
 * Free client context.
 */
void qe2ed_free_client_context(qe2ed_client_context_t *ctx);

/*
 * picoquic client callback.
 */
int qe2ed_client_callback(picoquic_cnx_t* cnx,
    uint64_t stream_id, uint8_t* bytes, size_t length,
    picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);

/*
 * picoquic client loop callback.
 */
int qe2ed_client_loop_callback(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode,
    void* callback_ctx, void * callback_arg);



#endif //QE2ED_CLIENT_H
