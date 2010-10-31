#include <stdarg.h>

#include "xmlrpc_config.h"

#include "bool.h"

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>
#include <xmlrpc-c/client_int.h>
#include <xmlrpc-c/client_global.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*=========================================================================
   Global Client
=========================================================================*/
#define MAX_SERVER 100

static struct xmlrpc_client * globalClientP;
static bool globalClientExists = false;
void* sync_func( void* t ); 



typedef struct __SyncThreadArg{
pthread_t tid;
int threadid;
xmlrpc_env * envP; 
char* serverUrl;
const char* methodName;
const char* format;
va_list args;
}SyncThreadArg; 


xmlrpc_value *result[MAX_SERVER];
pthread_mutex_t sync_write_result;


void 
xmlrpc_client_init2(xmlrpc_env *                      const envP,
                    int                               const flags,
                    const char *                      const appname,
                    const char *                      const appversion,
                    const struct xmlrpc_clientparms * const clientparmsP,
                    unsigned int                      const parmSize) {
/*----------------------------------------------------------------------------
   This function is not thread-safe.

-----------------------------------------------------------------------------*/
    if (globalClientExists)
        xmlrpc_faultf(
            envP,
            "Xmlrpc-c global client instance has already been created "
            "(need to call xmlrpc_client_cleanup() before you can "
            "reinitialize).");
    else {
        /* The following call is not thread-safe */
        xmlrpc_client_setup_global_const(envP);
        if (!envP->fault_occurred) {
            xmlrpc_client_create(envP, flags, appname, appversion,
                                 clientparmsP, parmSize, &globalClientP);
            if (!envP->fault_occurred)
                globalClientExists = true;

            if (envP->fault_occurred)
                xmlrpc_client_teardown_global_const();
        }
    }
}



void
xmlrpc_client_init(int          const flags,
                   const char * const appname,
                   const char * const appversion) {
/*----------------------------------------------------------------------------
   This function is not thread-safe.
-----------------------------------------------------------------------------*/
    struct xmlrpc_clientparms clientparms;

    /* As our interface does not allow for failure, we just fail silently ! */
    
    xmlrpc_env env;
    xmlrpc_env_init(&env);

    clientparms.transport = NULL;

    /* The following call is not thread-safe */
    xmlrpc_client_init2(&env, flags,
                        appname, appversion,
                        &clientparms, XMLRPC_CPSIZE(transport));

    xmlrpc_env_clean(&env);
}



void 
xmlrpc_client_cleanup() {
/*----------------------------------------------------------------------------
   This function is not thread-safe
-----------------------------------------------------------------------------*/
    XMLRPC_ASSERT(globalClientExists);

    xmlrpc_client_destroy(globalClientP);

    globalClientExists = false;

    /* The following call is not thread-safe */
    xmlrpc_client_teardown_global_const();
}



static void
validateGlobalClientExists(xmlrpc_env * const envP) {

    if (!globalClientExists)
        xmlrpc_faultf(envP,
                      "Xmlrpc-c global client instance "
                      "has not been created "
                      "(need to call xmlrpc_client_init2()).");
}



void
xmlrpc_client_transport_call(
    xmlrpc_env *               const envP,
    void *                     const reserved ATTR_UNUSED, 
        /* for client handle */
    const xmlrpc_server_info * const serverP,
    xmlrpc_mem_block *         const callXmlP,
    xmlrpc_mem_block **        const respXmlPP) {

    validateGlobalClientExists(envP);
    if (!envP->fault_occurred)
        xmlrpc_client_transport_call2(envP, globalClientP, serverP,
                                      callXmlP, respXmlPP);
}



static void
clientCall_va(xmlrpc_env *               const envP,
              const xmlrpc_server_info * const serverInfoP,
              const char *               const methodName,
              const char *               const format,
              va_list                          args,
              xmlrpc_value **            const resultPP) {

    validateGlobalClientExists(envP);
    if (!envP->fault_occurred) {
        xmlrpc_value * paramArrayP;
        const char * suffix;
        
        xmlrpc_build_value_va(envP, format, args, &paramArrayP, &suffix);
        
        if (!envP->fault_occurred) {
            if (*suffix != '\0')
                xmlrpc_faultf(envP, "Junk after the argument "
                              "specifier: '%s'.  "
                              "There must be exactly one arument.",
                              suffix);
            else
                xmlrpc_client_call2(envP, globalClientP, serverInfoP,
                                    methodName, paramArrayP, resultPP);
            
            xmlrpc_DECREF(paramArrayP);
        }
    }
}

