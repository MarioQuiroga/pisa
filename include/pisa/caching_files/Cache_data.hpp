#pragma once
class Cache_data{
public:
    std::vector<uint32_t> block;
    bool m_freq_decoded;
    std::vector<uint32_t> freqs;
    unsigned int * next_ptr;

    /*Cache_data(int c){
        count = c;
    }*/

    Cache_data(){
        
    }
};
