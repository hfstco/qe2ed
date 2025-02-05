//
// Created by Hofstätter, Matthias on 08.07.24.
//

#ifndef QE2ED_INTERNAL_H
#define QE2ED_INTERNAL_H
#include <picoquic.h>
#include <picoquic_packet_loop.h>

/*
 * Global qe2ed context. For client and server.
 */
typedef struct st_qe2ed_t {
    picoquic_cnx_t* cnx;
    picoquic_network_thread_ctx_t* network_thread_ctx;
} qe2ed_t;

/*
 * Create qe2ed context.
 */
qe2ed_t* qe2ed_create();

/*
 * Free qe2ed context.
 */
void qe2ed_free(qe2ed_t* qe2ed);



#endif //QE2ED_INTERNAL_H