xmlrpc_value * 
xmlrpc_client_call(xmlrpc_env * const envP,
                   const char * const serverUrl,
                   const char * const methodName,
                   const char * const format,
                   ...) {


    xmlrpc_value * resultP ;
    
    xmlrpc_server_info * serverInfoP;

    serverInfoP = xmlrpc_server_info_new(envP, serverUrl);
        
    if (!envP->fault_occurred) {
        va_list args;
        va_start(args, format);
        clientCall_va(envP, serverInfoP, methodName, format, args, &resultP);
	
        va_end(args);
        xmlrpc_server_info_free(serverInfoP);
    }    
    
    return resultP;
}

void* sync_func( void* t ) 
{
    SyncThreadArg *arg = (SyncThreadArg*)t; 
    printf("REQUESTING FROM URL:%s\n",arg->serverUrl);
 
    xmlrpc_server_info * serverInfoP;    
    xmlrpc_value * resultP ;

    serverInfoP = xmlrpc_server_info_new(arg->envP, arg->serverUrl);
       
    if (!arg->envP->fault_occurred) {
 
        clientCall_va(arg->envP, serverInfoP, arg->methodName, arg->format, arg->args, &resultP);
	
        xmlrpc_server_info_free(serverInfoP);
    }    

    pthread_mutex_lock(&sync_write_result);
    result[arg->threadid] = resultP;  
    pthread_mutex_unlock(&sync_write_result);
    printf("Wrote Results from %d\n",arg->threadid);
    pthread_exit(0);
}

xmlrpc_value *
xmlrpc_client_call_multi_sync(int semantics ,int nUrl , xmlrpc_env * const envP,
                   const char ** const serverUrl,
                   const char *  const methodName,
                   const char *  const format,
		   ...) {
xmlrpc_value *resultP;


int i = 0;
//printf("Sending Request to URL : %s\n",serverUrl[0]);
#if 1
SyncThreadArg * syncthreadarg = (SyncThreadArg*) malloc (sizeof(SyncThreadArg) * nUrl);

/*Making the Threads Joinable*/
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

va_list args;
va_start(args, format);

pthread_mutex_init(&sync_write_result, NULL);

for(i = 0; i < nUrl ; i++ ) 
{
syncthreadarg[i].threadid = i;
syncthreadarg[i].envP = envP;
syncthreadarg[i].serverUrl = serverUrl[i];
syncthreadarg[i].methodName = methodName;
syncthreadarg[i].format = format;
syncthreadarg[i].args = args;

pthread_create(&(syncthreadarg[i].tid),&attr,sync_func,(void*) &syncthreadarg[i]);

}

va_end(args);

pthread_attr_destroy(&attr);

for(i=0; i < nUrl; i++) 
pthread_join(syncthreadarg[i].tid, NULL);

printf("ALL Threads Complete & Joined\n");

//resultP = xmlrpc_client_call(envP, serverUrl[0],methodName,format);
#endif

return result[4];
}

xmlrpc_value * 
xmlrpc_client_call_server(xmlrpc_env *               const envP,
                          const xmlrpc_server_info * const serverP,
                          const char *               const methodName,
                          const char *               const format, 
                          ...) {

    va_list args;
    xmlrpc_value * resultP;

    va_start(args, format);
    clientCall_va(envP, serverP, methodName, format, args, &resultP);
    va_end(args);

    return resultP;
}



xmlrpc_value *
xmlrpc_client_call_server_params(
    xmlrpc_env *               const envP,
    const xmlrpc_server_info * const serverInfoP,
    const char *               const methodName,
    xmlrpc_value *             const paramArrayP) {

    xmlrpc_value * resultP;

    validateGlobalClientExists(envP);

    if (!envP->fault_occurred)
        xmlrpc_client_call2(envP, globalClientP,
                            serverInfoP, methodName, paramArrayP,
                            &resultP);

    return resultP;
}



xmlrpc_value * 
xmlrpc_client_call_params(xmlrpc_env *   const envP,
                          const char *   const serverUrl,
                          const char *   const methodName,
                          xmlrpc_value * const paramArrayP) {

    xmlrpc_value * resultP;

    validateGlobalClientExists(envP);

    if (!envP->fault_occurred) {
        xmlrpc_server_info * serverInfoP;

        serverInfoP = xmlrpc_server_info_new(envP, serverUrl);
        
        if (!envP->fault_occurred) {
            xmlrpc_client_call2(envP, globalClientP,
                                serverInfoP, methodName, paramArrayP,
                                &resultP);
            
            xmlrpc_server_info_free(serverInfoP);
        }
    }
    return resultP;
}                            



