#include <Windows.h>
#include <memory>
#include <vector>
#include <string>
#include <codecvt>
#include <iostream>

namespace {
void printUsage()
{
    const std::wstring msg =
        L"DllLoader -> load dll(s)\n"
        L"Usage DllLoader [OPTIONS] DllName func1 func2 ...\n"
        L" -h, --help       Display this message";
    std::wcout << msg << std::endl;
}

namespace str {
std::string ws2s(const std::wstring &wout)
{
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(wout);
}

std::wstring s2ws(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}
}

struct DllConfig
{
    std::wstring name;
    std::vector<std::wstring> functions;
};

struct Config
{
    bool help = false;
    DllConfig dll;

    void load(const std::vector<std::wstring> &args)
    {
        if (args.size() < 2) {
            printUsage();
            throw std::invalid_argument("Invalid number of argumanets: '" + std::to_string(args.size()) + "'. Should be 2 or more");
        }

        dll.name = args[1]; ///> name of dll to load

        const std::wstring CMD_HELP_SHORT = L"-h";
        const std::wstring CMD_HELP_LONG = L"--help";
        /// Set user provided values
        for (const std::wstring &command : args) {
            if (command == CMD_HELP_SHORT || command == CMD_HELP_LONG) {
                help = true;
            } else {
                dll.functions.push_back(command);
            }
        }
    }
};

void loadDll(const Config &cfg)
{
    HMODULE hDll = LoadLibraryW(cfg.dll.name.c_str());
    if (!hDll)
        throw std::runtime_error("Could not open dll '" + str::ws2s(cfg.dll.name) + "'");

    std::vector<FARPROC> functions;
    for (auto const f : cfg.dll.functions) {
        FARPROC tempFunc = GetProcAddress(hDll, str::ws2s(f).c_str());
        if (!tempFunc)
            throw std::runtime_error("Could not load function '" + str::ws2s(f) + "' from '" + str::ws2s(cfg.dll.name) + "'");
        functions.push_back(tempFunc);
    }
}

int mainImpl(const std::vector<std::wstring> &args)
{
    Config cfg;
    cfg.load(args);

    if (cfg.help) {
        printUsage();
    } else {
        loadDll(cfg);
    }

    return EXIT_SUCCESS;
}
}

int wmain(int argc, wchar_t *argv[])
{
    int exitCode = EXIT_FAILURE;
    try {
        exitCode = mainImpl({argv, argv + argc});
    } catch (std::exception const & e) {
        std::wcout << L"ERR [DllLoader]::main Unhandled exception -> " << e.what() << std::endl;
        exitCode = EXIT_FAILURE;
    } catch (...) {
        std::wcout << L"ERR [DllLoader]::main Unrecognized unhandled exception" << std::endl;
        exitCode = EXIT_FAILURE;
    }

    return exitCode;
}
