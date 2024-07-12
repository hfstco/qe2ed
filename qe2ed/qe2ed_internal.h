//
// Created by Hofstätter, Matthias on 08.07.24.
//

#ifndef QE2ED_INTERNAL_H
#define QE2ED_INTERNAL_H
#include <picoquic.h>
#include <picoquic_packet_loop.h>

typedef struct st_qe2ed_t {
    picoquic_cnx_t* cnx;
    picoquic_network_thread_ctx_t* network_thread_ctx;
} qe2ed_t;

qe2ed_t* qe2ed_create();
void qe2ed_free(qe2ed_t* qe2ed);



#endif //QE2ED_INTERNAL_H
