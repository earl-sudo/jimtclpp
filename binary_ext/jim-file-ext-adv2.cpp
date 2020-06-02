#define _CRT_SECURE_NO_WARNINGS 1

#include <jimautoconf.h>
#include <jim-api.h>
#include <jim-cppapi.h>

#include <filesystem>

#if jim_ext_afile

BEGIN_JIM_NAMESPACE

// ====================================================================
BEGIN_NS(FsImpl)
using namespace std;
namespace fs = std::filesystem;

bool file_exists(string_view path);
string file_rootname(string_view path);
string file_rootpath(string_view path);
string file_relativepath(string_view path);
string file_parentpath(string_view path);
string file_filename(string_view path);
string file_stem(string_view path);
string file_extension(string_view path);
bool file_hasrootpath(string_view path);
bool file_hasrootname(string_view path);
bool file_hasrootdirectory(string_view path);
bool file_hasrelativepath(string_view path);
bool file_hasparentpath(string_view path);
bool file_hasfilename(string_view path);
bool file_hasstem(string_view path);
bool file_hasextension(string_view path);
bool file_isabsolute(string_view path);
bool file_isrelative(string_view path);
bool file_equal(string_view lhs, string_view rhs);
string file_concat(string_view path, string_view addpath);
vector<string> file_split(string_view path);
string file_currentpath();
string file_absolute(string_view path);
void file_copydirs(string_view from, string_view to, fs::copy_options options);
void file_copy(string_view from, string_view to, fs::copy_options options);
void file_copysymlink(string_view from, string_view to);
void file_createdir(string_view dirname);
void file_changedir(string_view dirname);
int64_t file_size(string_view dirname);
int64_t file_hardlinkcount(string_view dirname);
fs::perms file_permissions(string_view dirname);
fs::perms file_setpermissions(string_view dirname, fs::perms permissions);
fs::perms file_addpermissions(string_view dirname, fs::perms permissions);
fs::perms file_removepermissions(string_view dirname, fs::perms permissions);
string file_readsymlink(string_view path);
void file_remove(string_view dirname);
void file_removeall(string_view dirname);
void file_resize(string_view dirname, int64_t newsize);
int64_t file_freespace(string_view dirname);
int64_t file_capacity(string_view dirname);
string file_tempdirpath();
bool file_isblockfile(string_view path);
bool file_ischarfile(string_view path);
bool file_isdirectory(string_view path);
bool file_isempty(string_view path);
bool file_isfifo(string_view path);
bool file_isother(string_view path);
bool file_isregular(string_view path);
bool file_issocket(string_view path);
bool file_issymlink(string_view path);
void file_rename(string_view origpath, string_view newpath);
vector<string> file_dir(string_view pathD, fs::directory_options options = fs::directory_options::none);
vector<string> file_recursivedir(string_view pathD, fs::directory_options options = fs::directory_options::none);
// --------------------------------------------------------------------

