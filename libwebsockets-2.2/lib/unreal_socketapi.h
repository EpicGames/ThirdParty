// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#ifdef __cplusplus
extern "C" 
{
#endif

#ifndef DISPATCH_FUNCTION
#define DISPATCH_FUNCTION(DISPATCH_TABLE, NAME) (DISPATCH_TABLE->_##NAME)
#endif

#if defined(USE_SOCKETAPI_DISPATCH)

struct FSocketAPIDispatchTable
{
	int (*_shutdown)(int, int);
	ssize_t (*_recv)(int, void*, size_t, int);
	int (*_close)(int);
	int (*_socket)(int, int, int);
	ssize_t (*_send)(int, const void*, size_t, int);
	unsigned short(*_htons)(unsigned short);
	unsigned short(*_ntohs)(unsigned short);
	int (*_bind)(int, const struct sockaddr*, socklen_t);
	int (*_connect)(int, const struct sockaddr*, socklen_t);
	int (*_getaddrinfo)(const char*, const char*, const struct addrinfo*, struct addrinfo**);
	void (*_freeaddrinfo)(struct addrinfo*);
	int (*_getpeername)(int, struct sockaddr*, socklen_t*);
	int (*_getsockname)(int, struct sockaddr*, socklen_t*);
	int (*_setsockopt)(int, int, int, const void*, socklen_t);
	int (*_getnameinfo)(const struct sockaddr*, socklen_t, char*, socklen_t, char*, socklen_t, int);
	int (*_read)(int, void*, size_t);
	int (*_write)(int, const void*, size_t);
	int (*_poll)(struct pollfd*, nfds_t, int);
	int (*_fcntl)(int, int, ...);
	int (*_inet_pton)(int, const char*, void*);
	const char* (*_inet_ntop)(int, const void*, char*, socklen_t);
	int (*_select)(int, fd_set*, fd_set*, fd_set*, struct timeval*);
	int (*_ioctl)(int, unsigned long, ...);
};

extern struct FSocketAPIDispatchTable* GSocketAPIDispatchTable;
extern void unreal_set_socketapi_table(struct FSocketAPIDispatchTable*);

#define shutdown(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, shutdown)(__VA_ARGS__)
#define recv(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, recv)(__VA_ARGS__)
#define close(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, close)(__VA_ARGS__)
#define socket(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, socket)(__VA_ARGS__)
#define send(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, send)(__VA_ARGS__)
#define htons(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, htons)(__VA_ARGS__)
#define ntohs(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, ntohs)(__VA_ARGS__)
#define bind(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, bind)(__VA_ARGS__)
#define connect(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, connect)(__VA_ARGS__)
#define getaddrinfo(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, getaddrinfo)(__VA_ARGS__)
#define freeaddrinfo(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, freeaddrinfo)(__VA_ARGS__)
#define getpeername(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, getpeername)(__VA_ARGS__)
#define getsockname(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, getsockname)(__VA_ARGS__)
#define setsockopt(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, setsockopt)(__VA_ARGS__)
#define getnameinfo(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, getnameinfo)(__VA_ARGS__)
#define read(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, read)(__VA_ARGS__)
#define write(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, write)(__VA_ARGS__)
#define poll(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, poll)(__VA_ARGS__)
#define fcntl(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, fcntl)(__VA_ARGS__)
#define inet_pton(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, inet_pton)(__VA_ARGS__)
#define inet_ntop(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, inet_ntop)(__VA_ARGS__)
#define select(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, select)(__VA_ARGS__)
#define ioctl(...) DISPATCH_FUNCTION(GSocketAPIDispatchTable, ioctl)(__VA_ARGS__)
#define getdtablesize() FOPEN_MAX

#endif

#ifdef __cplusplus
}
#endif