#include <jimautoconf.h>
#include <prj_compat.h>
#include <jim-api.h>

// =============================================================================================

#include <jim-aio-ext.h>

BEGIN_JIM_NAMESPACE

#if defined(HAVE_SOCKETS) && !defined(JIM_BOOTSTRAP) // #optionalCode #WinOff

union sockaddr_any {
    struct sockaddr sa;
    struct sockaddr_in sin;
#if IPV6 // #optionalCode #WinOff
    struct sockaddr_in6 sin6;
#endif
};

#ifndef HAVE_INET_NTOP // #optionalCode #WinOff
const char* inet_ntop(int af, const void* src, char* dst, int size) // #TODO
{
    if (af != PF_INET) {
        return NULL;
    }
    snprintf(dst, size, "%s", inet_ntoa(((struct sockaddr_in*)src)->sin_addr));
    return dst;
}
#endif

static int JimParseIPv6Address(Jim_InterpPtr interp, const char* hostport, union sockaddr_any* sa, int* salen) {
#if IPV6 // #optionalCode #WinOff
    /*
     * An IPv6 addr/port looks like:
     *   [::1]
     *   [::1]:2000
     *   [fe80::223:6cff:fe95:bdc0%en1]:2000
     *   [::]:2000
     *   2000
     *
     *   Note that the "any" address is ::, which is the same as when no address is specified.
     */
    char* sthost = NULL;
    const char* stport;
    int ret = JIM_OK;
    struct addrinfo req;
    struct addrinfo* ai;

    stport = strrchr(hostport, ':');
    if (!stport) {
        /* No : so, the whole thing is the port */
        stport = hostport;
        hostport = "::";
        sthost = Jim_StrDup(hostport);
    } else {
        stport++;
    }

    if (*hostport == '[') {
        /* This is a numeric ipv6 address */
        char* pt = strchr((char*)++hostport, ']');
        if (pt) {
            sthost = Jim_StrDupLen(hostport, pt - hostport);
        }
    }

    if (!sthost) {
        sthost = Jim_StrDupLen(hostport, stport - hostport - 1);
    }

    memset(&req, '\0', sizeof(req));
    req.ai_family = PF_INET6;

    if (prj_getaddrinfo(sthost, NULL, &req, &ai)) { // #NonPortFuncFix #SockFunc
        Jim_SetResultFormatted(interp, "Not a valid address: %s", hostport);
        ret = JIM_ERR;
    } else {
        memcpy(&sa->sin, ai->ai_addr, ai->ai_addrlen);
        *salen = ai->ai_addrlen;

        sa->sin.sin_port = htons(atoi(stport));

        prj_freeaddrinfo(ai); // #NonPortFuncFix #SockFunc
    }
    Jim_TFree<char>(sthost); // #FreeF 

    return ret;
#else
    Jim_SetResultString(interp, "ipv6 not supported", -1);
    return JIM_ERR;
#endif
}

static int JimParseIpAddress(Jim_InterpPtr interp, const char* hostport, union sockaddr_any* sa, int* salen) {
    /* An IPv4 addr/port looks like:
     *   192.168.1.5
     *   192.168.1.5:2000
     *   2000
     *
     * If the address is missing, INADDR_ANY is used.
     * If the port is missing, 0 is used (only useful for server sockets).
     */
    char* sthost = NULL;
    const char* stport;
    int ret = JIM_OK;

    stport = strrchr(hostport, ':');
    if (!stport) {
        /* No : so, the whole thing is the port */
        stport = hostport;
        sthost = Jim_StrDup("0.0.0.0");
    } else {
        sthost = Jim_StrDupLen(hostport, stport - hostport);
        stport++;
    }

    {
#ifdef HAVE_GETADDRINFO // #optionalCode #WinOff
        struct addrinfo req;
        struct addrinfo* ai;
        memset(&req, '\0', sizeof(req));
        req.ai_family = PF_INET;

        if (prj_getaddrinfo(sthost, NULL, &req, &ai)) { // #NonPortFuncFix #SockFunc
            ret = JIM_ERR;
        } else {
            memcpy(&sa->sin, ai->ai_addr, ai->ai_addrlen);
            *salen = ai->ai_addrlen;
            prj_freeaddrinfo(ai); // #NonPortFuncFix #SockFunc
        }
#else /* HAVE_GETADDRINFO */
        struct hostent* he;

        ret = JIM_ERR;

        if ((he = gethostbyname(sthost)) != NULL) {
            if (he->h_length == sizeof(sa->sin.sin_addr)) {
                *salen = sizeof(sa->sin);
                sa->sin.sin_family = he->h_addrtype;
                memcpy(&sa->sin.sin_addr, he->h_addr, he->h_length); /* set address */
                ret = JIM_OK;
            }
        }
#endif /* HAVE_GETADDRINFO */

        sa->sin.sin_port = htons(atoi(stport));
    }
    Jim_TFree<char>(sthost); // #FreeF 

    if (ret != JIM_OK) {
        Jim_SetResultFormatted(interp, "Not a valid address: %s", hostport);
    }

    return ret;
}