bool file_exists(string_view path) { // #fstype1
    return fs::exists(path);
}
string file_rootname(string_view path) { // #fstype2
    fs::path pathObj(path);
    return pathObj.root_name().generic_u8string();
}
string file_rootpath(string_view path) { // #fstype2
    fs::path pathObj(path);
    return pathObj.root_path().generic_u8string();
}
string file_relativepath(string_view path) { // #fstype2
    fs::path pathObj(path);
    return pathObj.relative_path().generic_u8string();
}
string file_parentpath(string_view path) { // #fstype2
    fs::path pathObj(path);
    return pathObj.parent_path().generic_u8string();
}
string file_filename(string_view path) { // #fstype2
    fs::path pathObj(path);
    return pathObj.filename().generic_u8string();
}
string file_stem(string_view path) { // #fstype2
    fs::path pathObj(path);
    return pathObj.stem().generic_u8string();
}
string file_extension(string_view path) { // #fstype2
    fs::path pathObj(path);
    return pathObj.stem().generic_u8string();
}
bool file_hasrootpath(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.has_root_path();
}
bool file_hasrootname(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.has_root_name();
}
bool file_hasrootdirectory(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.has_root_directory();
}
bool file_hasrelativepath(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.has_relative_path();
}
bool file_hasparentpath(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.has_parent_path();
}
bool file_hasfilename(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.has_filename();
}
bool file_hasstem(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.has_stem();
}
bool file_hasextension(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.has_extension();
}
bool file_isabsolute(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.is_absolute();
}
bool file_isrelative(string_view path) { // #fstype1
    fs::path pathObj(path);
    return pathObj.is_relative();
}
bool file_equal(string_view lhs, string_view rhs) { // #fstype1
    fs::path pathLhs(lhs), pathRhs(rhs);
    return pathLhs == pathRhs;
}
string file_concat(string_view path, string_view addpath) { // #fsuniq
    fs::path pathLhs(path), pathRhs(addpath);
    return (pathLhs / pathRhs).generic_u8string();
}
vector<string> file_split(string_view path) { // #fsuniq
    fs::path pathObj(path);
    vector<string> ret;
    for (auto& e : pathObj) {
        ret.push_back(e.generic_u8string());
    }
    return ret;
}
string file_currentpath() { // #fsuniq
    return fs::current_path().generic_u8string();
}
string file_absolute(string_view path) { // #fstype2
    fs::path pathObj(path);
    return fs::absolute(path).generic_u8string();
}
void file_copydirs(string_view from, string_view to, fs::copy_options options) {
    fs::path fromPath(from), toPath(to);
    fs::copy(from, to, options); //  Copies files and directories, with a variety of options
}
void file_copy(string_view from, string_view to, fs::copy_options options) {
    fs::path fromPath(from), toPath(to);
    fs::copy_file(from, to, options); //  Copies files and directories, with a variety of options
}
void file_copysymlink(string_view from, string_view to) {
    fs::path fromPath(from), toPath(to);
    fs::copy_symlink(from, to); //  Copies files and directories, with a variety of options
}

void file_createdir(string_view dirname) { // #fstype3
    fs::path pathObj(dirname);
    fs::create_directory(pathObj);
}
void file_changedir(string_view dirname) { // #fstype3
    fs::path pathObj(dirname);
    fs::current_path(pathObj);
}
int64_t file_size(string_view dirname) {  // #fstype4
    fs::path pathObj(dirname);
    return fs::file_size(pathObj);
}
int64_t file_hardlinkcount(string_view dirname) { // #fstype4
    fs::path pathObj(dirname);
    return fs::hard_link_count(pathObj);
}
// std::filesystem::last_write_time() too complicated
fs::perms file_permissions(string_view dirname) {
    fs::path pathObj(dirname);
    fs::perms ret;
    ret = fs::status(pathObj).permissions();
    return ret;
}
fs::perms file_setpermissions(string_view dirname, fs::perms permissions) { // #fstype5
    fs::path pathObj(dirname);
    fs::perms ret;
    fs::permissions(pathObj, permissions, fs::perm_options::replace);
    ret = fs::status(pathObj).permissions();
    return ret;
}
fs::perms file_addpermissions(string_view dirname, fs::perms permissions) { // #fstype5
    fs::path pathObj(dirname);
    fs::perms ret;
    fs::permissions(pathObj, permissions, fs::perm_options::add);
    ret = fs::status(pathObj).permissions();
    return ret;
}
fs::perms file_removepermissions(string_view dirname, fs::perms permissions) { // #fstype5
    fs::path pathObj(dirname);
    fs::perms ret;
    fs::permissions(pathObj, permissions, fs::perm_options::remove);
    ret = fs::status(pathObj).permissions();
    return ret;
}
string file_readsymlink(string_view path) { // #fstype2
    fs::path pathObj(path);
    return fs::read_symlink(path).generic_u8string();
}
void file_remove(string_view dirname) { // #fstype3
    fs::path pathObj(dirname);
    fs::remove(pathObj);
}
void file_removeall(string_view dirname) { // #fstype3
    fs::path pathObj(dirname);
    fs::remove_all(pathObj);
}
void file_resize(string_view dirname, int64_t newsize) { // #fstype3
    fs::path pathObj(dirname);
    fs::resize_file(pathObj, newsize);
}
int64_t file_freespace(string_view dirname) { // #fstype4
    fs::path pathObj(dirname);
    return fs::space(pathObj).free;
}
int64_t file_capacity(string_view dirname) { // #fstype4
    fs::path pathObj(dirname);
    return fs::space(pathObj).capacity;
}
// std::filesystem::status, std::filesystem::symlink_status
string file_tempdirpath() { // #fsuniq
    return fs::temp_directory_path().generic_u8string();
}
bool file_isblockfile(string_view path) { // #fstype1
    fs::path pathObj(path);
    return fs::is_block_file(pathObj);
}
bool file_ischarfile(string_view path) { // #fstype1
    fs::path pathObj(path);
    return fs::is_character_file(pathObj);
}
bool file_isdirectory(string_view path) { // #fstype1
    fs::path pathObj(path);
    return fs::is_directory(pathObj);
}
bool file_isempty(string_view path) { // #fstype1
    fs::path pathObj(path);
    return fs::is_empty(pathObj);
}
bool file_isfifo(string_view path) { // #fstype1
    fs::path pathObj(path);
    return fs::is_fifo(pathObj);
}
bool file_isother(string_view path) { // #fstype1
    fs::path pathObj(path);
    return fs::is_other(pathObj);
}
bool file_isregular(string_view path) { // #fstype1
    fs::path pathObj(path);
    return fs::is_regular_file(pathObj);
}
bool file_issocket(string_view path) { // #fstype1
    fs::path pathObj(path);
    return fs::is_socket(pathObj);
}
bool file_issymlink(string_view path) { // #fstype1
    fs::path pathObj(path);
    return fs::is_symlink(pathObj);
}
void file_rename(string_view origpath, string_view newpath) { // #fsuniq
    fs::path pathLhs(origpath), pathRhs(newpath);
    rename(pathLhs, pathRhs);
}
vector<string> file_dir(string_view pathD, fs::directory_options options) { // #fsuniq
    vector<string> ret;
    for (auto& p : fs::directory_iterator(fs::path(pathD), options)) {
        ret.push_back(p.path().generic_u8string());
    }
    return ret;
}
vector<string> file_recursivedir(string_view pathD, fs::directory_options options) { // #fsuniq
    vector<string> ret;
    for (auto& p : fs::recursive_directory_iterator(fs::path(pathD), options)) {
        ret.push_back(p.path().generic_u8string());
    }
    return ret;
}
END_NS(FsImpl)
// ====================================================================

