#include "movegen/movegen.h"
#include "uci/uci.h"

int main() {
    Movegen::init_diagonal_cache();
    Movegen::init_horizontal_cache();
    UCI::loop();
}