#ifdef HAVE_SYS_UN_H // #optionalCode #WinOff
static int JimParseDomainAddress(Jim_InterpPtr interp, const char* path, struct sockaddr_un* sa) {
    sa->sun_family = PF_UNIX;
    snprintf(sa->sun_path, sizeof(sa->sun_path), "%s", path);

    return JIM_OK;
}
#endif /* HAVE_SYS_UN_H */

/**
 * Format that address in 'sa' as a string and store in variable 'varObjPtr'
 */
static int JimFormatIpAddress(Jim_InterpPtr interp, Jim_Obj* varObjPtr, const union sockaddr_any* sa) {
    /* INET6_ADDRSTRLEN is 46. Add some for [] and port */
    char addrbuf[60];

#if IPV6 // #optionalCode #WinOff
    if (sa->sa.sa_family == PF_INET6) {
        addrbuf[0] = '[';
        /* Allow 9 for []:65535\0 */
        inet_ntop(sa->sa.sa_family, &sa->sin6.sin6_addr, addrbuf + 1, sizeof(addrbuf) - 9); // #NonPortFunc #SockFunc
        snprintf(addrbuf + strlen(addrbuf), 8, "]:%d", ntohs(sa->sin.sin_port));
    } else
#endif /* IPV6 */
        if (sa->sa.sa_family == PF_INET) {
            /* Allow 7 for :65535\0 */
            inet_ntop(sa->sa.sa_family, &sa->sin.sin_addr, addrbuf, sizeof(addrbuf) - 7); // #NonPortFunc #SockFunc
            snprintf(addrbuf + strlen(addrbuf), 7, ":%d", ntohs(sa->sin.sin_port));
        } else {
            /* recvfrom still works on unix domain sockets, etc */
            addrbuf[0] = 0;
        }

    return Jim_SetVariable(interp, varObjPtr, Jim_NewStringObj(interp, addrbuf, -1));
}

static Retval aio_cmd_recvfrom(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp);
    char* buf;
    union sockaddr_any sa;
    long len;
    socklen_t salen = sizeof(sa);
    int rlen;

    if (Jim_GetLong(interp, argv[0], &len) != JIM_OK) {
        return JIM_ERR;
    }

    buf = Jim_TAlloc<char>(len + 1); // #AllocF 

    rlen = prj_recvfrom(prj_fileno(af->fp), buf, len, 0, &sa.sa, &salen); // #NonPortFuncFix
    if (rlen < 0) {
        Jim_TFree<char>(buf); // #FreeF
        JimAioSetError(interp, NULL);
        return JIM_ERR;
    }
    buf[rlen] = 0;
    Jim_SetResult(interp, Jim_NewStringObjNoAlloc(interp, buf, rlen));

    if (argc > 1) {
        return JimFormatIpAddress(interp, argv[1], &sa);
    }

    return JIM_OK;
}


static Retval aio_cmd_sendto(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp);
    int wlen;
    int len;
    const char* wdata;
    union sockaddr_any sa;
    const char* addr = Jim_String(argv[1]);
    int salen;

    if (IPV6 && af->addr_family == PF_INET6) {
        if (JimParseIPv6Address(interp, addr, &sa, &salen) != JIM_OK) {
            return JIM_ERR;
        }
    } else if (JimParseIpAddress(interp, addr, &sa, &salen) != JIM_OK) {
        return JIM_ERR;
    }
    wdata = Jim_GetString(argv[0], &wlen);

    /* Note that we don't validate the socket type. Rely on sendto() failing if appropriate */
    len = prj_sendto(prj_fileno(af->fp), wdata, wlen, 0, &sa.sa, salen); // #NonPortFuncFix #SockFunc
    if (len < 0) {
        JimAioSetError(interp, NULL);
        return JIM_ERR;
    }
    Jim_SetResultInt(interp, len);
    return JIM_OK;
}

static Retval aio_cmd_accept(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp);
    int sock;
    union sockaddr_any sa;
    socklen_t addrlen = sizeof(sa);

    sock = prj_accept(af->fd, &sa.sa, &addrlen); // #NonPortFuncFix #SockFunc
    if (sock < 0) {
        JimAioSetError(interp, NULL);
        return JIM_ERR;
    }

    if (argc > 0) {
        if (JimFormatIpAddress(interp, argv[0], &sa) != JIM_OK) {
            return JIM_ERR;
        }
    }

    /* Create the file command */
    return JimMakeChannel(interp, NULL, sock, Jim_NewStringObj(interp, "accept", -1),
                          "aio.sockstream%ld", af->addr_family, "r+") ? JIM_OK : JIM_ERR;
}

