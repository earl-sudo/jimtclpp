//#include <string.h>

#ifdef _WIN32 // #TODO add a HAVE_IO_H
#  include <io.h>
#endif

#include <jim-config.h>
#include <jim-api.h>
#include <prj_compat.h>

#if jim_ext_package

#ifdef HAVE_UNISTD_H // #optionalCode #WinOff
#  include <unistd.h> // #NonPortHeader
#else
#  define R_OK 4
#endif


BEGIN_JIM_NAMESPACE

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-package-version.h>

/* All packages have a fixed, dummy version */
static const char *package_version_1 = version; 

/* -----------------------------------------------------------------------------
 * Packages handling
 * ---------------------------------------------------------------------------*/


JIM_EXPORT Retval Jim_PackageProvide(Jim_InterpPtr interp, const char *name, const char *ver, int flags)
{
    /* If the package was already provided returns an errorText_. */
    Jim_HashEntryPtr he = Jim_FindHashEntry(Jim_PackagesHT(interp), name);

    /* An empty result means the automatic entry. This can be replaced */
    if (he && *(const char *)Jim_KeyAsStr(he)) {
        if (flags & JIM_ERRMSG) {
            Jim_SetResultFormatted(interp, "package \"%s\" was already provided", name);
        }
        return JRET(JIM_ERR);
    }
    IGNOREEXTRADATA Jim_ReplaceHashEntry(Jim_PackagesHT(interp), name, (char *)ver);
    return JRET(JIM_OK);
}

#ifdef jim_ext_load // #optionalCode
int g_jim_ext_load_VAL = 1;
#else
int g_jim_ext_load_VAL = 0;
#endif

/**
 * Searches along a of paths for the given package.
 *
 * Returns the allocated path to the package file if found,
 * or nullptr if not found.
 */
static char *JimFindPackage(Jim_InterpPtr interp, Jim_ObjPtr prefixListObj, const char *pkgName)
{
    int i;
    char* buf = new_CharArray(JIM_PATH_LEN); // #AllocF
    int prefixc = Jim_ListLength(interp, prefixListObj);

    for (i = 0; i < prefixc; i++) {
        Jim_ObjPtr prefixObjPtr = Jim_ListGetIndex(interp, prefixListObj, i);
        const char *prefix = Jim_String(prefixObjPtr);

        /* Loadable modules are tried first */
        if (g_jim_ext_load_VAL) {
            snprintf(buf, JIM_PATH_LEN, "%s/%s.so", prefix, pkgName);
            if (prj_access(buf, R_OK) == 0) { // #NonPortFuncFix
                return buf;
            }
        }
        if (strcmp(prefix, ".") == 0) {
            snprintf(buf, JIM_PATH_LEN, "%s.tcl", pkgName);
        }
        else {
            snprintf(buf, JIM_PATH_LEN, "%s/%s.tcl", prefix, pkgName);
        }

        if (prj_access(buf, R_OK) == 0) { // #NonPortFuncFix
            return buf;
        }
    }
    free_CharArray(buf); // #FreeF 
    return nullptr;
}

/* Search for a suitable package under every dir specified by JIM_LIBPATH,
 * and load it if possible. If a suitable package was loaded with success
 * JRET(JIM_OK) is returned, otherwise JRET(JIM_ERR) is returned. */
static Retval JimLoadPackage(Jim_InterpPtr interp, const char *name, int flags MAYBE_USED)
{
    int retCode = JRET(JIM_ERR);
    Jim_ObjPtr libPathObjPtr = Jim_GetGlobalVariableStr(interp, JIM_LIBPATH, JIM_NONE);
    if (libPathObjPtr) {
        char *path;

        /* Scan every directory for the the first match */
        path = JimFindPackage(interp, libPathObjPtr, name);
        if (path) {
            const char *p;

            /* Note: Even if the file fails to load, we consider the package loaded.
             *       This prevents issues with recursion.
             *       Use a dummy version of "" to signify this case.
             */
            retCode = Jim_PackageProvide(interp, name, "", 0);
            if (retCode != JIM_OK) return retCode;


            /* Try to load/source it */
            p = strrchr(path, '.');

            if (p && strcmp(p, ".tcl") == 0) {
                Jim_IncrRefCount(libPathObjPtr);
                retCode = Jim_EvalFileGlobal(interp, path);
                Jim_DecrRefCount(interp, libPathObjPtr);
            }
            else {
                if (g_jim_ext_load_VAL) {
                    retCode = Jim_LoadLibrary(interp, path);
                }
            }
            if (retCode != JRET(JIM_OK)) {
                /* Upon failure, remove the dummy entry */
                IGNOREJIMRET Jim_DeleteHashEntry(Jim_PackagesHT(interp), name);
            }
            free_CharArray(path); // #FreeF 
        }

        return retCode;
    }
    return JRET(JIM_ERR);
}

