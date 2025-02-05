//
// Created by Hofstätter, Matthias on 29.01.25.
//

#ifndef QE2ED_UTILS_H
#define QE2ED_UTILS_H

#include "qe2ed_internal.h"
#include "picoquic_internal.h"

/* Opens .csv log file. Connection ID is used to build filename. */
int qe2ed_open_csv_log(picoquic_cnx_t *cnx, FILE **file);

#endif //QE2ED_UTILS_H
