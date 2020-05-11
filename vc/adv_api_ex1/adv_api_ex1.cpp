// adv_api_ex1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <prj_wrap.h>
#include <assert.h>

#define WH printf("FN: %s\n", __FUNCTION__);
#define createFile(FN) if (!prj_wrap::isWindows()) system("touch " FN); else system("echo > " FN);
#define listFile(FN) if (!prj_wrap::isWindows()) system("ls " FN); else system("dir " FN);
#define rmdir(FN) system("rmdir " FN);

void test_file_atime() {
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_atime("testfile.fil");
    assert(ret.v1 == 0);
    return;
}
void test_file_attributes() { printf("NOT-YET %s\n", __FUNCTION__);  }
void test_file_channels() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_copy() { printf("NOT-YET %s\n", __FUNCTION__);  }
void test_file_ctime() {
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_ctime("testfile.fil");
    assert(ret.v1 == 0);
    return;
}
void test_file_delete() { 
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_delete("testfile.fil", false);
    if (ret != 0) { printf("%s\n", prj_wrap::CppFile::getErrorMsg().data()); }

}
void test_file_executable() { printf("NOT-YET %s\n", __FUNCTION__);  }
void test_file_exists() {
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_exists("testfile.fil");
    assert(ret == 0);
    return;
}
void test_file_extension() {
    WH;
    auto ret = prj_wrap::CppFile::file_extension("testfile.fil");
    assert(ret.v1 == 0);
    return;
}
void test_file_isdirectory() {
    WH;
    auto ret = prj_wrap::CppFile::file_isdirectory(".");
    assert(ret == 0);
    return;
}
void test_file_isfile() {
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_isfile("testfile.fil");
    assert(ret == 0);
    return;
}
void test_file_join() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_link_hard() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_link_symbolic() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_mkdir() {
    WH;
    rmdir("testDir");
    auto ret = prj_wrap::CppFile::file_mkdir("testDir");
    assert(ret == 0);
    return;
}
void test_file_mtime() {
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_mtime("testfile.fil");
    assert(ret.v1 == 0);
    return;
}
void test_file_nativename() { printf("NOT-YET %s\n", __FUNCTION__);  }
void test_file_owned() {
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_owned("testfile.fil");
    //assert(ret == 0);
    return;
}
void test_file_pathtype() {
    WH;
    auto ret = prj_wrap::CppFile::file_pathtype(".");
    assert(ret == 0);
    return;
}
void test_file_readable() {
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_readable("testfile.fil");
    assert(ret == 0);
    return;
}
void test_file_readlink() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_rename() { 
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_rename("testfile.fil", "testfile_copy.fil");
    if (ret != 0) { printf("%s\n", prj_wrap::CppFile::getErrorMsg().data()); }
    listFile("*.fil");
}
void test_file_rootname() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_seperator() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_size() { 
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_size("testfile.fil");
    if (ret.v1 != 0) { printf("%s\n", prj_wrap::CppFile::getErrorMsg().data()); }
    printf("file_size %d\n", ret.v2);
    listFile("testfile.fil")
}
void test_file_split() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_stat() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_system() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_tail() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_type() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_volumes() { printf("NOT-YET %s\n", __FUNCTION__); }
void test_file_writable() { 
    WH;
    createFile("testfile.fil");
    auto ret = prj_wrap::CppFile::file_writable("testfile.fil");
    assert(ret == 0);
    return;
}

void all_test(void) { 
    test_file_atime();
    test_file_attributes();
    test_file_channels();
    test_file_ctime();
    test_file_delete();
    test_file_executable();
    test_file_exists();
    test_file_extension();
    test_file_isdirectory();
    test_file_isfile();
    test_file_join();
    test_file_link_hard();
    test_file_link_symbolic();
    // ERR test_file_mkdir();
    test_file_mtime();
    test_file_nativename();
    test_file_owned();
    test_file_pathtype();
    test_file_readable();
    test_file_readlink();
    test_file_rename();
    test_file_rootname();
    test_file_seperator();
    test_file_size();
    test_file_split();
    test_file_stat();
    test_file_system();
    test_file_tail();
    test_file_type();
    test_file_volumes();
    test_file_writable();
}

int main()
{
    std::cout << "Hello World!\n";
    system(R"(echo system-hello)");
    system(R"(echo hi > cmd_test.fil)");
    system(R"(/usr/bin/bash -c "echo system-hello")");
    all_test();
    return 0;
}
