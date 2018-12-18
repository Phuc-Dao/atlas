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