JIM_EXPORT Retval Jim_PackageRequire(Jim_InterpPtr interp, const char *name, int flags)
{
    Jim_HashEntryPtr he;

    /* Start with an empty errorText_ string */
    Jim_SetEmptyResult(interp);

    he = Jim_FindHashEntry(Jim_PackagesHT(interp), name);
    if (he == nullptr) {
        /* Try to load the package. */
        Retval retcode = JimLoadPackage(interp, name, flags);
        if (retcode != JRET(JIM_OK)) {
            if (flags & JIM_ERRMSG) {
                int len = Jim_Length(Jim_GetResult(interp));
                Jim_SetResultFormatted(interp, "%#s%sCan't load package %s",
                    Jim_GetResult(interp), len ? "\n" : "", name);
            }
            return retcode;
        }

        /* In case the package did not 'package provide' */
        IGNOREJIMRET Jim_PackageProvide(interp, name, package_version_1, 0);

        /* Now it must exist */
        he = Jim_FindHashEntry(Jim_PackagesHT(interp), (const void*)name);
    }

    Jim_SetResultString(interp, Jim_KeyAsStr(he), -1);
    return JRET(JIM_OK);
}

/*
 *----------------------------------------------------------------------
 *
 * package provide name_ ?version?
 *
 *      This procedure is invoked to declare that
 *      a particular package is now present in an interpreter.
 *      The package must not already be provided in the interpreter.
 *
 * Results:
 *      Returns JRET(JIM_OK) and sets results as "1.0" (the given version is ignored)
 *
 *----------------------------------------------------------------------
 */
static Retval package_cmd_provide(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv) // #JimCmd
{
    return Jim_PackageProvide(interp, Jim_String(argv[0]), package_version_1, JIM_ERRMSG);
}

/*
 *----------------------------------------------------------------------
 *
 * package require name_ ?version?
 *
 *      This procedure is load a given package.
 *      Note that the version is ignored.
 *
 * Results:
 *      Returns JRET(JIM_OK) and sets the package version.
 *
 *----------------------------------------------------------------------
 */
static Retval package_cmd_require(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv) // #JimCmd
{
    /* package require failing is important enough to add to the stack_ */
    Jim_IncrStackTrace(interp);

    return Jim_PackageRequire(interp, Jim_String(argv[0]), JIM_ERRMSG);
}

/*
 *----------------------------------------------------------------------
 *
 * package list
 *
 *      Returns a list of known packages
 *
 * Results:
 *      Returns JRET(JIM_OK) and sets a list of known packages.
 *
 *----------------------------------------------------------------------
 */
static Retval package_cmd_list(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    Jim_HashTableIterator *htiter;
    Jim_HashEntryPtr he;
    Jim_ObjPtr listObjPtr = Jim_NewListObj(interp, nullptr, 0);

    htiter = Jim_GetHashTableIterator(Jim_PackagesHT(interp));
    while ((he = Jim_NextHashEntry(htiter)) != nullptr) {
        Jim_ListAppendElement(interp, listObjPtr, Jim_NewStringObj(interp, Jim_KeyAsStr(he), -1));
    }
    Jim_FreeHashTableIterator(htiter);

    Jim_SetResult(interp, listObjPtr);

    return JRET(JIM_OK);
}

static const jim_subcmd_type g_package_command_table[] = { // #JimSubCmdDef
    {
        "provide",
        "name ?version?",
        package_cmd_provide,
        1,
        2,
        /* Description: Indicates that the current script provides the given package */
    },
    {
        "require",
        "name ?version?",
        package_cmd_require,
        1,
        2,
        /* Description: Loads the given package by looking in standard places */
    },
    {
        "list",
        nullptr,
        package_cmd_list,
        0,
        0,
        /* Description: Lists all known packages */
    },
    {
        
    }
};

JIM_EXPORT Retval Jim_packageInit(Jim_InterpPtr interp) // #JimCmdInit
{
    Retval ret = JIM_ERR;

    ret = Jim_CreateCommand(interp, "package", Jim_SubCmdProc, (void *)g_package_command_table, nullptr);
    if (ret != JIM_OK) return ret;

    return JRET(JIM_OK);
}

END_JIM_NAMESPACE

#endif // #if jim_ext_package
