#pragma once
#include <cstddef>
#include <unordered_map>
#include "Cache_data.hpp"

using namespace Policies;

template<typename Key, 
         class Data, 
         typename hash, 
         template<typename, class, typename> class Policy>
class Cache {
    std::unordered_map<Key, Data*, hash> storage;
    Policy<Key, Data, hash> policy;
    int max_entries;
    int cur_entries;

public:
    Cache(int size=0){
        max_entries = size;
        cur_entries = 0;
    }

    Data * get(Key key){
        typename std::unordered_map<Key, Data*, hash>::iterator i = storage.find(key);
        if(i == storage.end()){
            return NULL;
        }
        // return value, but first update its placed in your policy
        policy.update_place(key);
        return i->second;
    }


    void insert(Key key, Data data){
        typename std::unordered_map<Key, Data*, hash>::iterator i = storage.find(key);
        if(i == storage.end()){
            Data * d = new Data(data.block, data.next_ptr);   
            policy.insert(key, d, storage, max_entries, &cur_entries);
        }
    }

    bool empty(){
        return storage.empty();
    }

    int get_count_entries(){
        return cur_entries;
    }

    int get_max_entries(){
        return max_entries;
    }

    void set_max_entries(int size){
        max_entries = size;
    }

};
