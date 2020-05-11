#pragma once

#include <stdint.h>

#include <string_view>
#include <string>
#include <variant>
#include <vector>

namespace prj_wrap {

    typedef int64_t Retval;

    template<typename T1, typename T2>
    struct val2 {        T1 v1;        T2 v2;    };

    template<typename T1, typename T2, typename T3>
    struct val3 {        T1 v1;        T2 v2;        T3 v3;    };

    template<typename T1, typename T2, typename T3, typename T4>
    struct val4 {        T1 v1;        T2 v2;        T3 v3;        T4 v4;    };

    int get_errno(void);
    int& get_errno_ref(void);
    bool isWindows(void);

    namespace CppFile {
        std::string_view getErrorMsg(void);

        typedef val2<std::string_view /*option*/, std::variant<std::string_view, int64_t, double, bool>> Option;

        val2<Retval, int64_t> file_atime(std::string_view filename);
        Retval file_attributes(std::string_view filename, std::vector <Option>& options); // #cppExtNeed
        Retval file_channels(std::string_view pattern); // #cppExtNeed
        Retval file_copy(std::vector<std::string_view/*source*/>& sources, std::string_view dest, bool forced = false); // #cppExtNeed
        val2<Retval, int64_t> file_ctime(std::string_view filename);
        Retval file_delete(std::string_view pathname, bool forced);
        Retval file_delete(std::vector<std::string_view/*filename*/>& pathnames, bool forced);
        val2<Retval, std::string> file_dirname(std::string_view pathname);
        bool exists(std::string_view pathname);
        bool isReadable(std::string_view pathname);
        bool isWritable(std::string_view pathname);
        bool isExecutable(std::string_view pathname);
        Retval file_executable(std::string_view pathname);
        Retval file_exists(std::string_view pathname);
        val2<Retval, std::string> file_extension(std::string_view pathname);
        Retval file_isdirectory(std::string_view pathname);
        Retval file_isfile(std::string_view pathname);
        val2<Retval, std::string> file_join(std::vector<std::string_view>& argv);
        Retval file_link_hard(std::string_view source, std::string_view& dest);
        Retval file_link_hard(std::string_view pathname, std::vector<std::string_view/*dest*/>& dest);
        Retval file_link_symbolic(std::string_view source, std::string_view& dest);
        Retval file_link_symbolic(std::string_view pathname, std::vector<std::string_view/*dest*/>& dest);
        //val2<Retval, vector<file_lstat_struct>> file_lstat1(string_view name);
        Retval file_mkdir(std::string_view path);
        Retval file_mkdir(std::vector<std::string_view/*dirs*/>& dirs);
        val2<Retval, int64_t> file_mtime(std::string_view pathname);
        Retval file_mtime(std::string_view filename, int64_t us);
        Retval file_nativename(std::string_view pathname); // #cppExtNeed
        val2<Retval, std::string> file_normalize(std::string_view path);
        Retval file_owned(std::string_view pathname);
        Retval file_pathtype(std::string_view pathname); // #cppExtNeed
        Retval file_readable(std::string_view pathname);
        val2<Retval, std::string> file_readlink(std::string_view pathname);
        Retval file_rename(std::string_view source, std::string_view target, bool forced = false);
        Retval file_rootname(std::string_view pathname); // #cppExtNeed
        Retval file_seperator(std::string_view name);  // #cppExtNeed
        val2<Retval, int64_t> file_size(std::string_view name);
        Retval file_split(std::string_view name); // #cppExtNeed
        Retval file_stat(std::string_view name, std::string_view varName); // #cppExtNeed
        Retval file_system(std::string_view name); // #cppExtNeed
        val2<Retval, std::string> file_tail(std::string_view name);
#ifdef PRJ_OS_LINUX
        val3<Retval, int, std::string> file_maketempfile(const char* filename_template = NULL, bool unlink_file = true);
#endif
#ifdef PRJ_OS_WIN
        val2<Retval, std::string> file_temppath();
        val2<Retval, std::string> file_tempname(std::string_view filePath, const char* templateD);
//        val4<Retval, HANDLE, std::string, int> file_win_maketempfile(std::string_view filePath, bool unlink_file = true, const char* templateD = NULL);
#endif
        std::string_view file_type(std::string_view pathname);
        Retval file_volumes(std::string_view name); // #cppExtNeed
        Retval file_writable(std::string_view name);
    };
}; // namespace prj_wrap