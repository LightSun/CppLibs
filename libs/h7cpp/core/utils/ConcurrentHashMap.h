#pragma once

#include <iostream>
#include <atomic>
#include <vector>
#include <list>
#include <utility>
#include <functional>
#include <stdexcept>

template<typename K, typename V>
struct Entry{
    K* key;
    V* value;
    Entry(K* key, V* value): key(key), value(value){};
};

template<typename Key, typename Value, size_t Size>
class ConcurrentHashMap {
public:
    using KeyDeleter = std::function<void(Key&)>;
    using ValueDeleter = std::function<void(Value&)>;

private:
    struct _ParentDelegate{
        KeyDeleter kd_;
        ValueDeleter vd_;
    };
    struct HashNode {
        Key key;
        Value value;
        HashNode* next;

        _ParentDelegate* pd {nullptr};
        //

        HashNode(const Key& k, const Value& v) : key(k), value(v), next(nullptr) {}
        HashNode(_ParentDelegate* pd, const Key& k, const Value& v)
            : pd(pd), key(k), value(v), next(nullptr) {}
        ~HashNode(){
            if(pd){
                if(pd->kd_) pd->kd_(key);
                if(pd->vd_) pd->vd_(value);
            }
        }
    };

    struct Node{
        std::atomic<HashNode*> impl;
        std::atomic_bool impDoing_ {false}; //important op ing.

        ~Node(){
            HashNode* node = impl.load();
            while(node){
                HashNode* next = node->next;
                delete node;
                node = next;
            }
        }
        void clear(){
            bool expect = false;
            while(!impDoing_.compare_exchange_weak(expect, true)){
            }
            HashNode* node = impl.load();
            HashNode* fn = node;
            std::vector<HashNode*> delNodes;
            while(node){
                delNodes.push_back(node);
                node = node->next;
            }
            while(fn != nullptr && !impl.compare_exchange_weak(fn, nullptr)){
                fn = impl.load();
            }
            impDoing_.store(false);
            for(HashNode*& n : delNodes){
                delete n;
            }
        }
        //add to head
        void append(HashNode* newNode){
            while (impDoing_.load(std::memory_order_acquire)) {
                //wait
            }
            HashNode* oldHead;
            do {
                oldHead = impl.load();
                newNode->next = oldHead;
            } while (!impl.compare_exchange_weak(oldHead, newNode));
        }
        bool find(const Key& key, Value& value){
            while (impDoing_.load(std::memory_order_acquire)) {
                //wait
            }
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
        bool remove(const Key& key, Value* value){
            while (impDoing_.load(std::memory_order_acquire)) {
                //wait
            }
            HashNode* currentNode = impl.load();
            HashNode* prevNode = nullptr;
            while (currentNode) {
                if (currentNode->key == key) {
                    if (prevNode) {
                        prevNode->next = currentNode->next;
                    } else {
                        impl.store(currentNode->next);
                    }
                    if(value) *value = currentNode->value;
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
    _ParentDelegate parentDel_;
    std::atomic_int size_;

    size_t Hash(const Key& key) {
        return std::hash<Key>()(key) % Size;
    }

public:
    ConcurrentHashMap() : buckets_(Size) {}
    ~ConcurrentHashMap(){
    }

    void setKeyDeleter(KeyDeleter kd){parentDel_.keyDeleter_ = kd;}
    void setValueDeleter(ValueDeleter kd){parentDel_.valueDeleter_ = kd;}
    void clear(){
        for(auto& n: buckets_){
            n.clear();
        }
        size_ = 0;
    }
    int size()const{return size_.load(std::memory_order_acquire);}

    //std::vector<>

    void insert(const Key& key, const Value& value) {
        size_t index = Hash(key);
        buckets_[index].append(new HashNode(&parentDel_, key, value));
        size_.fetch_add(1);
    }

    bool find(const Key& key, Value& value) {
        size_t index = Hash(key);
        return buckets_[index].find(key, value);
    }

    bool remove(const Key& key, Value* value) {
        size_t index = Hash(key);
        if(buckets_[index].remove(key, value)){
            size_.fetch_add(-1);
            return true;
        }
        return false;
    }
};

/**
template<>
struct std::hash<S>
{
    std::size_t operator()(S const& s) const noexcept
    {
        std::size_t h1 = std::hash<std::string>{}(s.first_name);
        std::size_t h2 = std::hash<std::string>{}(s.last_name);
        return h1 ^ (h2 << 1); // or use boost::hash_combine
    }
};
  */

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
