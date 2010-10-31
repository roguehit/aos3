/* A simple synchronous XML-RPC client written in C, as an example of an
   Xmlrpc-c client.  This invokes the sample.add procedure that the Xmlrpc-c
   example xmlrpc_sample_add_server.c server provides.  I.e. it adds two
   numbers together, the hard way.

   This sends the RPC to the server running on the local system ("localhost"),
   HTTP Port 8080.
*/

#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "prime.h"
#include <openssl/ssl.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>
#include <xmlrpc-c/config.h>  /* information about this build environment */


#define NAME "Xmlrpc-c Test Client"
#define VERSION "1.0"

static void 
dieIfFaultOccurred (xmlrpc_env * const envP) {
    if (envP->fault_occurred) {
        fprintf(stderr, "ERROR: %s (%d)\n",
                envP->fault_string, envP->fault_code);
        exit(1);
    }
}

xmlrpc_value* generate_request_array ( );

const char* extract_result ( );


int 
main(int           const argc, 
     const char ** const argv) {

    if(argc < 2 )
    {
    fprintf(stderr,"Usage : ./client <Semantics> <No of Servers>\n");
    exit(EXIT_FAILURE);
    }

    const char const **serverUrl;
    int i = 0, port = 8080 ;
    xmlrpc_env env;
    xmlrpc_value * resultP;
    int nUrl = atoi(argv[2]);

    if(nUrl == 0)
    {
    fprintf(stderr,"Please enter non-zero number of servers\n");
    exit(EXIT_FAILURE);
    }

#if 0   
    serverUrl = (char const**) malloc (nUrl * sizeof(char*));
    for(i = 0 ; i < nUrl ; i++){
    serverUrl[i] = (char *) malloc(50 * sizeof(char));
    sprintf(serverUrl[i],"http://localhost:%d/RPC2",port++);
    }
#endif
#if 1 
        serverUrl = malloc(sizeof(char *) * 5);
        serverUrl[0] = "http://localhost:8080/RPC2";
        serverUrl[1] = "http://localhost:8081/RPC2";
        serverUrl[2] = "http://localhost:8082/RPC2";
        serverUrl[3] = "http://localhost:8083/RPC2";
        serverUrl[4] = "http://localhost:8084/RPC2"; 
#endif    
    const char * const methodName = "calculate_modexp";
    //const char * const serverUrl = "http://localhost:8080/RPC2";
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);

    /* Start up our XML-RPC client library. */
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
    dieIfFaultOccurred(&env);

 //    printf("Making XMLRPC call to server url '%s' method '%s' "           "to request the sum "           "of 5 and 7...\n", serverUrl, methodName);

    /* Make the remote procedure call */
   // printf("Result is : %d\n",xmlrpc_validate_utf8("Hello World"));
   xmlrpc_value *arr = xmlrpc_array_new(&env);
        
   //resultP = xmlrpc_client_call(&env, serverUrl, methodName,"(A)", generate_request_array(&env , arr) );
   resultP = xmlrpc_client_call_multi_sync(atoi(argv[1]),nUrl, &env, serverUrl, methodName,"(A)", generate_request_array(&env , arr));	
   printf("Checking for Death\n");
   dieIfFaultOccurred(&env);
    
    /* Get our sum and print it out. */
   //xmlrpc_read_int(&env, resultP, &primeResult);
  // xmlrpc_env *   const dummy;
   printf("Extracting Results\n");
   const char* result = extract_result ( &env , resultP);    

   dieIfFaultOccurred(&env);

   printf("2^p/m = %s\n", result);
   
   /*Dispose of our result value. */
   xmlrpc_DECREF(resultP);

    /* Clean up our error-handling environment. */
   xmlrpc_env_clean(&env);
    
    /* Shutdown our XML-RPC client library. */
   xmlrpc_client_cleanup();

   return 0;
}

xmlrpc_value* generate_request_array ( xmlrpc_env *env , xmlrpc_value *arr)
{
  
   Task task = gen_prime (task);

   xmlrpc_value *v1 = xmlrpc_build_value(env, "s", task._clientid);
   xmlrpc_value *v2 = xmlrpc_build_value(env, "s", task.p);
   xmlrpc_value *v3 = xmlrpc_build_value(env, "s", task.m);

   xmlrpc_array_append_item (env,arr,v1);
   xmlrpc_array_append_item (env,arr,v2);
   xmlrpc_array_append_item (env,arr,v3);
   
   return arr; 
}
const char* extract_result (xmlrpc_env *env , xmlrpc_value *temp)
{
   xmlrpc_value * arrayP;	
   xmlrpc_decompose_value(env, temp, "(A)", &arrayP);
   size_t size = xmlrpc_array_size(env, arrayP);
   xmlrpc_value * strctP;
   strctP = xmlrpc_array_get_item(env, arrayP, 0);
   const char *primeResult = (const char *)malloc (256);
   size_t str1_len;
   xmlrpc_read_string_lp(env, strctP, &str1_len, &primeResult);			
   
   return primeResult;
}
