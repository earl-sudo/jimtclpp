#define _CRT_SECURE_NO_WARNINGS 1
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 1

#include <jimautoconf.h>
#include <jim-api.h>
#include <jim-cppapi.h>

#if jim_ext_boost_exec

#include <boost/process.hpp>

#include <exception>
#include <vector>
#include <string>
#include <string_view>
#include <functional>
#include <filesystem>
#include <system_error>
#include <boost/asio.hpp>
#include <tuple>

BEGIN_JIM_NAMESPACE

BEGIN_NS(ExecImpl)
using namespace std;
namespace bp = boost::process;
namespace fs = boost::filesystem;

CHKRET string process_findexe(string_view toFind);
CHKRET string process_findexe(string_view toFind, vector<string>& altlocations);
CHKRET vector<string> process_envpath();
CHKRET int process_mypid();
CHKRET string process_shell();
CHKRET tuple<int, vector<string>> process_sync_stdoutLines(string_view command, int timeout = -1);
void process_spawn(string_view cmd);
CHKRET int process_system(string_view cmd, const char* stdoutFile = nullptr, const char* stderrFile = nullptr, const char* stdinFile = nullptr);
CHKRET int process_pipe_stdin_stdout_stderr(
    const string& exeName,
    const string& args,
    const std::string& input,
    std::string& output, /* stdout placed in this. */
    std::string& error, /* stderr placed in this. */
    int64_t sizeOutput = 8 * 1024, /* 128 << 10 */
    int64_t sizeErr = 8 * 1024, /* 128 << 10 */
    int timeout = -1);

string process_findexe(string_view toFind) {
    fs::path  toFindPath(toFind.data());
    return bp::search_path(toFindPath).string();
}
string process_findexe(string_view toFind, vector<string>& altlocations) {
    fs::path  toFindPath(toFind.data());
    vector<fs::path> altpaths;
    for (auto& al : altlocations) {
        altpaths.push_back(al);
    }
    return bp::search_path(toFindPath, altpaths).string();
}
vector<string> process_envpath() {
    vector<string> ret;
    for (auto& pe : boost::this_process::path()) {
        ret.push_back(pe.string());
    }
    return ret;
}
int process_mypid() {
    return boost::this_process::get_id();
}
string process_shell() {
    return bp::shell().string();
}

