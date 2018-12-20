/*
 * Example implementation of a dummy backend that does nothing.
 */

#include "atlas_backend.h"

const char* atlas_dst_state_str[] =
{
    "NOTFOUND", "NOTREADY", "   AVOID", "   READY"
};

#include <stdlib.h>   // calloc()/free()/abort()
#include <string.h>   // strdup()
#include <errno.h>    // ENOMEM
#include <sys/time.h> // gettimeofday()

void
atlas_backend_probe (glb_backend_thread_ctx_t* const ctx,
                   atlas_wdog_check_t*         const res,
                   const struct timespec*    const until)
{
    memset (res, 0, sizeof(*res));
    res->state = GLB_DST_NOTFOUND;
    

    if (!ctx->quit && !ctx->join)
    {
        ctx->waiting++;
        pthread_cond_signal (&ctx->cond);
        if (!pthread_cond_timedwait (&ctx->cond, &ctx->lock, until))
            *res = ctx->result;
    }

}

/*! Sample dummy backend context. */
struct atlas_backend_ctx
{
    const char* spec; // just for example
};

static void
backend_dummy_destroy (atlas_backend_ctx_t* ctx)
{
    free ((void*)ctx->spec);
    free (ctx);
}

static inline void
backend_timespec_add (struct timespec* t, long long i)
{
    i += t->tv_nsec;
    t->tv_sec += i / 1000000000;
    t->tv_nsec = i % 1000000000;
}

/*! Example backend thread that does nothing, just reports Atlas_DST_READY at
 *  scheduled intervals. */
static void*
backend_dummy_thread (void* arg)
{
    glb_backend_thread_ctx_t* ctx = arg;

    struct timeval now;
    gettimeofday (&now, NULL);

    struct timespec next = { /* establish starting point */
        .tv_sec  = now.tv_sec,
        .tv_nsec = now.tv_usec * 1000
    };

    /* failure to lock/unlock the mutex is absolutely fatal */
    if (pthread_mutex_lock (&ctx->lock)) abort();

    pthread_cond_signal (&ctx->cond); // watchdog is waiting for this signal

    while (!ctx->quit) /* main loop */
    {
#if SKIP
        if (pthread_mutex_unlock (&ctx->lock)) abort();

        /* here we should be polling the destination - backend-specific part */

        if (pthread_mutex_lock (&ctx->lock)) abort();
#endif

        ctx->result.state   = GLB_DST_READY;// destination always ready
        ctx->result.latency = 1.0;          // same latency for all destinations
        ctx->result.others  = NULL;         // no auto-discovered destinations
        ctx->result.others_len = 0;
        ctx->result.ready   = true;         // new data ready

        backend_timespec_add (&next, ctx->interval); // next wakeup

        /* this unlocks the context and watchdog can have full access to it */
        pthread_cond_timedwait (&ctx->cond, &ctx->lock, &next);
        /* here the context is locked again */
    }

    ctx->join = true; /* ready to be joined */

    if (pthread_mutex_unlock(&ctx->lock)) abort();

    return NULL;
}

static int
backend_dummy_init (atlas_backend_t* backend, const char* spec)
{
    atlas_backend_ctx_t* ctx = calloc (1, sizeof(*ctx));

    if (!ctx) return -ENOMEM;

    if (spec) {
        ctx->spec = strdup(spec);
        if (!ctx->spec) {
            free (ctx);
            return -ENOMEM;
        }
    }
    else {
        ctx->spec = NULL;
    }

    backend->ctx     = ctx;
    backend->thread  = backend_dummy_thread;
    backend->destroy = backend_dummy_destroy;

    return 0;
}

atlas_backend_init_t atlas_backend_dummy_init = backend_dummy_init;

