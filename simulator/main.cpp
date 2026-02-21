#include <windows.h>
#include <iostream>
#include <fstream>

#include "Simulator.hpp"

int main()
{
#ifndef IMG_PATH
    IMG_PATH "../../OS_FULL_PHYSICAL.img"
#endif

    try
    {
        // 你的模拟器启动逻辑
        run_simulator();
    }
    catch (const std::exception &e)
    {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "UNKNOWN CRASH" << std::endl;
    }

    return 0;
}
