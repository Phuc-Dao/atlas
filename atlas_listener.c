/*
 */

#include "atlas_listener.h"
#include "atlas_log.h"
#include "atlas_limits.h"
#include "atlas_cmd.h"
#include "atlas_misc.h"

#include <sys/socket.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <poll.h>

#ifndef _GNU_SOURCE
#include <fcntl.h>
#endif /* _GNU_SOURCE */

typedef struct pollfd pollfd_t;

struct atlas_listener
{
    const atlas_cnf_t* cnf;
    atlas_router_t* router;
    atlas_pool_t*   pool;
    pthread_t     thread;
    int           sock;
};

static void*
listener_thread (void* arg)
{
    atlas_listener_t* listener = arg;

    while (!atlas_terminate) {
        int            ret;
        int            client_sock;
        atlas_sockaddr_t client;
        socklen_t      client_size = sizeof(client);
        int            server_sock;
        atlas_sockaddr_t server;

#if defined(_GNU_SOURCE) && defined(SOCK_CLOEXEC)
        client_sock = accept4(listener->sock,
                              (struct sockaddr*) &client, &client_size,
                              SOCK_CLOEXEC);
#else
        client_sock = accept (listener->sock,
                              (struct sockaddr*) &client, &client_size);
#endif /* _GNU_SOURCE && SOCK_CLOEXEC*/

        if (client_sock < 0 || atlas_terminate) {
            if (client_sock < 0) {
                atlas_log_error ("Failed to accept connection: %d (%s)",
                               errno, strerror (errno));
            }
            else {
                atlas_log_debug ("Listener thread termonating.");
            }

            goto err;
        }

#if !defined(_GNU_SOURCE) || !defined(SOCK_CLOEXEC)
	(void) atlas_fd_setfd (client_sock, FD_CLOEXEC, true);
#endif /* !_GNU_SOURCE || !SOCK_CLOEXEC */

        ret = atlas_router_connect(listener->router, &client ,&server,
                                 &server_sock);
        if (server_sock < 0 && ret != -EINPROGRESS) {
            if (server_sock != -EMFILE)
                atlas_log_error("Failed to connect to destination: %d (%s)",
                              -ret, strerror(-ret));
            goto err1;
        }

        assert (0 == ret || -EINPROGRESS == ret);

        atlas_socket_setopt(client_sock, GLB_SOCK_NODELAY); // ignore error here

        ret = atlas_pool_add_conn (listener->pool,
                                 client_sock, &client,
                                 server_sock, &server,
                                 0 == ret);
        if (ret < 0) {
            atlas_log_error ("Failed to add connection to pool: "
                           "%d (%s)", -ret, strerror (-ret));
            goto err2;
        }

        if (listener->cnf->verbose) {
            atlas_sockaddr_str_t ca = glb_sockaddr_to_str (&client);
            atlas_sockaddr_str_t sa = glb_sockaddr_to_str (&server);
            atlas_log_info ("Accepted connection from %s to %s\n", ca.str,sa.str);
        }
        continue;

    err2:
        assert (server_sock > 0);
        close  (server_sock);
        atlas_router_disconnect (listener->router, &server, false);

    err1:
        assert (client_sock > 0);
        close  (client_sock);

    err:
        // to avoid busy loop in case of error
        if (!atlas_terminate) usleep (100000);
    }

    return NULL;
}

atlas_listener_t*
atlas_listener_create (const glb_cnf_t* const cnf,
                     atlas_router_t*    const router,
                     atlas_pool_t*      const pool,
                     int              const sock)
{
    atlas_listener_t* ret = NULL;

    if (listen (sock,
                cnf->max_conn ? cnf->max_conn : (1U << 14)/* 16K */)){
        atlas_log_error ("listen() failed: %d (%s)", errno, strerror (errno));
        return NULL;
    }

    ret = calloc (1, sizeof (atlas_listener_t));
    if (ret) {
        ret->cnf    = cnf;
        ret->router = router;
        ret->pool   = pool;
        ret->sock   = sock;

        if (pthread_create (&ret->thread, NULL, listener_thread, ret)) {
            atlas_log_error ("Failed to launch listener thread: %d (%s)",
                           errno, strerror (errno));
            free (ret);
            ret = NULL;
        }
    }
    else
    {
        atlas_log_error ("Failed to allocate listener object: %d (%s)",
                       errno, strerror (errno));
    }

    return ret;
}

extern void
atlas_listener_destroy (glb_listener_t* listener)
{
    /* need to connect to own socket to break the accept() call */
    atlas_sockaddr_t sockaddr;
    atlas_sockaddr_init (&sockaddr, "0.0.0.0", 0);
    int socket = atlas_socket_create (&sockaddr, 0);
    if (socket >= 0)
    {
        int err = connect (socket, (struct sockaddr*)&listener->cnf->inc_addr,
                           sizeof (listener->cnf->inc_addr));
        close (socket);
        if (err) {
            atlas_log_error ("Failed to connect to listener socket: %d (%s)",
                           errno, strerror(errno));
            atlas_log_error ("glb_listener_destroy(): failed to join thread.");
        }
        else {
            pthread_join (listener->thread, NULL);
        }
    }
    else {
        atlas_log_error ("Failed to create socket: %d (%s)",
                       -socket, strerror(-socket));
    }
    free (listener);
}

