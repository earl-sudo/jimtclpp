#pragma once

#include <jim-api.h>

#include <tuple>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <string_view>

BEGIN_JIM_NAMESPACE

using namespace std;


template<typename tocheck>
bool negativeIsError(tocheck v) { return v < 0; }

struct ArgumentCheck {
    int16_t minNumArgs_ = 0;
    int16_t maxNumArgs_ = 0;
};

struct JimObjError {
    Retval code_;
    JimObjError(Retval v) : code_(v) {}
};

enum JIMOBJ_ERRORS {
    JIM_OBJ_ERROR_NONE,
    JIMOBJ_ERROR_UNKNOWN = 100,
    JIMOBJ_ERROR_CONV_LONG, JIMOBJ_ERROR_CONV_INT64, JIMBOBJ_ERRROR_CONV_BOOL, JIMOBJ_ERROR_CONV_DOUBLE,
    JIMOBJ_ERROR_BADINDEX, JIMOBJ_ERROR_JUSTARETURN,
    JIMOBJ_ERROR_INVALIDTYPE, 
    JIMOBJ_ERROR_UNKNOWN_VAR_NAME
};


// Combines Jim_InterpPtr and Jim_ObjPtr into a convent package
struct JimObj {
    enum OBJ_TYPE { OBJ_NORMAL, OBJ_LIST , OBJ_STRING, OBJ_REFERENCE, OBJ_DICT, OBJ_INT, OBJ_DOUBLE };
    Jim_InterpPtr  interp_;
    Jim_ObjPtr  obj_;

    JimObj(Jim_InterpPtr  interp, Jim_ObjPtr  obj) : interp_(interp), obj_(obj) {}
    JimObj(const JimObj& lhs) : interp_(lhs.interp_), obj_(lhs.obj_) {}
    JimObj(Jim_InterpPtr interp, OBJ_TYPE type = OBJ_NORMAL) : interp_(interp) {
        switch (type) {
            case OBJ_NORMAL:
                obj_ = Jim_NewObj(interp_);
                break;
            case OBJ_LIST:
                obj_ = Jim_NewListObj(interp, NULL, 0);
                break;
            case OBJ_STRING:
                obj_ = Jim_NewStringObj(interp_, NULL, 0);
                break;
            case OBJ_REFERENCE:
                break;
            case OBJ_DICT:
                obj_ = Jim_NewDictObj(interp, NULL, 0);
                break;
            case OBJ_INT:
                obj_ = Jim_NewIntObj(interp_, 0);
                break;
            case OBJ_DOUBLE:
                obj_ = Jim_NewDoubleObj(interp_, 0);
                break;
            default:       
                throw JimObjError(JIMOBJ_ERROR_INVALIDTYPE);
                break;
        };
    }
    const JimObj& operator=(const JimObj& lhs) {
        interp_ = lhs.interp_;
        obj_ = lhs.obj_;
        return *this;
    }

    // Simply exports a value or throw exception.
    operator long() const {
        long ret = 0;
        Retval retcode = Jim_GetLong(interp_, obj_, &ret);
        if (retcode != JIM_OK) throw JimObjError(JIMOBJ_ERROR_CONV_LONG);
        return ret;
    }
    operator int64_t() const {
        int64_t ret = 0;
        Retval retcode = Jim_GetWide(interp_, obj_, &ret);
        if (retcode != JIM_OK) throw JimObjError(JIMOBJ_ERROR_CONV_INT64);
        return ret;
    }
    operator bool() const {
        int ret = 0;
        Retval retcode = Jim_GetBoolean(interp_, obj_, &ret);
        if (retcode != JIM_OK) throw JimObjError(JIMBOBJ_ERRROR_CONV_BOOL);
        return (bool) ret;
    }
    operator const char* () const {
        const char* ret = 0;
        return Jim_String(obj_);
    }
    operator double() const {
        double ret = 0;
        Retval retcode = Jim_GetDouble(interp_, obj_, &ret);
        if (retcode != JIM_OK) throw JimObjError(JIMOBJ_ERROR_CONV_DOUBLE);
        return ret;
    }

    bool isList(int index) const { return Jim_IsList(obj_); }
    int list_length() const {
        return Jim_ListLength(interp_, obj_);
    }

    // Contently list concatenation. 
    JimObj& append(const char* val, int len = -1) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewStringObj(interp_, val, len));
        return *this;
    }
    JimObj& append(long val) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewIntObj(interp_, val));
        return *this;
    }
    JimObj& append(int64_t val) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewIntObj(interp_, val));
        return *this;
    }
    JimObj& append(bool val) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewIntObj(interp_, val));
        return *this;
    }
    JimObj& append(double val) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewDoubleObj(interp_, val));
        return *this;
    }
    bool isDict() const { return Jim_IsDict(obj_); }

    bool operator==(string_view rhs) {
        return rhs == Jim_String(obj_);
    }
    bool operator!=(string_view rhs) {
        return rhs != Jim_String(obj_);
    }
    int referenceCount(int val = 0) {
        if (val > 0) while (val > 0) { Jim_IncrRefCount(obj_); val--; }
        if (val < 0) while (val < 0) { Jim_DecrRefCount(interp_, obj_); val++; }
        return Jim_RefCount(obj_);
    }
    void invalidateStringRep() { Jim_InvalidateStringRep(obj_);  }
    int length(void) {
        return Jim_Length(obj_);
    }
    int dict_size() { if (!isDict()) return -1; return Jim_DictSize(interp_, obj_); }
};

