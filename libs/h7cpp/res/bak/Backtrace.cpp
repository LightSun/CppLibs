/*
         Apache License
   Version 2.0, January 2004
http://www.apache.org/licenses/

Project: https://github.com/e-fever/xbacktrace
*/
//enableBacktraceLogOnUnhandledException -> xbt_enableBacktrace

#include "utils/Backtrace.h"

#ifdef _WIN32
   #define Q_OS_WIN 1
   //define something for Windows (32-bit and 64-bit, this part is common)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
    #elif TARGET_OS_IPHONE
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
        #define Q_OS_MAC 1
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __ANDROID__
    // android
#elif __linux__
    // linux
    #define Q_OS_LINUX 1
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif

//#include <QtCore>
#include <functional>

#include "utils/linux_backtrace_h7.h"

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#include <winnt.h>

#include <string>
#include <vector>
#include <Psapi.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <iterator>

#ifdef _
MSC_VER
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

// Some versions of imagehlp.dll lack the proper packing directives themselves
// so we need to do it.
#pragma pack( push, before_imagehlp, 8 )
#include <imagehlp.h>
#pragma pack( pop, before_imagehlp )
#endif
#endif

namespace XBacktrace {

namespace Private {

#if defined(Q_OS_WIN)

/// This section is originaly belonged to ExceptionHandler.h :  https://github.com/Furkanzmc/ExceptionHandler/
/**
 * This is a class to handle unhandled exceptions in a Windowns application. For more information on how to use it: https://github.com/Furkanzmc/ExceptionHandler/
 */

struct ModuleData {
    std::string imageName;
    std::string moduleName;
    void *baseAddress;
    DWORD loadSize;
};
typedef std::vector<ModuleData> ModuleList;
std::string showStack(HANDLE hThread, CONTEXT &c);
std::string filterCrash(EXCEPTION_POINTERS *ep);
void *loadModulesSymbols(HANDLE hProcess, DWORD pid);

// if you use C++ exception handling: install a translator function
// with set_se_translator(). In the context of that function (but *not*
// afterwards), you can either do your stack dump, or save the CONTEXT
// record as a local copy. Note that you must do the stack dump at the
// earliest opportunity, to avoid the interesting stack-frames being gone
// by the time you do the dump.
std::string filterCrash(EXCEPTION_POINTERS *ep)
{
    std::string crashReason = "";

    switch (ep->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
        crashReason = "ACCESS VIOLATION";
        break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        crashReason = "DATATYPE MISALIGNMENT";
        break;
    case EXCEPTION_BREAKPOINT:
        crashReason = "BREAKPOINT";
        break;
    case EXCEPTION_SINGLE_STEP:
        crashReason = "SINGLE STEP";
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        crashReason = "ARRAY BOUNDS EXCEEDED";
        break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        crashReason = "FLT DENORMAL OPERAND";
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        crashReason = "FLT DIVIDE BY ZERO";
        break;
    case EXCEPTION_FLT_INEXACT_RESULT:
        crashReason = "FLT INEXACT RESULT";
        break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        crashReason = "FLT INVALID OPERATION";
        break;
    case EXCEPTION_FLT_OVERFLOW:
        crashReason = "FLT OVERFLOW";
        break;
    case EXCEPTION_FLT_STACK_CHECK:
        crashReason = "FLT STACK CHECK";
        break;
    case EXCEPTION_FLT_UNDERFLOW:
        crashReason = "FLT UNDERFLOW";
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        crashReason = "INT DIVIDE BY ZERO";
        break;
    case EXCEPTION_INT_OVERFLOW:
        crashReason = "INT OVERFLOW";
        break;
    case EXCEPTION_PRIV_INSTRUCTION:
        crashReason = "PRIV INSTRUCTION";
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        crashReason = "IN PAGE ERROR";
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        crashReason = "ILLEGAL INSTRUCTION";
        break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        crashReason = "NONCONTINUABLE EXCEPTION";
        break;
    case EXCEPTION_STACK_OVERFLOW:
        crashReason = "STACK OVERFLOW";
        break;
    case EXCEPTION_INVALID_DISPOSITION:
        crashReason = "INVALID DISPOSITION";
        break;
    case EXCEPTION_GUARD_PAGE:
        crashReason = "GUARD PAGE";
        break;
    default:
        crashReason = "(UNKNOWN)";
        break;
    }

    HANDLE thread;
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &thread, 0, false, DUPLICATE_SAME_ACCESS);
    std::string errorOutput = "Crash Reason: " + crashReason + "\n";
    errorOutput += "Walking stack.\n";
    errorOutput += showStack(thread, *(ep->ContextRecord));
    errorOutput += "\nEnd of stack walk.\n";
    CloseHandle(thread);

