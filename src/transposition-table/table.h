#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <optional>
#include "move/move.h"

enum class NodeKind {
    EXACT,
    UPPER_BOUND,
    LOWER_BOUND,
};

class TableEntry {
    private:
        uint64_t hash;
        Move best;
        int depth;
        int score;
        NodeKind kind;
        bool occupied = false;

    public:
        TableEntry() = default;
        TableEntry(uint64_t h, Move b, int d, int s, 
                NodeKind k)
            : hash(h), best(b), depth(d), score(s), kind(k), occupied(true) {}
        uint64_t get_hash();
        Move get_best();
        int get_depth();
        int get_score();
        NodeKind get_kind();
        bool is_occupied();
};

class TranspositionTable {
    private:
        static constexpr std::size_t SIZE = (2 * 1024 * 1024) / sizeof(TableEntry);
        std::array<TableEntry, SIZE> lookup_table{};

    public:
        void set_entry(TableEntry entry, uint64_t hash);
        std::optional<TableEntry> get_entry(uint64_t hash);
        void clear();
};
