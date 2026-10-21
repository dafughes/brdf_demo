#include "util.h"

#include <fstream>
#include <sstream>

std::string read_file(std::string path)
{
    path = std::string(PROJECT_ROOT) + "/" + path;
    std::ifstream f(path);

    if (f.is_open())
    {
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }
    else
    {
        return "";
    }
}