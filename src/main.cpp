#include "movegen/movegen.h"
#include "uci/uci.h"
#include "transposition-table/zobrist.h"

int main() {
    Movegen::init();
    Zobrist::init();
    UCI::loop();
}