    return errorOutput;
}

class SymHandler
{
    HANDLE m_Process;
public:
    SymHandler(HANDLE process, char const *path = NULL, bool intrude = false) : m_Process(process)
    {
#ifdef _MSC_VER
        if (!SymInitialize(m_Process, path, intrude))
            throw(std::logic_error("Unable to initialize symbol handler"));
#endif
    }
    ~SymHandler()
    {
#ifdef _MSC_VER
        SymCleanup(m_Process);
#endif
    }
};

#ifdef _M_X64
#ifdef _MSC_VER
STACKFRAME64 initStackFrame(CONTEXT c)
{
    STACKFRAME64 s;
    s.AddrPC.Offset = c.Rip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Rsp;
    s.AddrStack.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.Rbp;
    s.AddrFrame.Mode = AddrModeFlat;
    return s;
}
#endif
#else
STACKFRAME64 initStackFrame(CONTEXT c)
{
    STACKFRAME64 s;
    s.AddrPC.Offset = c.Eip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Esp;
    s.AddrStack.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.Ebp;
    s.AddrFrame.Mode = AddrModeFlat;
    return s;
}
#endif

void symOptions(DWORD add, DWORD remove = 0)
{
#ifdef _MSC_VER
    DWORD symOptions = SymGetOptions();
    symOptions |= add;
    symOptions &= ~remove;
    SymSetOptions(symOptions);
#endif
}

#ifdef _MSC_VER
class Symbol
{
    typedef IMAGEHLP_SYMBOL64 SymType;
    SymType *m_Sym;
    static const int MAX_NAME_LEN = 1024;
public:
    Symbol(HANDLE process, DWORD64 address) : m_Sym((SymType *)::operator new(sizeof(*m_Sym) + MAX_NAME_LEN))
    {
        memset(m_Sym, '\0', sizeof(*m_Sym) + MAX_NAME_LEN);
        m_Sym->SizeOfStruct = sizeof(*m_Sym);
        m_Sym->MaxNameLength = MAX_NAME_LEN;
        DWORD64 displacement;

        if (!SymGetSymFromAddr64(process, address, &displacement, m_Sym))
            throw(std::logic_error("Bad symbol"));
    }

    std::string name()
    {
        return std::string(m_Sym->Name);
    }

