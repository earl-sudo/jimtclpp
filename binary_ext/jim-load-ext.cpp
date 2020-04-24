#include <string.h>
#include <stdio.h>

#include "jimautoconf.h"

#include "jim-api.h"
#include <prj_compat.h>


/* -----------------------------------------------------------------------------
 * Dynamic libraries support (WIN32 not supported)
 * ---------------------------------------------------------------------------*/

#ifdef HAVE_DLFCN_H // #optionalCode #WinOff
#include <dlfcn.h>
#endif

#ifndef RTLD_NOW // #optionalCode
    #define RTLD_NOW 0
#endif
#ifndef RTLD_LOCAL
    #define RTLD_LOCAL 0
#endif

BEGIN_JIM_NAMESPACE

static void JimFreeLoadHandles(Jim_Interp *interp, void *data);

/**
 * Note that Jim_LoadLibrary() requires a path to an existing file.
 *
 * If it is necessary to search JIM_LIBPATH, use Jim_PackageRequire() instead.
 */
int Jim_LoadLibrary(Jim_Interp *interp, const char *pathName)
{
    if (prj_funcDef(prj_dlopen)) { // #Unsupported
        return JIM_ERR;
    }

    void *handle = prj_dlopen(pathName, RTLD_NOW | RTLD_LOCAL); // #PosixSym
    if (handle == NULL) {
        Jim_SetResultFormatted(interp, "error loading extension \"%s\": %s", pathName,
            prj_dlerror());
    }
    else {
        /* We use a unique init symbol depending on the extension name.
         * This is done for compatibility between static and dynamic extensions.
         * For extension readline.so, the init symbol is "Jim_readlineInit"
         */
        const char *pt;
        const char *pkgname;
        int pkgnamelen;
        char initsym[40];
        typedef int jim_module_init_func_type(Jim_Interp *);
        jim_module_init_func_type *onload;

        pt = strrchr(pathName, '/');
        if (pt) {
            pkgname = pt + 1;
        }
        else {
            pkgname = pathName;
        }
        pt = strchr(pkgname, '.');
        if (pt) {
            pkgnamelen = (int)(pt - pkgname);
        }
        else {
            pkgnamelen = (int)strlen(pkgname);
        }
        snprintf(initsym, sizeof(initsym), "Jim_%.*sInit", pkgnamelen, pkgname);

        if ((onload = (jim_module_init_func_type *)prj_dlsym(handle, initsym)) == NULL) {
            Jim_SetResultFormatted(interp,
                "No %s symbol found in extension %s", initsym, pathName);
        }
        else if (onload(interp) != JIM_ERR) {
            /* Add this handle to the stack of handles to be freed */
            Jim_Stack *loadHandles = (Jim_Stack*)Jim_GetAssocData(interp, "load::handles");
            if (loadHandles == NULL) {
                loadHandles = Jim_AllocStack();
                Jim_InitStack(loadHandles);
                Jim_SetAssocData(interp, "load::handles", JimFreeLoadHandles, loadHandles);
            }
            Jim_StackPush(loadHandles, handle);

            Jim_SetEmptyResult(interp);

            return JIM_OK;
        }
    }
    if (handle) {
        prj_dlclose(handle);
    }
    return JIM_ERR;
}

static void JimFreeOneLoadHandle(void *handle)
{
    prj_dlclose(handle);
}

static void JimFreeLoadHandles(Jim_Interp *interp, void *data)
{
    Jim_Stack *handles = (Jim_Stack*)data;

    if (handles) {
        Jim_FreeStackElements(handles, JimFreeOneLoadHandle);
        Jim_FreeStack(handles);
        Jim_Free(handles); // #Free
    }
}


/* [load] */
static int Jim_LoadCoreCommand(Jim_Interp *interp, int argc, Jim_Obj *const *argv) // #JimCmd
{
    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "libraryFile");
        return JIM_ERR;
    }
    return Jim_LoadLibrary(interp, Jim_String(argv[1]));
}

int Jim_loadInit(Jim_Interp *interp) // #JimCmdInit
{
    Jim_CreateCommand(interp, "load", Jim_LoadCoreCommand, NULL, NULL);
    return JIM_OK;
}

END_JIM_NAMESPACE
