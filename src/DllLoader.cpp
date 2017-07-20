#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

void printUsage()
{
    const std::string msg =
        "DllLoader -> load dll(s)\n"
        "Usage DllLoader [OPTIONS] DllName func1 func2 ...\n"
        " -h, --help       Display this message\n";
    std::cout << msg << std::endl;
}

struct DllConfig {
    std::string name;
    std::vector<std::string> functions;
};

struct Config {
    bool help;
    DllConfig dll;

    /// Load default values
    Config()
    {
        help = false;
    }

    void load(INT argc, LPSTR argv[])
    {
        const std::string CMD_HELP_SHORT = "-h";
        const std::string CMD_HELP_LONG = "--help";

        if (argc < 2) {
            printUsage();
            throw std::invalid_argument("Invalid number of argumanets: '" + std::to_string(argc) + "'. Should be 2 or more");
        }

        /// Prepare data
        std::vector<std::string> commands;
        for (int i = 1; i < argc; ++i)
            commands.push_back(argv[i]);

        dll.name = commands.at(0); ///> name of dll to load

        /// Set user provided values
        for (unsigned i = 1; i < commands.size(); ++i) {
            std::string const & command = commands.at(i);
            if (command == CMD_HELP_SHORT || command == CMD_HELP_LONG) {
                help = true;
            } else {
                dll.functions.push_back(command);
            }
        }
    }
};

void loadDll(Config const & cfg)
{
    HMODULE hDll = LoadLibraryA(cfg.dll.name.c_str());
    if (hDll == NULL)
        throw std::runtime_error("Could not open dll '" + cfg.dll.name + "'");
    
    std::vector<FARPROC> functions;
    for (auto const f : cfg.dll.functions) {
        FARPROC tempFunc = GetProcAddress(hDll, f.c_str());
        if (tempFunc == NULL)
            throw std::runtime_error("Could not load function '" + f + "' from '" + cfg.dll.name + "'");
        functions.push_back(tempFunc);
    }
}

int mainImp(INT argc, LPSTR argv[])
{
    

    Config cfg;
    cfg.load(argc, argv);

    if (cfg.help) {
        printUsage();
    } else {
        loadDll(cfg);
    }

    return EXIT_SUCCESS;
}

int main(INT argc, LPSTR argv[])
{
    int exitCode;
    try {
        exitCode = mainImp(argc, argv);
    } catch (std::exception const & e) {
        std::cout << "ERR [DllLoader]::main Unhandled exception -> " << e.what() << std::endl;
        exitCode = EXIT_FAILURE;
    } catch (...) {
        std::cout << "ERR [DllLoader]::main Unrecognized unhandled exception" << std::endl;
        exitCode = EXIT_FAILURE;
    }

    return exitCode;
}
