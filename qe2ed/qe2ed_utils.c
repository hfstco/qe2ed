//
// Created by Hofstätter, Matthias on 29.01.25.
//

#include "qe2ed_utils.h"

int qe2ed_open_csv_log(picoquic_cnx_t *cnx, FILE **file) {
    int ret = 0;

    /* Get connection id. */
    char cid_name[2 * PICOQUIC_CONNECTION_ID_MAX_SIZE + 1];
    ret = picoquic_print_connection_id_hexa(cid_name, sizeof(cid_name), &cnx->initial_cnxid);

    /* Build filename from connection id. */
    char log_filename[512];
    if (ret == 0) {
      ret = picoquic_sprintf(log_filename, sizeof(log_filename), NULL, "%s.%s.csv",
              cid_name,
              (cnx->client_mode) ? "client" : "server");
    }

    /* Open log file. */
    if (ret == 0) {
          *file = fopen(log_filename, "a");
          if (*file == NULL) {
              fprintf(stdout, "Can't open file %s: %d\n", log_filename, errno);
              fflush(stdout);
              ret = -1;
          }
    }

    return ret;
}
