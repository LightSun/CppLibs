#pragma once

namespace h7 {
template<typename K, typename V>
    struct Pair{
        K key;
        V value;
        Pair(const K& key, const V& value){
            this->key = key;
            this->value = value;
        }
        Pair(){}
        Pair(const Pair<K,V>& p){
            this->key = p.key;
            this->value = p.value;
        }
        inline K& first(){
            return key;
        }
        inline V& second(){
            return value;
        }
        Pair<K,V>& operator=(const Pair<K,V>& p){
            this->key = p.key;
            this->value = p.value;
            return *this;
        }
    };

    template<typename K, typename V>
    Pair<K, V> make_pair(const K& key, const V& value){
        return Pair<K,V>(key, value);
    }
}
