#include "Utils.h"

void itos(int in, std::string &out)
{
    std::stringstream ss;
    ss << in;
    ss >> out;
}

std::string itos(int in)
{
    std::stringstream ss;
    std::string out;
    ss << in;
    ss >> out;
    return out;
}
