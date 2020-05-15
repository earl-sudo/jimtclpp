#include <jimautoconf.h>
#include <prj_compat.h>
#include <jim-api.h>

// =============================================================================================

#include <jim-aio-ext.h>

#if jim_ext_aio

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
const char* inet_ntop(int af, const void* src, char* dst, int size_) // #TODO Move to prj_compat.c
{
    if (af != PF_INET) {
        return NULL;
    }
    snprintf(dst, size_, "%s", inet_ntoa(((struct sockaddr_in*)src)->sin_addr));
    return dst;
}
#endif

static int JimParseIPv6Address(Jim_InterpPtr interp_, const char* hostport, union sockaddr_any* sa, int* salen) {
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
        hostport = "::"; // #MagicStr
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
        Jim_SetResultFormatted(interp_, "Not a valid address: %s", hostport); // #ErrStr
        ret = JIM_ERR;
    } else {
        memcpy(&sa->sin, ai->ai_addr, ai->ai_addrlen);
        *salen = ai->ai_addrlen;

        sa->sin.sin_port = htons(atoi(stport));

        prj_freeaddrinfo(ai); // #NonPortFuncFix #SockFunc
    }
    free_CharArray(sthost); // #FreeF 

    return ret;
#else
    Jim_SetResultString(interp_, "ipv6 not supported", -1); // #ErrStr
    return JIM_ERR;
#endif
}

static int JimParseIpAddress(Jim_InterpPtr interp_, const char* hostport, union sockaddr_any* sa, int* salen) {
    /* An IPv4 addr/port looks like:
     *   192.168.1.5
     *   192.168.1.5:2000
     *   2000
     *
     * If the address is missing_, INADDR_ANY is used.
     * If the port is missing_, 0 is used (only useful for server sockets).
     */
    char* sthost = NULL;
    const char* stport;
    int ret = JIM_OK;

    stport = strrchr(hostport, ':');
    if (!stport) {
        /* No : so, the whole thing is the port */
        stport = hostport;
        sthost = Jim_StrDup("0.0.0.0"); // #MagicStr
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
    Jim_TFree<char>(sthost,"char"); // #FreeF 

    if (ret != JIM_OK) {
        Jim_SetResultFormatted(interp_, "Not a valid address: %s", hostport); // #ErrStr
    }

    return ret;
}

#ifdef HAVE_SYS_UN_H // #optionalCode #WinOff
static int JimParseDomainAddress(Jim_InterpPtr interp_, const char* path, struct sockaddr_un* sa) {
    sa->sun_family = PF_UNIX;
    snprintf(sa->sun_path, sizeof(sa->sun_path), "%s", path);

    return JIM_OK;
}
#endif /* HAVE_SYS_UN_H */

/**
 * Format that address in 'sa' as a string and store in variable 'varObjPtr'
 */
static int JimFormatIpAddress(Jim_InterpPtr interp_, Jim_ObjPtr  varObjPtr, const union sockaddr_any* sa) {
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

    return Jim_SetVariable(interp_, varObjPtr, Jim_NewStringObj(interp_, addrbuf, -1));
}

static Retval aio_cmd_recvfrom(Jim_InterpPtr  interp_, int argc, Jim_ObjConstArray  argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp_);
    char* buf;
    union sockaddr_any sa;
    long len_;
    socklen_t salen = sizeof(sa);
    int rlen;

    if (Jim_GetLong(interp_, argv[0], &len_) != JIM_OK) {
        return JIM_ERR;
    }

    buf = new_CharArray(len_ + 1); // #AllocF 

    rlen = prj_recvfrom(prj_fileno(af->fp), buf, len_, 0, &sa.sa, &salen); // #NonPortFuncFix
    if (rlen < 0) {
        Jim_TFree<char>(buf,"buf"); // #FreeF
        JimAioSetError(interp_, NULL);
        return JIM_ERR;
    }
    buf[rlen] = 0;
    Jim_SetResult(interp_, Jim_NewStringObjNoAlloc(interp_, buf, rlen));

    if (argc > 1) {
        return JimFormatIpAddress(interp_, argv[1], &sa);
    }

    return JIM_OK;
}


