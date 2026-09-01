#include "transposition-table/table.h"
#include "move/move.h"
#include <cstdint>
#include <optional>

uint64_t TableEntry::get_hash() {
    return this->hash;
}

Move TableEntry::get_best() {
    return this->best;
}

int TableEntry::get_depth() {
    return this->depth;
}

int TableEntry::get_score() {
    return this->score;
}

NodeKind TableEntry::get_kind() {
    return this->kind;
}

bool TableEntry::is_occupied() {
    return this->occupied == true ? true : false;
}

void TranspositionTable::set_entry(TableEntry entry, uint64_t hash) {
    auto old = get_entry(hash);
    if (old.has_value()) {
        if (old->get_depth() < entry.get_depth()) {
            this->lookup_table[hash % SIZE] = entry;
        }
    } else {
        this->lookup_table[hash % SIZE] = entry;
    }
}

std::optional<TableEntry> TranspositionTable::get_entry(uint64_t hash) {
    auto& entry = lookup_table[hash % SIZE];
    if (!entry.is_occupied()) {
        return std::nullopt;
    }

    if (entry.get_hash() != hash) {
        return std::nullopt;
    }

    return entry;
}

void TranspositionTable::clear() {
    lookup_table.fill(TableEntry{});
}