static Retval aio_cmd_listen(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp);
    long backlog;

    if (Jim_GetLong(interp, argv[0], &backlog) != JIM_OK) {
        return JIM_ERR;
    }

    if (prj_listen(af->fd, backlog)) { // #NonPortFuncFix #SockFunc
        JimAioSetError(interp, NULL);
        return JIM_ERR;
    }

    return JIM_OK;
}

enum {
    SOCKOPT_BOOL = 0,
    SOCKOPT_INT = 1,
    SOCKOPT_TIMEVAL = 2   /* not currently supported */
};

static const struct sockopt_def {
    const char* name;
    int level;
    int opt;
    int type;   /* SOCKOPT_xxx */
} g_sockopts[] = {
#ifdef SOL_SOCKET // #optionalCode #WinOff
#ifdef SO_BROADCAST // #optionalCode #WinOff
    { "broadcast", SOL_SOCKET, SO_BROADCAST },
#endif
#ifdef SO_DEBUG // #optionalCode #WinOff
    { "debug", SOL_SOCKET, SO_DEBUG },
#endif
#ifdef SO_KEEPALIVE // #optionalCode #WinOff
    { "keepalive", SOL_SOCKET, SO_KEEPALIVE },
#endif
#ifdef SO_NOSIGPIPE // #optionalCode #WinOff
    { "nosigpipe", SOL_SOCKET, SO_NOSIGPIPE },
#endif
#ifdef SO_OOBINLINE // #optionalCode #WinOff
    { "oobinline", SOL_SOCKET, SO_OOBINLINE },
#endif
#ifdef SO_SNDBUF // #optionalCode #WinOff
    { "sndbuf", SOL_SOCKET, SO_SNDBUF, SOCKOPT_INT },
#endif
#ifdef SO_RCVBUF // #optionalCode #WinOff
    { "rcvbuf", SOL_SOCKET, SO_RCVBUF, SOCKOPT_INT },
#endif
#if 0 && defined(SO_SNDTIMEO) // #optionalCode #WinOff
    { "sndtimeo", SOL_SOCKET, SO_SNDTIMEO, SOCKOPT_TIMEVAL },
#endif
#if 0 && defined(SO_RCVTIMEO) // #optionalCode #WinOff
    { "rcvtimeo", SOL_SOCKET, SO_RCVTIMEO, SOCKOPT_TIMEVAL },
#endif
#endif
#ifdef IPPROTO_TCP // #optionalCode #WinOff
#ifdef TCP_NODELAY // #optionalCode #WinOff
    { "tcp_nodelay", IPPROTO_TCP, TCP_NODELAY },
#endif
#endif
};

static Retval aio_cmd_sockopt(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp);
    int i;

    if (argc == 0) {
        Jim_Obj* dictObjPtr = Jim_NewListObj(interp, NULL, 0);
        for (i = 0; i < sizeof(g_sockopts) / sizeof(*g_sockopts); i++) {
            int value = 0;
            socklen_t len = sizeof(value);
            if (prj_getsockopt(af->fd, g_sockopts[i].level, g_sockopts[i].opt, (void*) &value, &len) == 0) { // #NonPortFuncFix #SockFunc
                if (g_sockopts[i].type == SOCKOPT_BOOL) {
                    value = !!value;
                }
                Jim_ListAppendElement(interp, dictObjPtr, Jim_NewStringObj(interp, g_sockopts[i].name, -1));
                Jim_ListAppendElement(interp, dictObjPtr, Jim_NewIntObj(interp, value));
            }
        }
        Jim_SetResult(interp, dictObjPtr);
        return JIM_OK;
    }
    if (argc == 1) {
        return -1;
    }

    /* Set an option */
    for (i = 0; i < sizeof(g_sockopts) / sizeof(*g_sockopts); i++) {
        if (strcmp(Jim_String(argv[0]), g_sockopts[i].name) == 0) {
            int on;
            if (g_sockopts[i].type == SOCKOPT_BOOL) {
                if (Jim_GetBoolean(interp, argv[1], &on) != JIM_OK) {
                    return JIM_ERR;
                }
            } else {
                long longval;
                if (Jim_GetLong(interp, argv[1], &longval) != JIM_OK) {
                    return JIM_ERR;
                }
                on = longval;
            }
            if (prj_setsockopt(af->fd, g_sockopts[i].level, g_sockopts[i].opt, (void*) &on, sizeof(on)) < 0) { // #NonPortFuncFix #SockFunc
                Jim_SetResultFormatted(interp, "Failed to set %#s: %s", argv[0], strerror(errno));
                return JIM_ERR;
            }
            return JIM_OK;
        }
    }
    /* Not found */
    Jim_SetResultFormatted(interp, "Unknown sockopt %#s", argv[0]);
    return JIM_ERR;
}

#endif /* if defined(HAVE_SOCKETS) && !defined(JIM_BOOTSTRAP) */

END_JIM_NAMESPACE
