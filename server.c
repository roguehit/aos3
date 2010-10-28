/* A simple standalone XML-RPC server written in C. */

/* This server knows one RPC class (besides the system classes):
   "sample.add".

   The program takes one argument: the HTTP port number on which the server
   is to accept connections, in decimal.

   You can use the example program 'xmlrpc_sample_add_client' to send an RPC
   to this server.

   Example:

   $ ./xmlrpc_sample_add_server 8080&
   $ ./xmlrpc_sample_add_client

   For more fun, run client and server in separate terminals and turn on
   tracing for each:

   $ export XMLRPC_TRACE_XML=1
*/

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include "queue.h"

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/config.h>  /* information about this build environment */
#include <xmlrpc-c/server_abyss.h>
#include <openssl/ssl.h>


#define SLEEP(seconds) sleep(seconds);
void compute_mod( );

static xmlrpc_value *
sample_add(xmlrpc_env *   const env,
           xmlrpc_value * const param_array,
           void *         const serverInfo,
           void *         const channelInfo) {

   int i = 0;
   Task temp;
   xmlrpc_value * retval;

   xmlrpc_value * arrayP;	
	
   xmlrpc_decompose_value(env, param_array, "(A)", &arrayP);

   size_t size = xmlrpc_array_size(env, arrayP);

	if (env->fault_occurred)
		retval = NULL;
	else 
	{

		for (i = 0; i < size && !env->fault_occurred; ++i) 
		{
			xmlrpc_value * strctP;
			strctP = xmlrpc_array_get_item(env, arrayP, i);
			if (!env->fault_occurred) 
			{
				if(i == 0)
				{
					const char * str1;
					size_t str1_len;
					xmlrpc_read_string_lp(env, strctP, &str1_len, &str1);				
	
					//xmlrpc_int32 curly;
					//xmlrpc_decompose_value(env, strctP, "(s)",&ip[0]);
					strcpy(temp._clientid, str1);
				}
				else
				{
					const char * str1;
					size_t str1_len;
					xmlrpc_read_string_lp(env, strctP, &str1_len, &str1);				
				
					if(i == 1)
					strcpy(temp.p,str1);
				
					else if(i == 2)
					strcpy(temp.m,str1);

				}
			}
		}
	}

    xmlrpc_DECREF(arrayP);

    if (env->fault_occurred)
        return NULL;

    printf("Received: %s %s %s\n",temp._clientid,temp.p,temp.m);

    /* Add our two numbers. */
   // z = x + y;

    /* Sometimes, make it look hard (so client can see what it's like
       to do an RPC that takes a while).
    */
    compute_mod(&temp);
  
    xmlrpc_value *arr = xmlrpc_array_new(env);
    xmlrpc_value *v1 = xmlrpc_build_value(env, "s", temp.response);
    xmlrpc_array_append_item (env,arr,v1);

    /* Return our result. */
    return xmlrpc_build_value(env,"(A)", arr);
}



int 
main(int           const argc, 
     const char ** const argv) {

    struct xmlrpc_method_info3 const methodInfo = {
        /* .methodName     = */ "sample.add",
        /* .methodFunction = */ &sample_add,
    };
    xmlrpc_server_abyss_parms serverparm;
    xmlrpc_registry * registryP;
    xmlrpc_env env;
 
    if (argc-1 != 1) {
        fprintf(stderr, "You must specify 1 argument:  The TCP port "
                "number on which the server will accept connections "
                "for RPCs (8080 is a common choice).  "
                "You specified %d arguments.\n",  argc-1);
        exit(1);
    }
    
    xmlrpc_env_init(&env);

    registryP = xmlrpc_registry_new(&env);

    xmlrpc_registry_add_method3(&env, registryP, &methodInfo);

    /* In the modern form of the Abyss API, we supply parameters in memory
       like a normal API.  We select the modern form by setting
       config_file_name to NULL: 
    */
    serverparm.config_file_name = NULL;
    serverparm.registryP        = registryP;
    serverparm.port_number      = atoi(argv[1]);
    serverparm.log_file_name    = "/tmp/xmlrpc_log";

    printf("Running XML-RPC server...\n");

    xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));

    /* xmlrpc_server_abyss() never returns */

    return 0;
}

void compute_mod(Task * task)
{
	BN_CTX *context;
       	BIGNUM *r,*a,*p,*m;
        if(task->p==NULL){
	fprintf(stderr,"Wrong Exponent\n");
	exit(0);
        }
	
	context = BN_CTX_new();
        r = BN_new();
	a = BN_new();
	p = BN_new();
	m = BN_new();

	BN_dec2bn(&a,"2");
	BN_dec2bn(&p,task->p);
	BN_dec2bn(&m,task->m);
	BN_mod_exp(r,a,p,m,context);

	strcpy(task->response,BN_bn2dec(r));
	
	return;
}

