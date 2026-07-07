#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "googology/core/Notation.hpp"

namespace googology {

// Registry of notations by name. Used for the capability matrix and for
// looking up a notation implementation at runtime. Built-ins are registered
// in main.cpp (or a dedicated registration unit) before use.
class Registry {
    std::map<std::string, std::function<std::unique_ptr<Notation>()>> factories_;
public:
    void add(const std::string& name, std::function<std::unique_ptr<Notation>()> f) {
        factories_[name] = std::move(f);
    }
    std::unique_ptr<Notation> create(const std::string& name) const {
        auto it = factories_.find(name);
        return it == factories_.end() ? nullptr : it->second();
    }
    std::vector<std::string> list() const {
        std::vector<std::string> r;
        for (auto& p : factories_) r.push_back(p.first);
        return r;
    }
};

inline Registry& registry() {
    static Registry r;
    return r;
}

} // namespace googology
