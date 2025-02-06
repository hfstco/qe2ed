//
// Created by Hofstätter, Matthias on 08.07.24.
//

#include <string.h>
#include <stdlib.h>
#include "qe2ed_internal.h"

qe2ed_t* qe2ed_create() {
    qe2ed_t* qeq = malloc(sizeof(qe2ed_t));
    memset(qeq, 0, sizeof(qe2ed_t));

    qeq->nb_pp_packets = 20;

    return qeq;
}

void qe2ed_free(qe2ed_t* qe2ed) {
    free(qe2ed);
}
