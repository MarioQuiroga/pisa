#pragma once
class Cache_data{
public:
    std::vector<uint32_t> block;    
    std::vector<uint32_t> freqs;
    unsigned int * next_ptr;
    bool m_freq_decoded = false;

    /*Cache_data(int c){
        count = c;
    }*/

    Cache_data(std::vector<uint32_t> b,
                unsigned int * n){
        block = b;
        freqs = std::vector<uint32_t>(),
        next_ptr = n;
        m_freq_decoded = false;
    }
    Cache_data(){        
    }
    ~Cache_data(){    
    }
};
