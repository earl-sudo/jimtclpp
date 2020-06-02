#pragma once

#include <jim-api.h>
#include <prj_compat.h>

#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <array>

BEGIN_JIM_NAMESPACE

using namespace std;

struct FreeOnExit {
    Jim_InterpPtr   interp_;
    Jim_ObjPtr      obj_;
    FreeOnExit(Jim_InterpPtr i, Jim_ObjPtr o) : interp_(i), obj_(o) { }
    ~FreeOnExit() { Jim_FreeObj(interp_, obj_); }
};

struct CppApi {
    Jim_InterpPtr interp_;
    int argc_ = 0;
    Jim_ObjConstArray argv_ = nullptr;
    bool resultSet_ = false;
    bool inError_ = false;
    Retval ret = JIM_ERR;

    CppApi(Jim_InterpPtr interpD) : interp_(interpD) {}
    CppApi(Jim_InterpPtr interpD, int argcD, Jim_ObjConstArray argvD)
        : interp_(interpD), argc_(argcD), argv_(argvD) {}

    CHKRET inline Jim_ObjPtr  NewListObj();
    CHKRET inline Jim_ObjPtr  NewListObj(const vector<string>& elements);
    CHKRET inline Jim_ObjPtr  NewDictObj(int len, Jim_ObjConstArray elements);
    CHKRET inline Jim_ObjPtr  NewDictObjMerge(int len, Jim_ObjConstArray elements);
    CHKRET inline Jim_ObjPtr  NewObj(jim_wide wideValue);
    CHKRET inline Jim_ObjPtr  NewObj(string_view str);
    CHKRET inline Jim_ObjPtr  NewStrObj(char* input, int len = -1);
    inline CppApi& append(Jim_ObjPtr list, Jim_ObjPtr obj);
    inline CppApi& append(Jim_ObjPtr list, string_view str);
    inline CppApi& append(Jim_ObjPtr list, jim_wide val);
    CHKRET inline Jim_ObjPtr getVar(Jim_ObjPtr name, int flags = JIM_NONE);
    CHKRET inline Jim_ObjPtr getVar(string_view name, int flags = JIM_NONE);
    CHKRET inline Retval setVar(Jim_ObjPtr name, Jim_ObjPtr value);
    CHKRET inline Retval setVar(string_view name, Jim_ObjPtr value);
    CHKRET inline Retval setVar(string_view name, string_view value);
    inline void free(Jim_ObjPtr o);
    inline void setResult(Jim_ObjPtr o);
    inline void setResult(string_view str);
    inline void setResult(const char* str);
    inline void setResult(jim_wide val);
    inline void setResult(bool val);
    inline void setResult(const vector<string>& vals);
    CHKRET inline bool isEqual(Jim_ObjPtr o, string_view str);
    CHKRET inline bool isEqual(Jim_ObjPtr o1, Jim_ObjPtr o2, bool nocase = false);
    inline void invalidate(Jim_ObjPtr obj);
    CHKRET inline Retval evalPrefix(const char* prefix, int oc, Jim_ObjConstArray cv);
    CHKRET inline Retval packageProvided(string_view name, string_view ver, int flags = JIM_ERRMSG);
    CHKRET inline Retval createCmd(string_view name, Jim_CmdProc cmdProc,
                                   void* privData = nullptr, Jim_DelCmdProc* delProc = nullptr);
    CHKRET inline Retval createSubCmd(string_view parentName, jim_subcmd_type* subcmdArray);
    CHKRET inline tuple<Retval, jim_wide> getInt(Jim_ObjPtr o);
    CHKRET inline Retval getEnum(Jim_ObjPtr objPtr,
                                 const char* const* tablePtr, int* indexPtr,
                                 const char* name, int flags);
    inline void wrongNumArgs(int argc, Jim_ObjConstArray argv, string_view msg);
    CHKRET inline vector<string> getStrList(const Jim_ObjPtr objPtr);
};

inline const char* getStr(Jim_ObjPtr o) { return Jim_String(o); }
inline const char* getStr(Jim_ObjPtr o, int& len) { return Jim_GetString(o, &len); }

