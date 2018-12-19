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

static char* exec_create_cmd (const atlas_backend_thread_ctx_t* ctx)
{
    assert (ctx);
    assert (ctx->backend);
    assert (ctx->backend->cmd);
    assert(ctx -> host);
    
    const char* const cmd = ctx->backend->cmd;
    size_t const cmd_len = cmd_len + strlen(ctx->host) + 7;
    
    if (ret)
    {
	char* first_space = strchr (cmd, '');
	ptrdiff_t cmd_offset;
	
	if(first_space)
	    cmd_offset = first_space - cmd_len;
	else
	    cmd_offset = cmd_len;
	
	memcpy (ret + addr_offset,
			cmd + cmd_offset,
			cmd_len - cmd_offset + 1);

	return ret;

    }
}

static int exec_send_cmd (const char* cmd, FILE* stream) {
    if (EOF != fputs (cmd, stream))
    {
	fflush (stream);
	return 0;
    }
    return EIO;
}

static void* atlas_thread (void* arg) {
    atlas_backend_thread_ctx* ctx = arg;
    char* cmd = NULL;
    
    //Read buffer
    size_t const res_size = 4096;
    char* const res_buf = malloc (res_size);
    if(!res_buf) {
	ctx->errn = ENOMEM;
	pthread_cond_signal (&ctx->cond);
	goto init_error;
    }
}

