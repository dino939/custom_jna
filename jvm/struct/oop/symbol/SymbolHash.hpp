// SymbolHash.h
#pragma once
#include "Symbol.h"
#include <unordered_map>

class SymbolHash {
private:
    std::unordered_map<Symbol*, u2> _table;

public:
    SymbolHash() = default;

    // Добавить символ если его нет
    void add_if_absent(Symbol* sym, u2 value) {
        if (_table.find(sym) == _table.end()) {
            _table[sym] = value;
        }
    }

    u2 symbol_to_value(Symbol* sym) const {
        auto it = _table.find(sym);
        return (it == _table.end()) ? 0 : it->second;
    }

    bool contains(Symbol* sym) const {
        return _table.find(sym) != _table.end();
    }

    void clear() {
        _table.clear();
    }
    std::unordered_map<Symbol*, u2> table() const
    {
        return _table;
    }
    size_t size() const {
        return _table.size();
    }

    void reserve(size_t count) {
        _table.reserve(count);
    }
};