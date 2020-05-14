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
    double count_hit;
    double count_queries;

public:
    Cache(int size=0){
        max_entries = size;
        cur_entries = 0;
        count_hit = 0;
        count_queries = 0;
    }

    Data * get(Key key){
        count_queries += 1;
        typename std::unordered_map<Key, Data*, hash>::iterator i = storage.find(key);
        if(i == storage.end()){
            return NULL;
        }
        // return value, but first update its placed in your policy
        policy.update_place(key);
        count_hit += 1;
        return i->second;
    }

    double get_hit_ratio(){
        return count_hit/count_queries;
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
