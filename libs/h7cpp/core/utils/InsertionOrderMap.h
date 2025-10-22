#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <functional>

namespace h7 {

template<typename Key, typename Value>
class InsertionOrderMap {
private:
    std::vector<Key> insertion_order;
    std::unordered_map<Key, Value> data;

public:
    void insert(const Key& key, const Value& value) {
        if (data.find(key) == data.end()) {
            insertion_order.push_back(key);
        }
        data[key] = value;
    }

    Value& operator[](const Key& key) {
        if (data.find(key) == data.end()) {
            insertion_order.push_back(key);
        }
        return data[key];
    }
    const Value& at(const Key& key) const {
        return data.at(key);
    }
    bool contains(const Key& key) const {
        return data.find(key) != data.end();
    }
    size_t size() const {
        return data.size();
    }
    void for_each(const std::function<void(const Key&, const Value&)>& func) const {
        for (const auto& key : insertion_order) {
            auto it = data.find(key);
            if (it != data.end()) {
                func(it->first, it->second);
            }
        }
    }
    const std::vector<Key>& keys() const {
        return insertion_order;
    }
    std::vector<Value> values() const {
        std::vector<Value> result;
        for (const auto& key : insertion_order) {
            auto it = data.find(key);
            if (it != data.end()) {
                result.push_back(it->second);
            }
        }
        return result;
    }
    bool erase(const Key& key) {
        if (data.erase(key) > 0) {
            auto it = std::find(insertion_order.begin(), insertion_order.end(), key);
            if (it != insertion_order.end()) {
                insertion_order.erase(it);
                return true;
            }
        }
        return false;
    }
};
}
