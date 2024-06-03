#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <thread>
#include <functional>
#include <cassert>
#include <iostream>

template <typename T>
class HazardPointer {
    static const int MAX_HAZARD_POINTERS = 100;
    struct HazardPointerRecord {
        std::atomic<std::thread::id> id;
        std::atomic<T*> pointer;
    };

    static std::atomic<HazardPointerRecord*> hazard_pointers[MAX_HAZARD_POINTERS];

    std::atomic<T*>& get_hazard_pointer_for_current_thread() {
        thread_local static HazardPointerRecord* hazard_record = nullptr;
        if (!hazard_record) {
            for (int i = 0; i < MAX_HAZARD_POINTERS; ++i) {
                auto* p = hazard_pointers[i].load();
                std::thread::id cur_id = std::this_thread::get_id();
                if (p == nullptr) {
                    auto* new_p = new HazardPointerRecord();
                    new_p->id.store(cur_id);
                    new_p->pointer.store(nullptr);
                    if (hazard_pointers[i].compare_exchange_strong(p, new_p)) {
                        hazard_record = new_p;
                        break;
                    } else {
                        delete new_p;
                    }
                } else if (p->id.load() == cur_id && hazard_pointers[i].compare_exchange_strong(p, p)) {
                    hazard_record = p;
                    break;
                }
            }
            assert(hazard_record != nullptr); // Ensure we always get a hazard pointer
        }
        return hazard_record->pointer;
    }

public:
    std::atomic<T*>& get() {
        return get_hazard_pointer_for_current_thread();
    }

    static void clear_all() {
        for (int i = 0; i < MAX_HAZARD_POINTERS; ++i) {
            delete hazard_pointers[i].load();
        }
    }
};

template <typename T>
std::atomic<typename HazardPointer<T>::HazardPointerRecord*> HazardPointer<T>::hazard_pointers[MAX_HAZARD_POINTERS];

template <typename Key, typename Value>
class LockFreeHashMap {
private:
    struct Node {
        Key key;
        Value value;
        std::shared_ptr<Node> next;
        Node(const Key& k, const Value& v) : key(k), value(v), next(nullptr) {}
    };

    std::vector<std::atomic<std::shared_ptr<Node>>> table;
   // HazardPointer<Node> hazard_pointer;
    std::hash<Key> hashFunction;
    size_t capacity;
    std::atomic<size_t> element_count;
    std::atomic<bool> clearing;

public:
    LockFreeHashMap(size_t cap) : capacity(cap), table(cap) {
        for (auto& node : table) {
            node.store(nullptr);
        }
    }

    bool insert(const Key& key, const Value& value) {
        size_t index = hashFunction(key) % capacity;
        auto newNode = std::make_shared<Node>(key, value);

        while (true) {
            auto head = table[index].load();
            newNode->next = head;
            if (table[index].compare_exchange_weak(head, newNode)) {
                element_count.fetch_add(1, std::memory_order_relaxed);
                return true;
            }
        }
    }

    bool find(const Key& key, Value& value) {
        size_t index = hashFunction(key) % capacity;
        auto node = table[index].load();
        while (node != nullptr) {
            if (node->key == key) {
                value = node->value;
                return true;
            }
            node = node->next;
        }
        return false;
    }

    bool remove(const Key& key) {
        size_t index = hashFunction(key) % capacity;

        while (true) {
            auto head = table[index].load();
            std::shared_ptr<Node> prev = nullptr;
            auto node = head;

            while (node != nullptr && node->key != key) {
                prev = node;
                node = node->next;
            }

            if (node == nullptr) {
                return false; // Key not found
            }

            auto nextNode = node->next;

            if (prev == nullptr) { // Node to be removed is the head
                if (table[index].compare_exchange_weak(head, nextNode)) {
                    element_count.fetch_sub(1, std::memory_order_relaxed);
                    return true;
                }
            } else {
                prev->next = nextNode;
                element_count.fetch_sub(1, std::memory_order_relaxed);
                return true;
            }
        }
    }

    void clear() {
        clearing.store(true, std::memory_order_relaxed);
        for (size_t i = 0; i < capacity; ++i) {
            auto head = table[i].load();
            while (head != nullptr) {
                auto next = head->next;
                head = next;
            }
            table[i].store(nullptr);
        }
        element_count.store(0, std::memory_order_relaxed);
        clearing.store(false, std::memory_order_relaxed);
    }

    void traverse(std::function<void(const Key&, const Value&)> func) {
        for (size_t i = 0; i < capacity; ++i) {
            auto node = table[i].load();
            while (node != nullptr) {
                func(node->key, node->value);
                node = node->next;
            }
        }
    }
    size_t size() const {
       if (clearing.load(std::memory_order_relaxed)) {
           return 0; // If clearing is in progress, return 0
       }
       return element_count.load(std::memory_order_relaxed);
    }
};

//int main() {
//    LockFreeHashMap<int, std::string> map(10);
//    map.insert(1, "one");
//    map.insert(2, "two");
//    map.insert(3, "three");

//    std::string value;
//    if (map.find(2, value)) {
//        std::cout << "Found: " << value << std::endl;
//    } else {
//        std::cout << "Not found" << std::endl;
//    }

//    map.remove(2);
//    if (map.find(2, value)) {
//        std::cout << "Found: " << value << std::endl;
//    } else {
//        std::cout << "Not found" << std::endl;
//    }

//    map.traverse([](const int& key, const std::string& value) {
//        std::cout << key << ": " << value << std::endl;
//    });

//    map.clear();
//    map.traverse([](const int& key, const std::string& value) {
//        std::cout << key << ": " << value << std::endl;
//    });

//    return 0;
//}