    std::string undecoratedName()
    {
        std::vector<char> und_name(MAX_NAME_LEN);
        UnDecorateSymbolName(m_Sym->Name, &und_name[0], MAX_NAME_LEN, UNDNAME_COMPLETE);
        return std::string(&und_name[0], strlen(&und_name[0]));
    }
};
#endif

std::string showStack(HANDLE hThread, CONTEXT &c)
{
#ifdef _MSC_VER
    HANDLE process = GetCurrentProcess();
    int frameNumber = 0;
    DWORD offsetFromSymbol = 0;
    IMAGEHLP_LINE64 line = {0};

    SymHandler handler(process);

    symOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

    void *base = loadModulesSymbols(process, GetCurrentProcessId());

    STACKFRAME64 stackFrame = initStackFrame(c);

    line.SizeOfStruct = sizeof line;
    IMAGE_NT_HEADERS *headers = ImageNtHeader(base);
    DWORD imageType = headers->FileHeader.Machine;
    std::string errorOutput = "";

    do {
        if (!StackWalk64(imageType, process, hThread, &stackFrame, &c, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
            return std::string();
        }

        errorOutput += "\n" + std::to_string(frameNumber) + ": ";
        if (stackFrame.AddrPC.Offset != 0) {
            try {
                errorOutput += Symbol(process, stackFrame.AddrPC.Offset).undecoratedName();
            } catch (std::exception e) {
                errorOutput += "(" + std::string(e.what()) + ")";
            }

            errorOutput += " -> ";
            if (SymGetLineFromAddr64(process, stackFrame.AddrPC.Offset, &offsetFromSymbol, &line))
                errorOutput += std::string(line.FileName) + "(" + std::to_string(line.LineNumber) + ")";
        }
        else {
            errorOutput += " (No Symbols: PC == 0)";
        }

        ++frameNumber;
    }
    while (stackFrame.AddrReturn.Offset != 0);

    return errorOutput;
else
    fprintf(stderr, "showStack not impl for mingw.\n");
#endif
}

class GetModInfo
{
    HANDLE process;
    static const int BUFFER_LENGTH = 4096;

public:
    GetModInfo(HANDLE h) : process(h) {}

    ModuleData operator()(HMODULE module)
    {
        ModuleData ret;
        char temp[BUFFER_LENGTH];
        MODULEINFO mi;

        GetModuleInformation(process, module, &mi, sizeof(mi));
        ret.baseAddress = mi.lpBaseOfDll;
        ret.loadSize = mi.SizeOfImage;

        GetModuleFileNameExA(process, module, temp, sizeof(temp));
        ret.imageName = temp;
        GetModuleFileNameExA(process, module, temp, sizeof(temp));
        ret.moduleName = temp;
        std::vector<char> img(ret.imageName.begin(), ret.imageName.end());
        std::vector<char> mod(ret.moduleName.begin(), ret.moduleName.end());
#ifdef _MSC_VER
        SymLoadModule64(process, 0, &img[0], &mod[0], (DWORD64)ret.baseAddress, ret.loadSize);
#endif
        return ret;
    }
};

void *loadModulesSymbols(HANDLE process, DWORD pid)
{
    ModuleList modules;

    DWORD cbNeeded;
    std::vector<HMODULE> module_handles(1);

    EnumProcessModules(process, &module_handles[0], (DWORD) module_handles.size() * sizeof(HMODULE), &cbNeeded);
    module_handles.resize(cbNeeded / sizeof(HMODULE));
    EnumProcessModules(process, &module_handles[0], (DWORD) module_handles.size() * sizeof(HMODULE), &cbNeeded);

    std::transform(module_handles.begin(), module_handles.end(), std::back_inserter(modules), GetModInfo(process));
    return modules[0].baseAddress;
}

/// End of ExceptionHandler.h

#endif
}

inline void enableBacktraceLogOnUnhandledException(const char* exe_name,int ret_code) {

    static int __ret_code = ret_code;
    static const char* __exe_name = exe_name;

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    class Backtrace {
    public:

        static void handler(int sig) {
#ifdef Q_OS_LINUX
            if(__exe_name){
                _get_backtrace2(__exe_name);
            }else{
                _get_backtrace();
            }
            // print out all the frames to stderr
            fprintf(stderr, "Error: signal %d:\n", sig);
#else

            void *array[100];
            size_t size;

            // get void*'s for all entries on the stack
            size = backtrace(array, 100);

            // print out all the frames to stderr
            fprintf(stderr, "Error: signal %d:\n", sig);
            backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
            exit(__ret_code);
        }
    };

    signal(SIGSEGV, Backtrace::handler);
#endif

#if defined(Q_OS_WIN)
    class Backtrace {
    public:

        static LONG handler(struct _EXCEPTION_POINTERS *ExInfo)
        {
            std::cerr << Private::filterCrash(ExInfo) << std::endl;
            ExitProcess(__ret_code);
        }
    };

    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)Backtrace::handler);
#endif
}

inline void enableBacktraceLogOnUnhandledException(const char* exe_name) {
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    enableBacktraceLogOnUnhandledException(exe_name, -1);
#endif

#if defined(Q_OS_WIN)
#ifdef _MSC_VER
    enableBacktraceLogOnUnhandledException([]() {return EXCEPTION_EXECUTE_HANDLER;});
#endif
#endif
}

inline void attachConsole() {
    // Reference: https://forum.qt.io/topic/56484/solved-attach-console-to-gui-application-on-windows/4

#if defined(Q_OS_WIN)
    FreeConsole();

    // create a separate new console window
    AllocConsole();

    // attach the new console to this application's process
    AttachConsole(GetCurrentProcessId());

    // reopen the std I/O streams to redirect I/O to the new console
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    freopen("CON", "r", stdin);
#endif
}

}

//-------------------------
namespace h7 {

void Backtrace::enableBacktrace(const char* exe_name,int ret_code){
    XBacktrace::enableBacktraceLogOnUnhandledException(exe_name, ret_code);
}

//only for windows
void Backtrace::attachConsole(){
    XBacktrace::attachConsole();
}

}