void 
xmlrpc_client_call_server_asynch_params(
    xmlrpc_server_info * const serverInfoP,
    const char *         const methodName,
    xmlrpc_response_handler    responseHandler,
    void *               const userData,
    xmlrpc_value *       const paramArrayP) {

    xmlrpc_env env;

    xmlrpc_env_init(&env);

    validateGlobalClientExists(&env);

    if (!env.fault_occurred)
        xmlrpc_client_start_rpc(&env, globalClientP,
                                serverInfoP, methodName, paramArrayP,
                                responseHandler, userData);

    if (env.fault_occurred) {
        /* Unfortunately, we have no way to return an error and the
           regular callback for a failed RPC is designed to have the
           parameter array passed to it.  This was probably an oversight
           of the original asynch design, but now we have to be as
           backward compatible as possible, so we do this:
        */
        (*responseHandler)(serverInfoP->serverUrl,
                           methodName, paramArrayP, userData,
                           &env, NULL);
    }
    xmlrpc_env_clean(&env);
}



void 
xmlrpc_client_call_asynch(const char * const serverUrl,
                          const char * const methodName,
                          xmlrpc_response_handler responseHandler,
                          void *       const userData,
                          const char * const format,
                          ...) {

    xmlrpc_env env;

    xmlrpc_env_init(&env);

    validateGlobalClientExists(&env);

    if (!env.fault_occurred) {
        xmlrpc_value * paramArrayP;
        const char * suffix;
        va_list args;
    
        va_start(args, format);
        xmlrpc_build_value_va(&env, format, args, &paramArrayP, &suffix);
        va_end(args);
    
        if (!env.fault_occurred) {
            if (*suffix != '\0')
                xmlrpc_faultf(&env, "Junk after the argument "
                              "specifier: '%s'.  "
                              "There must be exactly one arument.",
                              suffix);
            else
                xmlrpc_client_call_asynch_params(
                    serverUrl, methodName, responseHandler, userData,
                    paramArrayP);
        }
    }
    if (env.fault_occurred)
        (*responseHandler)(serverUrl, methodName, NULL, userData, &env, NULL);

    xmlrpc_env_clean(&env);
}



void
xmlrpc_client_call_asynch_params(const char *   const serverUrl,
                                 const char *   const methodName,
                                 xmlrpc_response_handler responseHandler,
                                 void *         const userData,
                                 xmlrpc_value * const paramArrayP) {
    xmlrpc_env env;
    xmlrpc_server_info * serverInfoP;

    xmlrpc_env_init(&env);

    serverInfoP = xmlrpc_server_info_new(&env, serverUrl);

    if (!env.fault_occurred) {
        xmlrpc_client_call_server_asynch_params(
            serverInfoP, methodName, responseHandler, userData, paramArrayP);
        
        xmlrpc_server_info_free(serverInfoP);
    }
    if (env.fault_occurred)
        (*responseHandler)(serverUrl, methodName, paramArrayP, userData,
                           &env, NULL);
    xmlrpc_env_clean(&env);
}



void 
xmlrpc_client_call_server_asynch(xmlrpc_server_info * const serverInfoP,
                                 const char *         const methodName,
                                 xmlrpc_response_handler    responseHandler,
                                 void *               const userData,
                                 const char *         const format,
                                 ...) {

    xmlrpc_env env;
    xmlrpc_value * paramArrayP;
    const char * suffix;
    va_list args;
    
    xmlrpc_env_init(&env);

    va_start(args, format);
    xmlrpc_build_value_va(&env, format, args, &paramArrayP, &suffix);
    va_end(args);

    if (!env.fault_occurred) {
        if (*suffix != '\0')
            xmlrpc_faultf(&env, "Junk after the argument "
                          "specifier: '%s'.  "
                          "There must be exactly one arument.",
                          suffix);
        else
            xmlrpc_client_call_server_asynch_params(
                serverInfoP, methodName, responseHandler, userData,
                paramArrayP);

        xmlrpc_DECREF(paramArrayP);
    }
    if (env.fault_occurred)
        (*responseHandler)(serverInfoP->serverUrl, methodName, NULL,
                           userData, &env, NULL);

    xmlrpc_env_clean(&env);
}



void 
xmlrpc_client_event_loop_finish_asynch(void) {

    XMLRPC_ASSERT(globalClientExists);
    xmlrpc_client_event_loop_finish(globalClientP);
}



void 
xmlrpc_client_event_loop_finish_asynch_timeout(
    unsigned long const milliseconds) {

    XMLRPC_ASSERT(globalClientExists);
    xmlrpc_client_event_loop_finish_timeout(globalClientP, milliseconds);
}
