#include <pybind11/pybind11.h>
#include <string>
#include "ticker_loader.h"

int smart_update(std::string dirPath, int cutOffDays)
{
    return updateWhenNeeded(dirPath.data(), cutOffDays);
}

PYBIND11_MODULE(ticker_loader, m)
{
    m.def("smart_update", &smart_update);
}