tuple<int, vector<string>> process_sync_stdoutLines(string_view command, int timeout) {
    bp::ipstream stdout_pipe_stream;

    auto stdoutToPipe = bp::std_out > stdout_pipe_stream;
    auto stdoutToStdout = bp::std_out > stdout;
    auto stderrToStdErr = bp::std_err > stderr;

    bp::child cmd(command.data(), stdoutToPipe, stderrToStdErr);
    string line;
    int retCode = -1;
    vector<string> ret;
    while (stdout_pipe_stream && getline(stdout_pipe_stream, line, '\n') && !line.empty()) {
        ret.push_back(line);
    }
    if (timeout > 0) {
        if (!cmd.wait_for(std::chrono::seconds(timeout))) {
            cmd.terminate(); //then terminate
        }
    } else {
        cmd.wait();
    }
    retCode = cmd.exit_code();
    return tuple<int, vector<string>>(retCode, ret);
}
void process_spawn(string_view cmd) {
    bp::spawn(cmd.data());
}
// system() with pipe to/from files without need for a shell.
int process_system(string_view cmd, const char* stdoutFile, const char* stderrFile, const char* stdinFile) {
    if (stdoutFile != nullptr) {
        auto stdoutPipe = bp::std_out > fs::path(stdoutFile);
        if (stderrFile != nullptr) {
            auto stderrPipe = bp::std_err > fs::path(stderrFile);
            if (stdinFile) {
                auto stdinPipe = bp::std_in < fs::path(stdinFile);
                return bp::system(cmd.data(), stdoutPipe, stderrPipe, stdinPipe); // stdout>file,stderr>file,stdin<file
            } else {
                auto stdinPipe = bp::std_in.close();
                return bp::system(cmd.data(), stdoutPipe, stderrPipe, stdinPipe); // stdout>file,stderr>file,stdin.close()
            }
        } else {
            auto stderrPipe = bp::std_err > bp::null;
            if (stdinFile) {
                auto stdinPipe = bp::std_in < fs::path(stdinFile);
                return bp::system(cmd.data(), stdoutPipe, stderrPipe, stdinPipe); // stdout>file,stderr>null,stdin<file
            } else {
                auto stdinPipe = bp::std_in.close();
                return bp::system(cmd.data(), stdoutPipe, stderrPipe, stdinPipe); // stdout>file,stderr>null,stdin.close()
            }
        }
    } else {
        auto stdoutPipe = bp::std_out > bp::null;
        if (stderrFile != nullptr) {
            auto stderrPipe = bp::std_err > fs::path(stderrFile);
            if (stdinFile) {
                auto stdinPipe = bp::std_in < fs::path(stdinFile);
                return bp::system(cmd.data(), stdoutPipe, stderrPipe, stdinPipe); // stdout>null,stderr>file,stdin<file
            } else {
                auto stdinPipe = bp::std_in.close();
                return bp::system(cmd.data(), stdoutPipe, stderrPipe, stdinPipe); // stdout>null,stderr>file,stdin.close()
            }
        } else {
            auto stderrPipe = bp::std_err > bp::null;
            if (stdinFile) {
                auto stdinPipe = bp::std_in < fs::path(stdinFile);
                return bp::system(cmd.data(), stdoutPipe, stderrPipe, stdinPipe); // stdout>null,stderr>null,stdin<file
            } else {
                auto stdinPipe = bp::std_in.close();
                return bp::system(cmd.data(), stdoutPipe, stderrPipe, stdinPipe); // stdout>null,stderr>null,stdin.close()
            }
        }
    }
    return -1;
}

int process_pipe_stdin_stdout_stderr(
    const string& exeName,
    const string& args,
    const std::string& input,
    std::string& output, /* stdout placed in this. */
    std::string& error, /* stderr placed in this. */
    int64_t sizeOutput, /* 128 << 10 */
    int64_t sizeErr, /* 128 << 10 */
    int timeout
) {
    using namespace boost;

    asio::io_service ios;


    // Setup stdout buffers and pipe
    std::vector<char> vOut((const unsigned int)sizeOutput);
    auto outBuffer{ asio::buffer(vOut) };
    process::async_pipe pipeOut(ios);

    // function to handle stdout.
    std::function<void(const system::error_code& ec, std::size_t n)> onStdOut;

    // body of function to handle stdout
    onStdOut = [&](const system::error_code& ec, size_t n) {
        // share with parent: output, vOut, pipeOut, outBuffer

        // Grow output string to fit data.
        output.reserve(output.size() + n);
        // Append vOut to end of output string.
        output.insert(output.end(), vOut.begin(), vOut.begin() + n);
        if (!ec) {
            asio::async_read(pipeOut, outBuffer, onStdOut);
        }
    };

    // Setup stderr buffers and pipe.
    std::vector<char> vErr((const unsigned int) sizeErr);
    auto errBuffer{ asio::buffer(vErr) };
    process::async_pipe pipeErr(ios);

    // function to handle stderr.
    std::function<void(const system::error_code& ec, std::size_t n)> onStdErr;

    // body of function to handle stderr.
    onStdErr = [&](const system::error_code& ec, size_t n) {
        // share with parent: error, vErr, pipeErr, errBuff

        // Grow output string to fit data.
        error.reserve(error.size() + n);
        // Append vErr to end of error string.
        error.insert(error.end(), vErr.begin(), vErr.begin() + n);
        if (!ec) {
            asio::async_read(pipeErr, errBuffer, onStdErr);
        }
    };

    // Create actual process
    string cmd(exeName + " " + args);
    int exitCode = -1;

    if (input.length()) {
        // Setup stdin pipe.
        auto inBuffer{ asio::buffer(input) };
        process::async_pipe pipeIn(ios);

        process::child cmdObj(
            cmd,
            process::std_out > pipeOut, // Attach pipe to stdout.
            process::std_err > pipeErr, // Attach pipe to stderr.
            process::std_in < pipeIn    // Attach pipe to stdin.
        );

        asio::async_write(pipeIn, inBuffer,
                          [&](const system::error_code& ec, std::size_t n) {
            pipeIn.async_close();
        });

        asio::async_read(pipeOut, outBuffer, onStdOut);
        asio::async_read(pipeErr, errBuffer, onStdErr);

        ios.run();
        if (timeout > 0) {
            if (!cmdObj.wait_for(std::chrono::seconds(timeout))) {
                cmdObj.terminate(); //then terminate
            }
        } else {
            cmdObj.wait();
            exitCode = cmdObj.exit_code();
        }

    } else {
        process::child cmdObj(
            cmd,
            process::std_out > pipeOut, // Attach pipe to stdout.
            process::std_err > pipeErr, // Attach pipe to stderr.
            process::std_in.close()    // Attach pipe to stdin.
        );

        asio::async_read(pipeOut, outBuffer, onStdOut);
        asio::async_read(pipeErr, errBuffer, onStdErr);

        ios.run();
        if (timeout > 0) {
            if (!cmdObj.wait_for(std::chrono::seconds(timeout))) {
                cmdObj.terminate(); //then terminate
            }
        } else {
            cmdObj.wait();
            exitCode = cmdObj.exit_code();
        }
    }

    return exitCode;
}

