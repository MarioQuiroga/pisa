#pragma once
#include <iostream>
#include <list>
#include <unordered_map>
#include "../caching_files/Cache_data.hpp"

namespace Policies{
template<typename Key, class Data, typename hash = std::hash<Key>>
class lru_cache {
public:
    std::list<Key> entries;
    std::unordered_map<Key, std::pair<Data, typename std::list<Key>::iterator>, hash> storage;

    lru_cache(){}

    Data * get(Key key){
        typename std::unordered_map<Key, std::pair<Data, typename std::list<Key>::iterator>, hash>::iterator i = storage.find(key);
        if(i == storage.end()){
            return NULL;
        }
        typename std::list<Key>::iterator it = i->second.second;
        if(it != entries.begin()){ // If not recently used update place
            entries.erase(it);
            entries.push_front(key);
            i->second.second = entries.begin();
        }    
        return &(i->second.first);
    }

    void insert(Key key, Data data, int max_entries, int * cur_entries){  
        if(max_entries == *cur_entries){
            Key old_key = entries.back();
            entries.pop_back();            
            storage.erase(old_key);
        }else{
            *cur_entries += 1;
        }
        entries.push_front(key);
        typename std::list<Key>::iterator it = entries.begin();
        std::pair<Data, typename std::list<Key>::iterator> pair(data, it);
        std::pair<Key,std::pair<Data, typename std::list<Key>::iterator>> d(key,pair);
        storage.insert(d);
    } 
};

}