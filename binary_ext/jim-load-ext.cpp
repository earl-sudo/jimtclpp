
#include <jimautoconf.h>

#include <jim-api.h>
#include <prj_compat.h>


/* -----------------------------------------------------------------------------
 * Dynamic libraries support (WIN32 not supported)
 * ---------------------------------------------------------------------------*/

#ifdef HAVE_DLFCN_H // #optionalCode #WinOff
#include <dlfcn.h> // #NonPortHeader
#endif

#ifndef RTLD_NOW // #optionalCode
    #define RTLD_NOW 0
#endif
#ifndef RTLD_LOCAL
    #define RTLD_LOCAL 0
#endif

BEGIN_JIM_NAMESPACE

static void JimFreeLoadHandles(Jim_InterpPtr interp, void *data);

/**
 * Note that Jim_LoadLibrary() requires a path to an existing file.
 *
 * If it is necessary to search JIM_LIBPATH, use Jim_PackageRequire() instead.
 */
JIM_EXPORT Retval Jim_LoadLibrary(Jim_InterpPtr interp, const char *pathName)
{
    if (prj_funcDef(prj_dlopen)) { // #Unsupported #NonPortFuncFix
        return JIM_ERR;
    }

    void *handle = prj_dlopen(pathName, RTLD_NOW | RTLD_LOCAL); // #PosixSym #NonPortFuncFix
    if (handle == NULL) {
        Jim_SetResultFormatted(interp, "error loading extension \"%s\": %s", pathName,
            prj_dlerror()); // #NonPortFuncFix
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
        typedef int jim_module_init_func_type(Jim_InterpPtr );
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

        if ((onload = (jim_module_init_func_type *)prj_dlsym(handle, initsym)) == NULL) { // #NonPortFuncFix
            Jim_SetResultFormatted(interp,
                "No %s symbol found in extension %s", initsym, pathName);
        }
        else if (onload(interp) != JIM_ERR) {
            /* Add this handle to the stack of handles to be freed */
            Jim_StackPtr loadHandles = (Jim_StackPtr )Jim_GetAssocData(interp, "load::handles");
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
        prj_dlclose(handle); // #NonPortFuncFix
    }
    return JIM_ERR;
}

static void JimFreeOneLoadHandle(void *handle)
{
    prj_dlclose(handle); // #NonPortFuncFix
}

static void JimFreeLoadHandles(Jim_InterpPtr interp, void *data)
{
    Jim_StackPtr handles = (Jim_StackPtr )data;

    if (handles) {
        Jim_FreeStackElements(handles, JimFreeOneLoadHandle);
        Jim_FreeStack(handles);
        Jim_TFree<Jim_Stack>(handles); // #FreeF 
    }
}


/* [load] */
static Retval Jim_LoadCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "libraryFile");
        return JIM_ERR;
    }
    return Jim_LoadLibrary(interp, Jim_String(argv[1]));
}

Retval Jim_loadInit(Jim_InterpPtr interp) // #JimCmdInit
{
    Jim_CreateCommand(interp, "load", Jim_LoadCoreCommand, NULL, NULL);
    return JIM_OK;
}

END_JIM_NAMESPACE
