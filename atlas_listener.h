/*
 *Header file for the atlas listern
 */

#ifndef _atlas_listener_h_
#define _atlas_listener_h_

#include "atlas_router.h"
#include "atlasb_pool.h"

typedef struct atlas_listener glb_listener_t;

extern atlas_listener_t*
atlas_listener_create (const glb_cnf_t* cnf,
                     atlas_router_t*    router,
                     atlas_pool_t*      pool,
                     int              listen_sock);

extern void
atlas_listener_destroy (glb_listener_t* listener);

#endif // _glb_listener_h_