#define FILE_SYSTEM_TRY try {
#define FILE_SYSTEM_CATCH } catch (std::filesystem::filesystem_error& fse)  \
{                                                                           \
    jim.setResult(fse.what());                                              \
    return JRET(JIM_ERR);                                                         \
}

static Retval file_exists(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ { 
    CppApi      jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_exists(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK); 
}
static Retval file_rootname(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ { 
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_rootname(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK); 
}
static Retval file_rootpath(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ { 
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_rootpath(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK); 
}
static Retval file_relativepath(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ { 
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_relativepath(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK); 
}
static Retval file_parentpath(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ { 
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_parentpath(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_filename(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ { 
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_filename(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_stem(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ { 
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_stem(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_extension(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_extension(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_hasrootpath(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_hasrootpath(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_hasrootname(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_hasrootname(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_hasrootdirectory(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_hasrootdirectory(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_hasrelativepath(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_hasrelativepath(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_hasparentpath(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_hasparentpath(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_hasfilename(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_hasparentpath(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_hasstem(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_hasstem(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_hasextension(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_hasextension(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_isabsolute(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_isabsolute(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_isrelative(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_isrelative(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_equal(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_equal(getStr(argv[0]), getStr(argv[1])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_concat(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_concat(getStr(argv[0]), getStr(argv[1])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_split(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_split(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_currentpath(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_currentpath());
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_absolute(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_absolute(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_copydirs(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    std::filesystem::copy_options opt = std::filesystem::copy_options::none;
    if (argc == 3) {
        auto val = jim.getInt(argv[2]);
        if (get<0>(val) != JRET(JIM_OK)) {
            jim.setResult("Invalid number");
            return JRET(JIM_ERR);
        }
        opt = (std::filesystem::copy_options)std::get<1>(jim.getInt(argv[2]));
    } 
    FILE_SYSTEM_TRY
    FsImpl::file_copydirs(getStr(argv[0]), getStr(argv[1]), opt);
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_copy(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    std::filesystem::copy_options opt = std::filesystem::copy_options::none;
    if (argc == 3) {
        auto val = jim.getInt(argv[2]);
        if (get<0>(val) != JRET(JIM_OK)) {
            jim.setResult("Invalid number");
            return JRET(JIM_ERR);
        }
        opt = (std::filesystem::copy_options)std::get<1>(jim.getInt(argv[2]));
    }
    FILE_SYSTEM_TRY
    FsImpl::file_copy(getStr(argv[0]), getStr(argv[1]), opt);
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_copysymlink(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    FsImpl::file_copysymlink(getStr(argv[0]), getStr(argv[1]));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_createdir(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    FsImpl::file_createdir(getStr(argv[0]));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_changedir(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    FsImpl::file_changedir(getStr(argv[0]));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_size(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_size(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_hardlinkcount(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_hardlinkcount(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_permissions(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd #TODO */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult((jim_wide)FsImpl::file_permissions(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_setpermissions(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd #TODO */ {
    CppApi  jim(interp);
    auto val = jim.getInt(argv[2]);
    if (get<0>(val) != JRET(JIM_OK)) {
        jim.setResult("Invalid number");
        return JRET(JIM_ERR);
    }
    FILE_SYSTEM_TRY
    jim.setResult((jim_wide) FsImpl::file_setpermissions(getStr(argv[0]), (std::filesystem::perms)get<1>(val)));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_addpermissions(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd #TODO */ {
    CppApi  jim(interp);
    auto val = jim.getInt(argv[2]);
    if (get<0>(val) != JRET(JIM_OK)) {
        jim.setResult("Invalid number");
        return JRET(JIM_ERR);
    }
    FILE_SYSTEM_TRY
    jim.setResult((jim_wide) FsImpl::file_addpermissions(getStr(argv[0]), (std::filesystem::perms)get<1>(val)));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_removepermissions(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd #TODO */ {
    CppApi  jim(interp);
    auto val = jim.getInt(argv[2]);
    if (get<0>(val) != JRET(JIM_OK)) {
        jim.setResult("Invalid number");
        return JRET(JIM_ERR);
    }
    FILE_SYSTEM_TRY
    jim.setResult((jim_wide) FsImpl::file_removepermissions(getStr(argv[0]), (std::filesystem::perms)get<1>(val)));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_readsymlink(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_readsymlink(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_remove(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    FsImpl::file_remove(getStr(argv[0]));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_removeall(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    FsImpl::file_removeall(getStr(argv[0]));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_resize(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    auto val = jim.getInt(argv[1]);
    if (get<0>(val) != JRET(JIM_OK)) {
        jim.setResult("Invalid number");
        return JRET(JIM_ERR);
    }
    FILE_SYSTEM_TRY
    FsImpl::file_resize(getStr(argv[0]), get<1>(val));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_freespace(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_freespace(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_capacity(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_capacity(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_tempdirpath(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_tempdirpath());
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_isblockfile(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_isblockfile(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_ischarfile(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_ischarfile(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_isdirectory(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_isdirectory(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_isempty(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_isempty(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_isfifo(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_isfifo(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_isother(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_isother(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_isregular(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_isregular(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_issocket(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_issocket(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_issymlink(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_issymlink(getStr(argv[0])));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_rename(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_SYSTEM_TRY
    FsImpl::file_rename(getStr(argv[0]), getStr(argv[1]));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_dir(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    std::filesystem::directory_options opt = std::filesystem::directory_options::none;
    if (argc == 2) {
        auto val = jim.getInt(argv[1]);
        if (get<0>(val) != JRET(JIM_OK)) {
            jim.setResult("Invalid number");
            return JRET(JIM_ERR);
        }
        opt = (std::filesystem::directory_options)std::get<1>(jim.getInt(argv[2]));
    }
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_dir(getStr(argv[0]), opt));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}
static Retval file_recursivedir(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    std::filesystem::directory_options opt = std::filesystem::directory_options::none;
    if (argc == 2) {
        auto val = jim.getInt(argv[1]);
        if (get<0>(val) != JRET(JIM_OK)) {
            jim.setResult("Invalid number");
            return JRET(JIM_ERR);
        }
        opt = (std::filesystem::directory_options)std::get<1>(jim.getInt(argv[2]));
    }
    FILE_SYSTEM_TRY
    jim.setResult(FsImpl::file_recursivedir(getStr(argv[0]), opt));
    FILE_SYSTEM_CATCH
    return JRET(JIM_OK);
}

// Define a commands sub-commands.
static const jim_subcmd_type g_fileadv2_subcommand_table[] = { // #JimSubCmdDef
{ /*#JimCmdOpts*/ "exists", "filepath", file_exists, 1, 1 /* Description: file_exists */ },
{ /*#JimCmdOpts*/ "rootname", "filepath", file_rootname, 1, 1 /* Description: file_rootname */ },
{ /*#JimCmdOpts*/ "rootpath", "filepath", file_rootpath, 1, 1 /* Description: file_rootpath */ },
{ /*#JimCmdOpts*/ "relativepath", "filepath", file_relativepath, 1, 1 /* Description: file_relativepath */ },
{ /*#JimCmdOpts*/ "parentpath", "filepath", file_parentpath, 1, 1 /* Description: file_parentpath */ },
{ /*#JimCmdOpts*/ "filename", "filepath", file_filename, 1, 1 /* Description: file_filename */ },
{ /*#JimCmdOpts*/ "stem", "filepath", file_stem, 1, 1 /* Description: file_stem */ },
{ /*#JimCmdOpts*/ "extension", "filepath", file_extension, 1, 1 /* Description: file_extension */ },
{ /*#JimCmdOpts*/ "hasrootpath", "filepath", file_hasrootpath, 1, 1 /* Description: file_hasrootpath */ },
{ /*#JimCmdOpts*/ "hasrootname", "filepath", file_hasrootname, 1, 1 /* Description: file_hasrootname */ },
{ /*#JimCmdOpts*/ "hasrootdirectory", "filepath", file_hasrootdirectory, 1, 1 /* Description: file_hasrootdirectory */ },
{ /*#JimCmdOpts*/ "hasrelativepath", "filepath", file_hasrelativepath, 1, 1 /* Description: file_hasrelativepath */ },
{ /*#JimCmdOpts*/ "hasparentpath", "filepath", file_hasparentpath, 1, 1 /* Description: file_hasparentpath */ },
{ /*#JimCmdOpts*/ "hasfilename", "filepath", file_hasfilename, 1, 1 /* Description: file_hasfilename */ },
{ /*#JimCmdOpts*/ "hasstem", "filepath", file_hasstem, 1, 1 /* Description: file_hasstem */ },
{ /*#JimCmdOpts*/ "hasextension", "filepath", file_hasextension, 1, 1 /* Description: file_hasextension */ },
{ /*#JimCmdOpts*/ "isabsolute", "filepath", file_isabsolute, 1, 1 /* Description: file_isabsolute */ },
{ /*#JimCmdOpts*/ "isrelative", "filepath", file_isrelative, 1, 1 /* Description: file_isrelative */ },
{ /*#JimCmdOpts*/ "equal", "filepath1 filepath2", file_equal, 2, 2 /* Description: file_equal */ },
{ /*#JimCmdOpts*/ "concat", "filepathpart1 filepathpart2", file_concat, 2, 2 /* Description: file_concat */ },
{ /*#JimCmdOpts*/ "split", "filepath", file_split, 1, 1 /* Description: file_split */ },
{ /*#JimCmdOpts*/ "currentpath", "", file_currentpath, 0, 0 /* Description: file_currentpath */ },
{ /*#JimCmdOpts*/ "absolute", "filepath", file_absolute, 1, 1 /* Description: file_absolute */ },
{ /*#JimCmdOpts*/ "copydirs", "filepathfrom filepathto ?options?", file_copydirs, 2, 3 /* Description: file_copydirs */ },
{ /*#JimCmdOpts*/ "copy", "filepathfrom filepathto ?options?", file_copy, 2, 3 /* Description: file_copy */ },
{ /*#JimCmdOpts*/ "copysymlink", "filepathfrom filepathto", file_copysymlink, 2, 2 /* Description: file_copysymlink */ },
{ /*#JimCmdOpts*/ "createdir", "filepath", file_createdir, 1, 1 /* Description: file_createdir */ },
{ /*#JimCmdOpts*/ "changedir", "filepath", file_changedir, 1, 1 /* Description: file_changedir */ },
{ /*#JimCmdOpts*/ "size", "filepath", file_size, 1, 1 /* Description: file_size */ },
{ /*#JimCmdOpts*/ "hardlinkcount", "filepath", file_hardlinkcount, 1, 1 /* Description: file_hardlinkcount */ },
{ /*#JimCmdOpts*/ "permissions", "filepath", file_permissions, 1, 1 /* Description: file_permissions */ },
{ /*#JimCmdOpts*/ "setpermissions", "filepath options", file_setpermissions, 2, 2 /* Description: file_setpermissions */ },
{ /*#JimCmdOpts*/ "addpermissions", "filepath options", file_addpermissions, 2, 2 /* Description: file_addpermissions */ },
{ /*#JimCmdOpts*/ "removepermissions", "filepath options", file_removepermissions, 2, 2 /* Description: file_removepermissions */ },
{ /*#JimCmdOpts*/ "readsymlink", "filepath", file_readsymlink, 1, 1 /* Description: file_readsymlink */ },
{ /*#JimCmdOpts*/ "remove", "filepath", file_remove, 1, 1 /* Description: file_remove */ },
{ /*#JimCmdOpts*/ "removeall", "filepath", file_removeall, 1, 1 /* Description: file_removeall */ },
{ /*#JimCmdOpts*/ "resize", "filepath size", file_resize, 2, 2 /* Description: file_resize */ },
{ /*#JimCmdOpts*/ "freespace", "filepath", file_freespace, 1, 1 /* Description: file_freespace */ },
{ /*#JimCmdOpts*/ "capacity", "filepath", file_capacity, 1, 1 /* Description: file_capacity */ },
{ /*#JimCmdOpts*/ "tempdirpath", "", file_tempdirpath, 0, 0 /* Description: file_tempdirpath */ },
{ /*#JimCmdOpts*/ "isblockfile", "filepath", file_isblockfile, 1, 1 /* Description: file_isblockfile */ },
{ /*#JimCmdOpts*/ "ischarfile", "filepath", file_ischarfile, 1, 1 /* Description: file_ischarfile */ },
{ /*#JimCmdOpts*/ "isdirectory", "filepath", file_isdirectory, 1, 1 /* Description: file_isdirectory */ },
{ /*#JimCmdOpts*/ "isempty", "filepath", file_isempty, 1, 1 /* Description: file_isempty */ },
{ /*#JimCmdOpts*/ "isfifo", "filepath", file_isfifo, 1, 1 /* Description: file_isfifo */ },
{ /*#JimCmdOpts*/ "isother", "filepath", file_isother, 1, 1 /* Description: file_isother */ },
{ /*#JimCmdOpts*/ "isregular", "filepath", file_isregular, 1, 1 /* Description: file_isregular */ },
{ /*#JimCmdOpts*/ "issocket", "filepath", file_issocket, 1, 1 /* Description: file_issocket */ },
{ /*#JimCmdOpts*/ "issymlink", "filepath", file_issymlink, 1, 1 /* Description: file_issymlink */ },
{ /*#JimCmdOpts*/ "rename", "filepathfrom filepathto", file_rename, 2, 2 /* Description: file_rename */ },
{ /*#JimCmdOpts*/ "dir", "filepath ?options?", file_dir, 1, 2 /* Description: file_dir */ },
{ /*#JimCmdOpts*/ "recursivedir", "filepath ?options?", file_recursivedir, 1, 2 /* Description: file_recursivedir */ },
    {  }
};

// Parser of subcommand
static int fileadv2SubCmdProc(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    return Jim_CallSubCmd(interp, Jim_ParseSubCmd(interp, g_fileadv2_subcommand_table, argc, argv), argc, argv);
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-file-ext-adv2-version.h>


// Called to setup extension.
JIM_EXPORT Retval Jim_fileadv2Init(Jim_InterpPtr interp) // #JimCmdInit
{
    CppApi      jim(interp);

    // Give package name and version.
    if (jim.packageProvided("fs", version, JIM_ERRMSG))
        return JRET(JIM_ERR);

    // Create a command with subcommands
    jim.ret = jim.createCmd(/* name of parent command */ "fs",
                            /* pases subcommands */  fileadv2SubCmdProc,
                            /* package private data */ nullptr,
                            /* called on removal of pacakge */ nullptr);
    if (jim.ret != JIM_OK) return jim.ret;

    return JRET(JIM_OK);
}

END_JIM_NAMESPACE

#endif // #if jim_ext_afile
