#ifndef ARGRESOLVER_H
#define ARGRESOLVER_H

#include <iostream>
#include <vector>
#include <string>

class ArgResolver {
public:
    class Arg {
    private:
        std::string name;
        std::string Val;
    public:
        Arg() = default;

        Arg(const std::string &_name, const std::string &_Val) {
            name = _name;
            Val = _Val;
        }

        inline bool empty() const {
            return name.empty() && Val.empty();
        }

        inline bool is(const std::string &_name) const { return name == _name; }

        inline bool isnoname() const { return name.empty(); }

        inline std::string getName() const { return name; }

        inline int getIntVal() const { return std::stoi(Val); }

        inline double getFloatVal() const { return std::stod(Val); }

        inline std::string getStrVal() const { return Val; }

        inline friend std::ostream
        &operator<<(std::ostream &os, const ArgResolver::Arg &arg) {
            if (arg.isnoname())
                return os << "No Name Val:" << arg.getStrVal();
            else
                return os << "name:" << arg.getName() << "\tVal:" << arg.getStrVal();
        }
    };

private:
    std::vector<Arg> Args;
    ArgResolver::Arg nonameArg;
    std::string selfname;
public:
    ArgResolver(int argc, char *argv[], bool blank);

    Arg findArg(const std::string &_name);

    std::string getSelfname() const {
        return selfname;
    }

    void printAll() const;
};

#endif //ARGRESOLVER_H
