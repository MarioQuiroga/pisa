/*
    Inspired by solutions found in:
    http://dhruvbird.com/lfu.pdf
*/
#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include "../caching_files/Cache_data.hpp"
#include "util/node.hpp"

namespace Policies{
template<class Key, class Data, typename hash = std::hash<Key>>
class lfu_cache {    
public:
    
    int min = -1;
    std::unordered_map<Key, std::pair<Data*, typename std::list<Key>::iterator>, hash> storage; //cache K and V
    std::unordered_map<Key, int, hash> counts; //K and counters
    std::unordered_map<int, typename std::list<Key>> lists; //Counter and item 
 
    lfu_cache(){
        std::list<Key> list;
        std::pair<int, std::list<Key>> d(1, list);
        lists.insert(d);
    }

    Data * get(Key key){
        typename std::unordered_map<Key, std::pair<Data*, typename std::list<Key>::iterator>, hash>::iterator i = storage.find(key);
        if(i == storage.end()){
            return NULL;
        }
        // Get the count from counts map
        int count = counts[key];
        // increase the counter
        counts[key] = count + 1;
        // remove the element from the counter to linkedhashset
        lists.at(count).erase(i->second.second); // iterator for key
        // when current min does not have any data, next one would be the min
        if (count == min && lists.at(count).size() == 0){
            min++;
        }
        if (lists.find(count + 1) == lists.end()){
            std::list<Key> list;
            std::pair<int, std::list<Key>> d(count + 1, list);
            lists.insert(d);
        }
        lists.at(count+1).push_back(key);
        storage.at(key).second = lists.at(count+1).end(); // store iterator for key        
        return i->second.first;
    } 

    void insert(Key key, Data * data, int max_entries, int * cur_entries){   
        if(max_entries == *cur_entries){
            auto evit = lists.at(min).begin();            
            storage.erase(*evit);
            counts.erase(*evit);              
            lists.at(min).pop_front();     
        }else{
                *cur_entries += 1;
        }        
        lists.at(1).push_back(key);
        typename std::list<Key>::iterator it = lists.at(1).end();
        std::pair<Data, typename std::list<Key>::iterator> pair(data, it);
        std::pair<Key,std::pair<Data, typename std::list<Key>::iterator>> d(key,pair);
        counts[key] = 1;
        min = 1;
        storage.insert(d);
    } 
};
}