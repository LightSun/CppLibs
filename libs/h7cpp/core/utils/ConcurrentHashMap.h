#pragma once

#include <iostream>
#include <atomic>
#include <vector>
#include <list>
#include <utility>
#include <functional>
#include <stdexcept>

template<typename Key, typename Value, size_t Size>
class ConcurrentHashMap {
private:
    struct HashNode {
        Key key;
        Value value;
        HashNode* next;

        HashNode(const Key& k, const Value& v) : key(k), value(v), next(nullptr) {}
    };

    struct Node{
        std::atomic<HashNode*> impl;

        void append(HashNode* newNode){
            HashNode* oldHead;
            do {
                oldHead = impl.load();
                newNode->next = oldHead;
            } while (!impl.compare_exchange_weak(oldHead, newNode));
        }
        bool find(const Key& key, Value& value){
            HashNode* currentNode = impl.load();
            while (currentNode) {
                if (currentNode->key == key) {
                    value = currentNode->value;
                    return true;
                }
                currentNode = currentNode->next;
            }
            return false;
        }
        bool remove(const Key& key){
            HashNode* currentNode = impl.load();
            HashNode* prevNode = nullptr;
            while (currentNode) {
                if (currentNode->key == key) {
                    if (prevNode) {
                        prevNode->next = currentNode->next;
                    } else {
                        impl.store(currentNode->next);
                    }
                    delete currentNode;
                    return true;
                }
                prevNode = currentNode;
                currentNode = currentNode->next;
            }
            return false;
        }
    };

    std::vector<Node> buckets_;

    size_t Hash(const Key& key) {
        return std::hash<Key>()(key) % Size;
    }

public:
    ConcurrentHashMap() : buckets_(Size) {}
    ~ConcurrentHashMap(){
    }

    void Insert(const Key& key, const Value& value) {
        size_t index = Hash(key);
        buckets_[index].append(new HashNode(key, value));
    }

    bool Find(const Key& key, Value& value) {
        size_t index = Hash(key);
        return buckets_[index].find(key, value);;
    }

    bool Erase(const Key& key) {
        size_t index = Hash(key);
        return buckets_[index].remove(key);
    }
};

//int main() {
//    ConcurrentHashMap<int, std::string, 10> map;

//    map.Insert(1, "one");
//    map.Insert(2, "two");
//    map.Insert(3, "three");
//    map.Insert(11, "eleven"); // 哈希冲突，会添加到相应桶的链表中

//    std::string value;
//    if (map.Find(2, value)) {
//        std::cout << "Found value: " << value << std::endl;
//    } else {
//        std::cout << "Value not found" << std::endl;
//    }

//    return 0;
//}
