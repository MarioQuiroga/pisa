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

    Cache_data(std::vector<uint32_t> block,
                unsigned int * next_ptr){
        this->block = block;
        this->freqs = std::vector<uint32_t>(),
        this->next_ptr = next_ptr;
        this->m_freq_decoded = false;
    }
    Cache_data(){
        
    }
};
