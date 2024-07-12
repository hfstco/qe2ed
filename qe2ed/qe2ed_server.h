//
// Created by Hofstätter, Matthias on 08.07.24.
//

#ifndef QE2ED_SERVER_H
#define QE2ED_SERVER_H

#include <picoquic.h>
#include <picoquic_packet_loop.h>

typedef struct st_qe2ed_server_context_t {
    uint8_t* buffer;
    size_t size;
    size_t offset;
} qe2ed_server_context_t;


int qe2ed_server_callback(picoquic_cnx_t* cnx,
    uint64_t stream_id, uint8_t* bytes, size_t length,
    picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);

int qe2ed_server_loop_callback(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode,
    void* callback_ctx, void * callback_arg);



#endif //QE2ED_SERVER_H