END_NS(ExecImpl)

#define FILE_PROCESS_TRY try {
#define FILE_PROCESS_CATCH                                                  \
} catch (std::filesystem::filesystem_error& fse)  {                         \
    jim.setResult(fse.what());                                              \
    return JRET(JIM_ERR);                                                         \
} catch (std::exception& exc) {                                             \
    jim.setResult(exc.what());                                              \
    return JRET(JIM_ERR);                                                         \
}

static Retval process_findexe(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_PROCESS_TRY;
    jim.setResult(ExecImpl::process_findexe(getStr(argv[0])));
    FILE_PROCESS_CATCH;
    return JRET(JIM_OK);
}
static Retval process_findexeWithPaths(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_PROCESS_TRY;
    auto val = jim.getStrList(argv[1]);
    jim.setResult(ExecImpl::process_findexe(getStr(argv[0]), val));
    FILE_PROCESS_CATCH;
    return JRET(JIM_OK);
}
static Retval process_envpath(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_PROCESS_TRY;
    jim.setResult(ExecImpl::process_envpath());
    FILE_PROCESS_CATCH;
    return JRET(JIM_OK);
}
static Retval process_mypid(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_PROCESS_TRY;
    jim.setResult((jim_wide)ExecImpl::process_mypid());
    FILE_PROCESS_CATCH;
    return JRET(JIM_OK);
}
static Retval process_shell(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_PROCESS_TRY;
    jim.setResult(ExecImpl::process_shell());
    FILE_PROCESS_CATCH;
    return JRET(JIM_OK);
}
static Retval process_sync_stdoutLines(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_PROCESS_TRY;
    if (argc == 1) {
        auto val = ExecImpl::process_sync_stdoutLines(getStr(argv[0]));
        jim.setResult(get<1>(val)); // #TODO add return code.
    } else if (argc == 2) {
        auto val = jim.getInt(argv[2]);
        if (get<0>(val) != JRET(JIM_OK)) {
            jim.setResult("Invalid number");
            return JRET(JIM_ERR);
        }
        auto val1 = ExecImpl::process_sync_stdoutLines(getStr(argv[0]), (int)get<1>(val));
        jim.setResult(get<1>(val1)); // #TODO add return code
    }
    FILE_PROCESS_CATCH;
    return JRET(JIM_OK);
}
static Retval process_spawn(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_PROCESS_TRY;
    ExecImpl::process_spawn(getStr(argv[0]));
    FILE_PROCESS_CATCH;
    return JRET(JIM_OK);
}
static Retval process_system(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    CppApi  jim(interp);
    FILE_PROCESS_TRY;
    jim.setResult((jim_wide)ExecImpl::process_system(getStr(argv[0])));
    FILE_PROCESS_CATCH;
    return JRET(JIM_OK);
}
static Retval process_pipe_stdin_stdout_stderr(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) /* #JimCmd */ {
    // cmd ?args? ?inputText? ?timeout?
    CppApi  jim(interp);
    string stdinBuff, args, stdoutBuf, stderrBuf, command;
    int timeout = -1;
    FILE_PROCESS_TRY;
    command = getStr(argv[0]);
    if (argc >= 2) args = getStr(argv[1]);
    if (argc >= 3) stdinBuff = getStr(argv[2]);
    if (argc >= 4) {
        auto val = jim.getInt(argv[4]);
        if (get<0>(val) != JRET(JIM_OK)) {
            jim.setResult("Invalid number");
            return JRET(JIM_ERR);
        }
        timeout = (int)get<1>(val);
    }
    int ret = ExecImpl::process_pipe_stdin_stdout_stderr(command, args, stdinBuff, stdoutBuf, stderrBuf, 1024,1024,timeout);
    Jim_ObjPtr listObj = jim.NewListObj();
    jim.append(listObj, ret).append(listObj, stdoutBuf).append(listObj, stderrBuf);
    jim.setResult(listObj);
    FILE_PROCESS_CATCH;
    return JRET(JIM_OK);
}
// Define a commands sub-commands.
static const jim_subcmd_type g_boost_exec_subcommand_table[] = { // #JimSubCmdDef
{ /*#JimCmdOpts*/ "findexe", "tofind", process_findexe, 1, 1 /* Description: findexe */ },
{ /*#JimCmdOpts*/ "findexewpaths", "tofind listOfPaths", process_findexeWithPaths, 2, 2 /* Description: findexewpaths */ },
{ /*#JimCmdOpts*/ "envpath", "", process_envpath, 0, 0 /* Description: envpath */ },
{ /*#JimCmdOpts*/ "mypid", "", process_mypid, 0, 0 /* Description: mypid */ },
{ /*#JimCmdOpts*/ "shell", "", process_shell, 0, 0 /* Description: shell */ },
{ /*#JimCmdOpts*/ "synclines", "command ?timeout?", process_sync_stdoutLines, 1, 2 /* Description: synclines */ },
{ /*#JimCmdOpts*/ "spawn", "command", process_spawn, 1, 1 /* Description: spawn */ },
{ /*#JimCmdOpts*/ "system", "command ?stdoutFileOrEmpty? ?stderrFileOrEmpty? ?stdinFileOrEmpty", process_system, 1, 4 /* Description: system */ },
{ /*#JimCmdOpts*/ "pipe", "cmd ?args? ?inputText? ?timeout?", process_pipe_stdin_stdout_stderr, 1, 4 /* Description: pipe */ },
    {  }
};

// Parser of subcommand
static int fileadv2SubCmdProc(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    return Jim_CallSubCmd(interp, Jim_ParseSubCmd(interp, g_boost_exec_subcommand_table, argc, argv), argc, argv);
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-file-exec-boost-ext-version.h>

// Called to setup extension.
JIM_EXPORT Retval Jim_BoostExec(Jim_InterpPtr interp) // #JimCmdInit
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
    if (jim.ret != JIM_OK) return ret;

    return JRET(JIM_OK);
}

END_JIM_NAMESPACE

#endif // jim_ext_boost_exec