inline Jim_ObjPtr  CppApi::NewListObj() { return Jim_NewListObj(interp_, nullptr, 0); }
inline Jim_ObjPtr  CppApi::NewListObj(const vector<string>& elements) {
    auto listObj = NewListObj();
    for (auto& e : elements) {
        append(listObj, e.c_str());
    }
    return listObj;
}
inline Jim_ObjPtr  CppApi::NewDictObj(int len, Jim_ObjConstArray elements) {
    return Jim_NewDictObj(interp_, elements, len);
}
inline Jim_ObjPtr  CppApi::NewDictObjMerge(int len, Jim_ObjConstArray elements) {
    return Jim_DictMerge(interp_, len, elements);
}
inline Jim_ObjPtr  CppApi::NewObj(jim_wide wideValue) { return Jim_NewIntObj(interp_, wideValue); }
inline Jim_ObjPtr  CppApi::NewObj(string_view str) { return Jim_NewStringObj(interp_, str.data(), CAST(int)str.size()); }
inline Jim_ObjPtr  CppApi::NewStrObj(char* input, int len) {
    return Jim_NewStringObjNoAlloc(interp_, input, len);
}
inline CppApi& CppApi::append(Jim_ObjPtr list, Jim_ObjPtr obj) { Jim_ListAppendElement(interp_, list, obj); return *this; }
inline CppApi& CppApi::append(Jim_ObjPtr list, string_view str) {
    append(list, NewObj(str)); return *this;
}
inline CppApi& CppApi::append(Jim_ObjPtr list, jim_wide val) {
    append(list, NewObj(val)); return *this;
}
inline Jim_ObjPtr CppApi::getVar(Jim_ObjPtr name, int flags) {
    return Jim_GetVariable(interp_, name, flags);
}
inline Jim_ObjPtr CppApi::getVar(string_view name, int flags) {
    return Jim_GetVariableStr(interp_, name.data(), flags);
}
inline Retval CppApi::setVar(Jim_ObjPtr name, Jim_ObjPtr value) {
    return Jim_SetVariable(interp_, name, value);
}
inline Retval CppApi::setVar(string_view name, Jim_ObjPtr value) {
    return Jim_SetVariableStr(interp_, name.data(), value);
}
inline Retval CppApi::setVar(string_view name, string_view value) {
    return Jim_SetVariableStrWithStr(interp_, name.data(), value.data());
}
inline void CppApi::free(Jim_ObjPtr o) { Jim_FreeObj(interp_, o); }
inline void CppApi::setResult(Jim_ObjPtr o) { Jim_SetResult(interp_, o); resultSet_ = true; }
inline void CppApi::setResult(string_view str) { Jim_SetResultString(interp_, str.data(), CAST(int)str.size()); resultSet_ = true; }
inline void CppApi::setResult(const char* str) { Jim_SetResultString(interp_, str, -1); resultSet_ = true; }
inline void CppApi::setResult(jim_wide val) { Jim_SetResultInt(interp_, val); resultSet_ = true; }
inline void CppApi::setResult(bool val) { Jim_SetResultBool(interp_, (long_long) val); resultSet_ = true; }
inline void CppApi::setResult(const vector<string>& vals) {
    auto list = NewListObj();
    for (auto v : vals) {
        append(list, NewObj(v));
    }
    setResult(list);
}
// Jim_SetResultFormatted
// Jim_NewStringObjNoAlloc
inline bool CppApi::isEqual(Jim_ObjPtr o, string_view str) {
    return Jim_CompareStringImmediate(interp_, o, str.data()) == 0;
}
inline bool CppApi::isEqual(Jim_ObjPtr o1, Jim_ObjPtr o2, bool nocase) {
    return Jim_StringCompareObj(interp_, o1, o2, nocase);
}
inline void CppApi::invalidate(Jim_ObjPtr obj) { Jim_InvalidateStringRep(obj); }
inline Retval CppApi::evalPrefix(const char* prefix, int oc, Jim_ObjConstArray cv) {
    return Jim_EvalPrefix(interp_, prefix, oc, cv);
}
inline Retval CppApi::packageProvided(string_view name, string_view ver, int flags) {
    return Jim_PackageProvide(interp_, name.data(), ver.data(), flags);
}
inline Retval CppApi::createCmd(string_view name, Jim_CmdProc cmdProc,
                                       void* privData, Jim_DelCmdProc* delProc) {
    return Jim_CreateCommand(interp_, name.data(), cmdProc, privData, delProc);
}
inline Retval CppApi::createSubCmd(string_view parentName, jim_subcmd_type* subcmdArray) {
    return Jim_CreateCommand(interp_, parentName.data(), Jim_SubCmdProc, (void*) subcmdArray, nullptr);
}
inline Retval CppApi::getEnum(Jim_ObjPtr objPtr, const char* const* tablePtr, int* indexPtr, const char* name, int flags) {
    return Jim_GetEnum(interp_, objPtr, tablePtr, indexPtr, name, flags);
}
inline tuple<Retval, jim_wide> CppApi::getInt(Jim_ObjPtr o) {
    jim_wide val = 0;
    Retval retval = Jim_GetWide(interp_, o, &val);
    return tuple<Retval, jim_wide>(retval, val);
}
inline void CppApi::wrongNumArgs(int argc, Jim_ObjConstArray argv, string_view msg) {
    Jim_WrongNumArgs(interp_, argc, argv, msg.data());
    inError_ = true;
}
inline vector<string> CppApi::getStrList(const Jim_ObjPtr objPtr) {
    int index = 0;
    Jim_ObjPtr obj = nullptr;
    vector<string> ret;
    while ((obj = Jim_ListGetIndex(interp_, objPtr, index)) != nullptr) {
        ret.push_back(getStr(obj));
    }
    return ret;
}

// Jim_CompareStringImmediate

// String format with dynamic resizing of buffer.
template<typename... Args>
std::string string_format(const std::string& format, Args&&... args) {
    size_t size = std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), std::forward<Args>(args)...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

END_JIM_NAMESPACE
