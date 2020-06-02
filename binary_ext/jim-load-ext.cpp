
#include <jimautoconf.h>

#include <jim-api.h>
#include <prj_compat.h>

#if jim_ext_load

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
        return JRET(JIM_ERR);
    }

    void *handle = prj_dlopen(pathName, RTLD_NOW | RTLD_LOCAL); // #PosixSym #NonPortFuncFix
    if (handle == nullptr) {
        Jim_SetResultFormatted(interp, "error loading extension \"%s\": %s", pathName,
            prj_dlerror()); // #NonPortFuncFix
    }
    else {
        /* We use a unique_ init symbol depending on the extension name_.
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

        if ((onload = (jim_module_init_func_type *)prj_dlsym(handle, initsym)) == nullptr) { // #NonPortFuncFix
            Jim_SetResultFormatted(interp,
                "No %s symbol found in extension %s", initsym, pathName);
        }
        else if (onload(interp) != JRET(JIM_ERR)) {
            /* Add this handle to the stack_ of handles to be freed */
            Jim_StackPtr loadHandles = (Jim_StackPtr )Jim_GetAssocData(interp, "load::handles");
            if (loadHandles == nullptr) {
                loadHandles = Jim_AllocStack();
                Jim_InitStack(loadHandles);
                IGNOREJIMRET Jim_SetAssocData(interp, "load::handles", JimFreeLoadHandles, loadHandles);
            }
            Jim_StackPush(loadHandles, handle);

            Jim_SetEmptyResult(interp);

            return JRET(JIM_OK);
        }
    }
    if (handle) {
        prj_dlclose(handle); // #NonPortFuncFix
    }
    return JRET(JIM_ERR);
}

static void JimFreeOneLoadHandle(void *handle)
{
    prj_dlclose(handle); // #NonPortFuncFix
}

static void JimFreeLoadHandles(Jim_InterpPtr interp MAYBE_USED, void *data)
{
    Jim_StackPtr handles = (Jim_StackPtr )data;

    if (handles) {
        Jim_FreeStackElements(handles, JimFreeOneLoadHandle);
        Jim_FreeStack(handles);
        free_Jim_Stack(handles); // #FreeF 
    }
}


/* [load] */
static Retval Jim_LoadCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "libraryFile");
        return JRET(JIM_ERR);
    }
    return Jim_LoadLibrary(interp, Jim_String(argv[1]));
}

JIM_EXPORT Retval Jim_loadInit(Jim_InterpPtr interp) // #JimCmdInit
{
    Retval ret = JIM_ERR;
    ret = Jim_CreateCommand(interp, "load", Jim_LoadCoreCommand, nullptr, nullptr);
    if (ret != JIM_OK) return ret;
    return JRET(JIM_OK);
}

END_JIM_NAMESPACE

#endif // #if jim_ext_load