// Wraps the standard args of Jim Jim_InterpPtr, argc, argv
struct JimArgs {
private:
    Jim_InterpPtr        interp_;
    int                  argc_;
    Jim_ObjConstArray    argv_;
public:
    bool setResults_ = false;

    JimArgs(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) : interp_(interp), argc_(argc), argv_(argv) {}

    // Access to arguments
    int numArgs() const { return argc_; }
    JimObj arg(int index) {
        if (index >= numArgs()) throw JimObjError(JIMOBJ_ERROR_BADINDEX);
        return JimObj(interp_, argv_[index]);
    }

    // Convert C native to a JimObj
    JimObj val(const char* val, int len = -1) {
        return  JimObj(interp_, Jim_NewStringObj(interp_, val, len));
    }
    JimObj val(long val) {
        return  JimObj(interp_, Jim_NewIntObj(interp_, val));
    }
    JimObj val(int64_t val) {
        return  JimObj(interp_, Jim_NewIntObj(interp_, val));
    }
    JimObj listVal(void) {
        return  JimObj(interp_, Jim_NewListObj(interp_, NULL, 0));
    }
    JimObj dictVal(void) {
        return  JimObj(interp_, Jim_NewDictObj(interp_, NULL, 0));
    }

#if 0
    JimObj getNamedNoErr(Jim_ObjPtr  varName) { // Create if doesn't exists.
        Jim_ObjPtr  objPtr = Jim_GetVariable(interp_, varName, JIM_NONE);
        if (objPtr) {

        }
    }
#endif

    // Set return value 
    void return_(const char* val, int len = -1) { setResults_ = true;  Jim_SetResultString(interp_, val, len); throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_(long val) { setResults_ = true;  Jim_SetResultInt(interp_, val); throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_(int64_t val) { setResults_ = true;  Jim_SetResultInt(interp_, val); throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_(bool val) { setResults_ = true;  Jim_SetResultBool(interp_, (long_long) val);  throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_() { setResults_ = true;  Jim_SetEmptyResult(interp_); throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_(JimObj& obj) { setResults_ = true; Jim_SetResult(interp_, obj.obj_); }
};

struct JimInterp {
    Jim_InterpPtr  interp_;
    JimInterp(Jim_Interp* interp) : interp_(interp) {}

    JimObj get(JimObj& name, JIM_INTERP_FLAG_FLAGS flags = JIM_NONE) {
        auto v = Jim_GetVariable(interp_, name.obj_, flags);
        if (v == NULL) throw JimObjError(JIMOBJ_ERROR_UNKNOWN);
        JimObj  ret(interp_, v);
        return ret;
    }
    JimObj get(Jim_ObjPtr name, JIM_INTERP_FLAG_FLAGS flags = JIM_NONE) {
        auto v = Jim_GetVariable(interp_, name, flags);
        if (v == NULL) throw JimObjError(JIMOBJ_ERROR_UNKNOWN);
        JimObj  ret(interp_, v);
        return ret;
    }
    void set(JimObj& name, JimObj& value) {
        if (Jim_SetVariable(interp_, name.obj_, value.obj_)) {
            throw  JimObjError(JIMOBJ_ERROR_UNKNOWN);
        }
    }
    JimObj dict_merge(JimObj& name, JimObj& listValues) {
        Jim_ObjPtr objv[2] = { name.obj_, listValues.obj_ };
        auto v = Jim_DictMerge(interp_, 2, objv);
        if (v == NULL) throw JimObjError(JIMOBJ_ERROR_UNKNOWN);
        return JimObj(interp_, v);
    }
    void free(JimObj& obj) { Jim_FreeObj(interp_, obj.obj_); }
    void free(Jim_Obj* obj) { Jim_FreeObj(interp_, obj); }
};

// Tries to take care of everything you need to do to wrap a function.
struct JimCmd {
    string cmd_;
    string description_;

    // Wrap all the checking of arguments in one object to keep JimCmd simpler.
    int16_t minNumArgs_ = -1;
    int16_t maxNumArgs_ = -1;


    JimCmd(void) : cmd_("cmdNone"), description_("arg1") {}

    Retval jimcmdCaller(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) {
        Retval ret = JIM_OK;
        try {
            JimArgs  args(interp, argc, argv);
            jimcmd(args);
            //if (!args.setResults_) args.return_();
        } catch (JimObjError& joe) {
            if (joe.code_ == JIMOBJ_ERROR_JUSTARETURN) {
            } else {
                ret = JIM_ERR;
            }
        }
        return ret;
    }
    virtual void jimcmd(JimArgs& args) { // Actual specialization.
        JimObj   path(args.arg(0));

        //args.return_(fileSize1((const char*) path));
    }
    JimCmd(const string& cmd, const string& description) : cmd_(cmd), description_(description) {
    }
};



END_JIM_NAMESPACE
