#include "tools/magics.h"

#include <iostream>

int main() {
    std::cout << "DIAGONAL_MAGICS" << std::endl << std::flush;
    Magics::find_diag_magics();

    std::cout << std::endl;

    std::cout << "HORIZONTAL_MAGICS" << std::endl << std::flush;
    Magics::find_hori_magics();

    return 0;
}
