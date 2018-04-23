// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "private-libwebsockets.h"

//@UE4 BEGIN - Allow using function pointers instead of linking for the SSL and socket APIs.

#if defined(USE_SOCKETAPI_DISPATCH)
struct FSocketAPIDispatchTable* GSocketAPIDispatchTable;

LWS_VISIBLE void
unreal_set_socketapi_table(struct FSocketAPIDispatchTable* table)
{
	GSocketAPIDispatchTable = table;
}
#endif

#if defined(USE_UNREAL_SSL)
struct FUnrealSSLDispatchTable* GUnrealSSLDispatchTable;

LWS_VISIBLE void
unreal_set_unreal_ssl_table(struct FUnrealSSLDispatchTable* table)
{
	GUnrealSSLDispatchTable = table;
}
#endif

//@UE4 END - Allow using function pointers instead of linking for the SSL and socket APIs.