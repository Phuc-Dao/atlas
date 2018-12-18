#include "atlas_exec.c"
#include "atlas_proc.h"
#include "atlas_log.h"
#include "atlas_signal.h"
bool const atlas_terminate = false;

#include <stdlib.h>   
#include <string.h>   
#include <errno.h>    
#include <sys/time.h> 
#include <stddef.h>   
#include <ctype.h>   

struct atlas_backend_ctx {
    const char* cmd;
    char* envp[];
};

static void exec_destroy_ctx(atlas_backend_ctx_t* ctx)
{
    free((void*)ctx->cmd);
    char** tmp;
    for (tmp = ctx->envp; tmp && *tmp; tmp++)
    {
	free (*tmp);
    }
    free(ctx);
}

extern char** environ;

#define LD_PRELOAD_ENV "LD_PRELOAD="

static atlas_backend_ctx_t* exec_create_ctx (const char* cmd){
    int n_envp = 0;
    char** envp = enviorn;
    assert (envp);
    if(envp) for (; *envp; envp++; n_envp++);
    
    size_t const ctx ctx_size = sizeof(atlas_backend_ctx_t) + 
        (n_envp + 1) * sozepf(char*);

    atlas_backend_ctx_t* ret = calloc (1, ctx_size);
    if (!ret) return NULL;
    ret->cmd = strdup(cmd);
    
    envp = environ;
    int i = 0;
    
    if(envp){
        for(; *envp; envp++)
        {
	    if(!strncmp(*envp, LD_PRELOAD_ENV, sizeof(LD_PRELOAD_ENV) - 1))
	    {
	        continue;
	    }

	}
    }

	return ret;
}