static Retval aio_cmd_sendto(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp_);
    int wlen;
    int len_;
    const char* wdata;
    union sockaddr_any sa;
    const char* addr = Jim_String(argv[1]);
    int salen;

    if (IPV6 && af->addr_family == PF_INET6) {
        if (JimParseIPv6Address(interp_, addr, &sa, &salen) != JIM_OK) {
            return JIM_ERR;
        }
    } else if (JimParseIpAddress(interp_, addr, &sa, &salen) != JIM_OK) {
        return JIM_ERR;
    }
    wdata = Jim_GetString(argv[0], &wlen);

    /* Note that we don't validate the socket tokenType_. Rely on sendto() failing if appropriate */
    len_ = prj_sendto(prj_fileno(af->fp), wdata, wlen, 0, &sa.sa, salen); // #NonPortFuncFix #SockFunc
    if (len_ < 0) {
        JimAioSetError(interp_, NULL);
        return JIM_ERR;
    }
    Jim_SetResultInt(interp_, len_);
    return JIM_OK;
}

static Retval aio_cmd_accept(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp_);
    int sock;
    union sockaddr_any sa;
    socklen_t addrlen = sizeof(sa);

    sock = prj_accept(af->fd, &sa.sa, &addrlen); // #NonPortFuncFix #SockFunc
    if (sock < 0) {
        JimAioSetError(interp_, NULL);
        return JIM_ERR;
    }

    if (argc > 0) {
        if (JimFormatIpAddress(interp_, argv[0], &sa) != JIM_OK) {
            return JIM_ERR;
        }
    }

    /* Create the file command_ */
    return JimMakeChannel(interp_, NULL, sock, Jim_NewStringObj(interp_, "accept", -1),
                          "aio.sockstream%ld", af->addr_family, "r+") ? JIM_OK : JIM_ERR;
}

static Retval aio_cmd_listen(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp_);
    long backlog;

    if (Jim_GetLong(interp_, argv[0], &backlog) != JIM_OK) {
        return JIM_ERR;
    }

    if (prj_listen(af->fd, backlog)) { // #NonPortFuncFix #SockFunc
        JimAioSetError(interp_, NULL);
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
    const char* name_;
    int level_;
    int opt;
    int tokenType_;   /* SOCKOPT_xxx */
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

static Retval aio_cmd_sockopt(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp_);
    int i;

    if (argc == 0) {
        Jim_ObjPtr  dictObjPtr = Jim_NewListObj(interp_, NULL, 0);
        for (i = 0; i < sizeof(g_sockopts) / sizeof(*g_sockopts); i++) {
            int value = 0;
            socklen_t len_ = sizeof(value);
            if (prj_getsockopt(af->fd, g_sockopts[i].level_, g_sockopts[i].opt, (void*) &value, &len_) == 0) { // #NonPortFuncFix #SockFunc
                if (g_sockopts[i].tokenType_ == SOCKOPT_BOOL) {
                    value = !!value;
                }
                Jim_ListAppendElement(interp_, dictObjPtr, Jim_NewStringObj(interp_, g_sockopts[i].name_, -1));
                Jim_ListAppendElement(interp_, dictObjPtr, Jim_NewIntObj(interp_, value));
            }
        }
        Jim_SetResult(interp_, dictObjPtr);
        return JIM_OK;
    }
    if (argc == 1) {
        return -1;
    }

    /* Set an option */
    for (i = 0; i < sizeof(g_sockopts) / sizeof(*g_sockopts); i++) {
        if (strcmp(Jim_String(argv[0]), g_sockopts[i].name_) == 0) {
            int on;
            if (g_sockopts[i].tokenType_ == SOCKOPT_BOOL) {
                if (Jim_GetBoolean(interp_, argv[1], &on) != JIM_OK) {
                    return JIM_ERR;
                }
            } else {
                long longval;
                if (Jim_GetLong(interp_, argv[1], &longval) != JIM_OK) {
                    return JIM_ERR;
                }
                on = longval;
            }
            if (prj_setsockopt(af->fd, g_sockopts[i].level_, g_sockopts[i].opt, (void*) &on, sizeof(on)) < 0) { // #NonPortFuncFix #SockFunc
                Jim_SetResultFormatted(interp_, "Failed to set %#s: %s", argv[0], strerror(errno)); // #ErrStr
                return JIM_ERR;
            }
            return JIM_OK;
        }
    }
    /* Not found */
    Jim_SetResultFormatted(interp_, "Unknown sockopt %#s", argv[0]); // #ErrStr
    return JIM_ERR;
}

#endif /* if defined(HAVE_SOCKETS) && !defined(JIM_BOOTSTRAP) */

END_JIM_NAMESPACE

#endif // #if jim_ext_aio
