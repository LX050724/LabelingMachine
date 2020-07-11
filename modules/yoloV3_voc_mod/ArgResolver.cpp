#include "ArgResolver.h"

void ArgResolver::printAll() const {
    std::cout << "selfname:" << selfname << std::endl;
    if (!nonameArg.empty())
        std::cout << nonameArg << std::endl;
    for (Arg arg : Args) {
        std::cout << arg << std::endl;
    }
}

ArgResolver::Arg ArgResolver::findArg(const std::string &_name) {
    for (Arg arg : Args) {
        if (arg.is(_name))
            return arg;
    }
    return Arg();
}

ArgResolver::ArgResolver(int argc, char **argv, bool blank) : nonameArg() {
    selfname = argv[0];
    for (int i = 1; i < argc; ++i) {
        std::string str(argv[i]);
        if (str[0] == '-') {
            if (str[1] == '-') {
                Args.emplace_back(str.substr(2), argv[++i]);
            } else {
                if (blank)
                    Args.emplace_back(str.substr(1, 1), argv[++i]);
                else
                    Args.emplace_back(str.substr(1, 1), str.substr(2));
            }
        } else {
            nonameArg = Arg("", str);
        }
    }
}
