/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 *    server.c
 *
 *    (C) Copyright 2015 Olivier Delhomme
 *     e-mail : olivier.delhomme@free.fr
 *
 *    "Sauvegarde" is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    "Sauvegarde" is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this project.  If not, see <http://www.gnu.org/licenses/>
 */
#define MHD_PLATFORM_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <errno.h>


/**
 * @struct upload_t
 * @brief Structure to manage uploads of data in POST command.
 *
 */
typedef struct
{
    char *buffer; /**< buffer that will grab all upload_data from MHD_ahc callback       */
    long pos;     /**< position in the buffer (at the end it is the size of that buffer) */
    long number;  /**< number of upload_data buffers received                            */
} upload_t;


/**
 * @param connection is the connection in MHD
 * @returns in bytes the value (a guint) of the Content-Length: header
 */
static long get_content_length(struct MHD_Connection *connection)
{
    const char *length = NULL;
    long len = 9888777;

    length = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Content-Length");

    if (length != NULL)
        {
            if (sscanf(length, "%ld", &len) <= 0)
                {
                    fprintf(stderr, "Could not guess Content-Length header value: %s\n", strerror(errno));
                    len = 9888777;
                }

            if (len < 0 || len > 4294967296)
                {
                    len = 9888777;
                }
        }

    return len;
}


/**
 * MHD_AccessHandlerCallback function that manages all connections requests
 */
static int ahc(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    int success = MHD_NO;
    upload_t *pp = (upload_t *) *con_cls;
    long len = 0;
    struct MHD_Response *response = NULL;
    char *answer = NULL;

    /* fprintf(stderr, "%p %p %s %s %s %p %zd\n", cls, connection, url, method, version, upload_data, *upload_data_size); */

    if (pp == NULL)
        {
            /* Initialzing the structure at first connection       */
            len = get_content_length(connection);
            pp = (upload_t *) malloc(sizeof(upload_t));
            pp->pos = 0;
            pp->buffer = malloc(sizeof(char) * (len + 1));
            pp->number = 0;
            *con_cls = pp;

            success = MHD_YES;
        }
    else if (*upload_data_size != 0)
        {
            /* Getting data whatever they are */
            memcpy(pp->buffer + pp->pos, upload_data, *upload_data_size);
            pp->pos = pp->pos + *upload_data_size;

            pp->number = pp->number + 1;

            *con_cls = pp;
            *upload_data_size = 0;

            success = MHD_YES;
        }
    else
        {
            /* reset when done */
            *con_cls = NULL;
            pp->buffer[pp->pos] = '\0';

            /* to investigate libmicrohttpd shrinkage */
            if (pp->number != 0)
                {
                    fprintf(stderr, "%ld %ld %ld\n", pp->pos, pp->number, pp->pos / pp->number);
                }

            /* Do something with the  datas */
            usleep(300);

            free(pp->buffer);
            free(pp);

            answer = (char *) malloc (8);
            sprintf(answer, "Ok.");
            response = MHD_create_response_from_buffer(strlen(answer), (void *) answer, MHD_RESPMEM_MUST_FREE);
            success = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
        }

    return success;
}


int main(int argc, char **argv)
{
    struct MHD_Daemon *d;

    d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG, 6666, NULL, NULL, &ahc, NULL, MHD_OPTION_CONNECTION_MEMORY_LIMIT, (size_t) 131070 , MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120, MHD_OPTION_END);

     if (d == NULL)
        {
            fprintf(stderr,"Error while spawning libmicrohttpd daemon\n");
            return 1;
        }

    (void) getc(stdin);
    MHD_stop_daemon(d);

    return 0;
}
