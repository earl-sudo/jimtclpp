#include "jimautoconf.h"
#include "prj_compat.h"
#include <jim-api.h>

// =============================================================================================

#include <jim-aio-ext.h>

#if defined(JIM_SSL) // #optionalCode #WinOff
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#if defined(JIM_SSL) && !defined(JIM_BOOTSTRAP) // #optionalCode  #WinOff

BEGIN_JIM_NAMESPACE

static SSL_CTX* JimAioSslCtx(Jim_InterpPtr interp);

static int ssl_writer(struct AioFile* af, const char* buf, int len) {
    return SSL_write((SSL*) af->ssl, buf, len);
}

static int ssl_reader(struct AioFile* af, char* buf, int len) {
    return SSL_read((SSL*) af->ssl, buf, len);
}

static const char* ssl_getline(struct AioFile* af, char* buf, int len) {
    size_t i;
    for (i = 0; i < len + 1; i++) {
        if (SSL_read((SSL*) af->ssl, &buf[i], 1) != 1) {
            if (i == 0) {
                return NULL;
            }
            break;
        }
        if (buf[i] == '\n') {
            break;
        }
    }
    buf[i] = '\0';
    return buf;
}

static int ssl_error(const struct AioFile* af) {
    if (ERR_peek_error() == 0) {
        return JIM_OK;
    }

    return JIM_ERR;
}

static const char* ssl_strerror(struct AioFile* af) {
    int err = ERR_get_error();

    if (err) {
        return ERR_error_string(err, NULL);
    } else {
        return stdio_strerror(af);
    }
}

static int ssl_verify(struct AioFile* af) {
    X509* cert;

    cert = SSL_get_peer_certificate((const SSL*) af->ssl);
    if (!cert) {
        return JIM_ERR;
    }
    X509_free(cert);

    if (SSL_get_verify_result((const SSL*) af->ssl) == X509_V_OK) {
        return JIM_OK;
    }

    return JIM_ERR;
}

static const JimAioFopsType g_ssl_fops = {
    ssl_writer,
    ssl_reader,
    ssl_getline,
    ssl_error,
    ssl_strerror,
    ssl_verify
};

static Retval aio_cmd_ssl(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd #SSLCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp);
    SSL* ssl;
    SSL_CTX* ssl_ctx;
    int server = 0;

    if (argc == 5) {
        if (!Jim_CompareStringImmediate(interp, argv[2], "-server")) {
            return JIM_ERR;
        }
        server = 1;
    } else if (argc != 2) {
        Jim_WrongNumArgs(interp, 2, argv, "?-server cert priv?");
        return JIM_ERR;
    }

    if (af->ssl) {
        Jim_SetResultFormatted(interp, "%#s: stream is already ssl", argv[0]);
        return JIM_ERR;
    }

    ssl_ctx = JimAioSslCtx(interp);
    if (ssl_ctx == NULL) {
        return JIM_ERR;
    }

    ssl = SSL_new(ssl_ctx);
    if (ssl == NULL) {
        goto out;
    }

    SSL_set_cipher_list(ssl, "ALL");

    if (SSL_set_fd(ssl, prj_fileno(af->fp)) == 0) {
        goto out;
    }

    if (server) {
        if (SSL_use_certificate_file(ssl, Jim_String(argv[3]), SSL_FILETYPE_PEM) != 1) {
            goto out;
        }

        if (SSL_use_PrivateKey_file(ssl, Jim_String(argv[4]), SSL_FILETYPE_PEM) != 1) {
            goto out;
        }

        if (SSL_accept(ssl) != 1) {
            goto out;
        }
    } else {
        if (SSL_connect(ssl) != 1) {
            goto out;
        }
    }

    af->ssl = ssl;
    af->fops = &g_ssl_fops;

    /* Set the command name as the result */
    Jim_SetResult(interp, argv[0]);

    return JIM_OK;

out:
    if (ssl) {
        SSL_free(ssl);
    }
    Jim_SetResultString(interp, ERR_error_string(ERR_get_error(), NULL), -1);
    return JIM_ERR;
}

static Retval aio_cmd_verify(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd #SSLCmd
{
    AioFile* af = (AioFile*) Jim_CmdPrivData(interp);
    Retval ret;

    if (!af->fops->verify) {
        return JIM_OK;
    }

    ret = af->fops->verify(af);
    if (ret != JIM_OK) {
        if (JimCheckStreamError(interp, af) == JIM_OK) {
            Jim_SetResultString(interp, "failed to verify the connection authenticity", -1);
        }
    }
    return ret;
}

END_JIM_NAMESPACE

#endif /* if defined(JIM_SSL) && !defined(JIM_BOOTSTRAP) */