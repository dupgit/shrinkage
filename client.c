

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <signal.h>
#include <string.h>
#include <poll.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>
#include <sys/fanotify.h>
#include <curl/curl.h>


/**
 * @struct comm_t
 * @brief Structure that will contain everything needed to the
 *        communication layer.
 */
typedef struct
{
    CURL *ch; /**< Curl easy handle for a connection                */
    char *buffer;     /**< Buffer to pass things from the callback function */
    char *readbuffer; /**< Buffer to be read                                */
    char *conn;       /**< Connexion string that should be http://ip:port   */
    ulong seq;         /**< sequence number when receiving multiples parts   */
    ulong pos;          /**< Position in readbuffer                           */
    size_t length;     /**< length of buffer                                 */
} comm_t;




/**
 * Used by libcurl to retrieve informations
 * @param buffer is the buffer where received data are written by libcurl
 * @param size is the size of an element in buffer
 * @param nmemb is the number of elements in buffer
 * @param[in,out] userp is a user pointer and MUST be a pointer to comm_t *
 *                structure
 * @returns should return the size of the data taken into account.
 *          Everything different from the size passed to this function is
 *          considered as an error by libcurl.
 */
static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    comm_t *comm = (comm_t *) userp;

    if (comm != NULL)
        {
            if (comm->seq == 0)
                {

                    comm->buffer = (char *) malloc(size * nmemb);
                    memcpy(comm->buffer, buffer, size * nmemb);
                }

            comm->seq = comm->seq + 1;
        }

    return (size * nmemb);
}




/**
 * Used by libcurl to retrieve informations
 * @param buffer is the buffer where received data are written by libcurl
 * @param size is the size of an element in buffer
 * @param nitems is the number of elements in buffer
 * @param[in,out] userp is a user pointer and MUST be a pointer to comm_t *
 *                structure
 * @returns should return the size of the data taken into account.
 *          Everything different from the size passed to this function is
 *          considered as an error by libcurl.
 */
static size_t read_data(char *buffer, size_t size, size_t nitems, void *userp)
{
    comm_t *comm = (comm_t *) userp;
    size_t whole_size = 0;

    if (comm != NULL)
        {
            if (comm->pos >= comm->length)
                {
                    return 0;
                }
            else
                {
                    whole_size = size * nitems;

                    if ((comm->pos + whole_size) > comm->length)
                        {
                            whole_size = comm->length - comm->pos;
                            memcpy(buffer, comm->readbuffer + comm->pos, whole_size);
                            comm->pos = comm->length;
                            return (whole_size);
                        }
                    else
                        {
                            memcpy(buffer, comm->readbuffer + comm->pos, whole_size);
                            comm->pos = comm->pos + whole_size;
                            return whole_size;
                        }
                }
        }

    return 0;
}

/**
 * Creates a new communication comm_t * structure.
 * @param conn a gchar * connection string that should be some url like
 *        string : http://ip:port or http://servername:port
 * @returns a newly allocated comm_t * structure where sender and receiver
 *          are set to NULL.
 */
comm_t *init_comm_struct(char *conn)
{
    comm_t *comm = NULL;

    comm = (comm_t *) malloc(sizeof(comm_t));

    comm->ch = curl_easy_init();
    comm->buffer = NULL;
    comm->conn = conn;
    comm->readbuffer = NULL;
    comm->pos = 0;
    comm->length = 0;

    return comm;
}


int main(int argc, char **argv)
{
    char *conn = "http://127.0.0.1:6666/blah.json";
    int success = CURLE_FAILED_INIT;
    char *error_buf = NULL;
    char *len = NULL;
    struct curl_slist *chunk = NULL;
    comm_t *comm = NULL;


    curl_global_init(CURL_GLOBAL_ALL);
    comm = init_comm_struct(conn);

    while (1)
        {
            comm->readbuffer = (char *) malloc(5987456);
            comm->seq = 0;
            comm->pos = 0;
            error_buf = (char *) malloc(CURL_ERROR_SIZE + 1);

            comm->length = 5987456;

            curl_easy_reset(comm->ch);
            curl_easy_setopt(comm->ch, CURLOPT_POST, 1);
            curl_easy_setopt(comm->ch, CURLOPT_READFUNCTION, read_data);
            curl_easy_setopt(comm->ch, CURLOPT_READDATA, comm);
            curl_easy_setopt(comm->ch, CURLOPT_URL, conn);
            curl_easy_setopt(comm->ch, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(comm->ch, CURLOPT_WRITEDATA, comm);
            curl_easy_setopt(comm->ch, CURLOPT_ERRORBUFFER, error_buf);
            /* curl_easy_setopt(comm->ch, CURLOPT_VERBOSE, 1L); */

            chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
            len = (char *) malloc(42);
            sprintf(len, "Content-Length: %zd", comm->length);
            chunk = curl_slist_append(chunk, len);
            chunk = curl_slist_append(chunk, "Content-Type: application/json");
            curl_easy_setopt(comm->ch, CURLOPT_HTTPHEADER, chunk);

            success = curl_easy_perform(comm->ch);
            curl_slist_free_all(chunk);

            chunk = NULL;
            free(comm->readbuffer);
            free(comm->buffer);
            free(error_buf);

            usleep(200000);
        }

    return 0;
}
