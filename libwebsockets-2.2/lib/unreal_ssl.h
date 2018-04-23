// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#ifdef __cplusplus
extern "C" 
{
#endif

#ifndef DISPATCH_FUNCTION
#define DISPATCH_FUNCTION(DISPATCH_TABLE, NAME) (DISPATCH_TABLE->_##NAME)
#endif

#if defined(USE_UNREAL_SSL)

struct FUnrealSSLDispatchTable
{
	void* (*_ctx_new)(void*);
	void (*_ctx_free)(void*);
	void* (*_new)(void*);
	int (*_connect)(void*);
	int (*_shutdown)(void*);
	void (*_free)(void*);
	int (*_pending)(void*);
	int (*_set_socketfd)(void*, int);
	int (*_read)(void*, void*, int);
	int (*_write)(void*, const void*, int);
	int (*_set_hostname)(void*, const char*, int);
	int (*_get_srand)(void*, int);
	int (*_get_error)(void*, int);
};

extern struct FUnrealSSLDispatchTable* GUnrealSSLDispatchTable;
extern void unreal_set_unreal_ssl_table(struct FUnrealSSLDispatchTable*);

#define SSL_CTX_new DISPATCH_FUNCTION(GUnrealSSLDispatchTable, ctx_new)
#define SSL_CTX_free DISPATCH_FUNCTION(GUnrealSSLDispatchTable, ctx_free)
#define SSL_new DISPATCH_FUNCTION(GUnrealSSLDispatchTable, new)
#define SSL_connect DISPATCH_FUNCTION(GUnrealSSLDispatchTable, connect)
#define SSL_shutdown DISPATCH_FUNCTION(GUnrealSSLDispatchTable, shutdown)
#define SSL_free DISPATCH_FUNCTION(GUnrealSSLDispatchTable, free)
#define SSL_set_socketfd DISPATCH_FUNCTION(GUnrealSSLDispatchTable, set_socketfd)
#define SSL_pending DISPATCH_FUNCTION(GUnrealSSLDispatchTable, pending)
#define SSL_read DISPATCH_FUNCTION(GUnrealSSLDispatchTable, read)
#define SSL_write DISPATCH_FUNCTION(GUnrealSSLDispatchTable, write)
#define SSL_set_hostname DISPATCH_FUNCTION(GUnrealSSLDispatchTable, set_hostname)
#define SSL_get_srand DISPATCH_FUNCTION(GUnrealSSLDispatchTable, get_srand)
#define SSL_get_error DISPATCH_FUNCTION(GUnrealSSLDispatchTable, get_error)
#define ERR_get_error(...) 0
#define ERR_error_string_n(...)
#define ERR_error_string(...) ""
#define UNREAL_SSL_ERROR_WOULDBLOCK -2

#endif

#ifdef __cplusplus
}
#endif