#include "set.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

void init_cache() {
    srand(time(NULL));

    for (int i = 0; i < D_CACHE_SETS; i++) {
        for (int j = 0; j < D_CACHE_WAYS; j++) {
            d_cache[i][j].valid = 0;
            d_cache[i][j].dirty = 0;
            d_cache[i][j].tag = 0;
            d_cache[i][j].lru_counter = j;
            d_cache[i][j].second_chance_bit = 0;
        }
    }

    for (int i = 0; i < I_CACHE_SETS; i++) {
        for (int j = 0; j < I_CACHE_WAYS; j++) {
            i_cache[i][j].valid = 0;
            i_cache[i][j].dirty = 0;
            i_cache[i][j].tag = 0;
            i_cache[i][j].lru_counter = j;
            i_cache[i][j].second_chance_bit = 0;
        }
    }
}

void update_lru(CacheLine* cache_set, int accessed_way, int ways) {
    int prev_lru = cache_set[accessed_way].lru_counter;
    cache_set[accessed_way].lru_counter = 0;

    for (int i = 0; i < ways; i++) {
        if (i == accessed_way) continue;
        if (cache_set[i].lru_counter < prev_lru) {
            cache_set[i].lru_counter++;
        }
    }
}

int find_lru_victim(CacheLine* cache_set, int ways) {
    int victim_way = 0;
    int max_lru = -1;
    for (int i = 0; i < ways; i++) {
        if (cache_set[i].lru_counter > max_lru) {
            max_lru = cache_set[i].lru_counter;
            victim_way = i;
        }
    }
    return victim_way;
}

int find_random_victim() {
    return rand() % D_CACHE_WAYS;
}

int find_fifo_victim_d_cache(int set_index) {
    int victim_way = d_cache_fifo_ptr[set_index];
    d_cache_fifo_ptr[set_index] = (victim_way + 1) % D_CACHE_WAYS;
    return victim_way;
}

int find_fifo_victim_i_cache(int set_index) {
    int victim_way = i_cache_fifo_ptr[set_index];
    i_cache_fifo_ptr[set_index] = (victim_way + 1) % I_CACHE_WAYS;
    return victim_way;
}

int find_sca_victim_d_cache(int set_index) {
    while (1) {
        int current_way = d_cache_fifo_ptr[set_index];
        if (d_cache[set_index][current_way].second_chance_bit == 0) {
            d_cache_fifo_ptr[set_index] = (current_way + 1) % D_CACHE_WAYS;
            return current_way;
        }
        else {
            d_cache[set_index][current_way].second_chance_bit = 0;
            d_cache_fifo_ptr[set_index] = (current_way + 1) % D_CACHE_WAYS;
        }
    }
}

int find_sca_victim_i_cache(int set_index) {
    while (1) {
        int current_way = i_cache_fifo_ptr[set_index];
        if (i_cache[set_index][current_way].second_chance_bit == 0) {
            i_cache_fifo_ptr[set_index] = (current_way + 1) % I_CACHE_WAYS;
            return current_way;
        }
        else {
            i_cache[set_index][current_way].second_chance_bit = 0;
            i_cache_fifo_ptr[set_index] = (current_way + 1) % I_CACHE_WAYS;
        }
    }
}

CacheResult access_d_cache(unsigned int address, unsigned int write_data, int write_enable) {
    CacheResult result = { .hit = 0, .data = 0 };

    int offset_bits = 0;
    if (CACHE_LINE_SIZE > 1) {
        int size = CACHE_LINE_SIZE;
        while (size >>= 1) offset_bits++;
    }
    unsigned int offset = address & ((1 << offset_bits) - 1);

#if (CURRENT_MAPPING == MAPPING_SET_ASSOCIATIVE || CURRENT_MAPPING == MAPPING_DIRECT_MAPPED)
    int index_bits = 0;
    if (D_CACHE_SETS > 1) {
        int sets = D_CACHE_SETS;
        while (sets >>= 1) index_bits++;
    }
    unsigned int index = (address >> offset_bits) & ((1 << index_bits) - 1);
    unsigned int tag = address >> (offset_bits + index_bits);

    CacheLine* target_set = d_cache[index];
    for (int i = 0; i < D_CACHE_WAYS; i++) {
        if (target_set[i].valid && target_set[i].tag == tag) {
            result.hit = 1;
            d_cache_hits++;
#if (CURRENT_REPLACEMENT_POLICY == POLICY_SCA)
            target_set[i].second_chance_bit = 1;
#elif (CURRENT_REPLACEMENT_POLICY == POLICY_LRU)
            update_lru(target_set, i, D_CACHE_WAYS);
#endif

            if (write_enable) {
                memcpy(&target_set[i].data[offset], &write_data, 4);
                target_set[i].dirty = 1;
            }
            else {
                memcpy(&result.data, &target_set[i].data[offset], 4);
            }
            return result;
        }
    }

#elif (CURRENT_MAPPING == MAPPING_FULLY_ASSOCIATIVE)
    unsigned int tag = address >> offset_bits;

    CacheLine* target_set = d_cache[0];
    for (int i = 0; i < D_CACHE_WAYS; i++) {
        if (target_set[i].valid && target_set[i].tag == tag) {
            result.hit = 1;
            d_cache_hits++;
#if (CURRENT_REPLACEMENT_POLICY == POLICY_SCA)
            target_set[i].second_chance_bit = 1;
#elif (CURRENT_REPLACEMENT_POLICY == POLICY_LRU)
            update_lru(target_set, i, D_CACHE_WAYS);
#endif

            if (write_enable) {
                memcpy(&target_set[i].data[offset], &write_data, 4);
                target_set[i].dirty = 1;
            }
            else {
                memcpy(&result.data, &target_set[i].data[offset], 4);
            }
            return result;
        }
    }
#endif

    result.hit = 0;
    d_cache_misses++;
    return result;
}

CacheResult access_i_cache(unsigned int address) {
    CacheResult result = { .hit = 0, .data = 0 };

    int offset_bits = 0;
    if (CACHE_LINE_SIZE > 1) {
        int size = CACHE_LINE_SIZE;
        while (size >>= 1) offset_bits++;
    }
    unsigned int offset = address & ((1 << offset_bits) - 1);

#if (CURRENT_MAPPING == MAPPING_SET_ASSOCIATIVE || CURRENT_MAPPING == MAPPING_DIRECT_MAPPED)
    int index_bits = 0;
    if (I_CACHE_SETS > 1) {
        int sets = I_CACHE_SETS;
        while (sets >>= 1) index_bits++;
    }
    unsigned int index = (address >> offset_bits) & ((1 << index_bits) - 1);
    unsigned int tag = address >> (offset_bits + index_bits);

    CacheLine* target_set = i_cache[index];
    for (int i = 0; i < I_CACHE_WAYS; i++) {
        if (target_set[i].valid && target_set[i].tag == tag) {
            result.hit = 1;
            i_cache_hits++;
#if (CURRENT_REPLACEMENT_POLICY == POLICY_SCA)
            target_set[i].second_chance_bit = 1;
#elif (CURRENT_REPLACEMENT_POLICY == POLICY_LRU)
            update_lru(target_set, i, I_CACHE_WAYS);
#endif
            memcpy(&result.data, &target_set[i].data[offset], 4);
            return result;
        }
    }
#elif (CURRENT_MAPPING == MAPPING_FULLY_ASSOCIATIVE)
    unsigned int tag = address >> offset_bits;

    CacheLine* target_set = i_cache[0];
    for (int i = 0; i < I_CACHE_WAYS; i++) {
        if (target_set[i].valid && target_set[i].tag == tag) {
            result.hit = 1;
            i_cache_hits++;
#if (CURRENT_REPLACEMENT_POLICY == POLICY_SCA)
            target_set[i].second_chance_bit = 1;
#elif (CURRENT_REPLACEMENT_POLICY == POLICY_LRU)
            update_lru(target_set, i, I_CACHE_WAYS);
#endif
            memcpy(&result.data, &target_set[i].data[offset], 4);
            return result;
        }
    }
#endif

    result.hit = 0;
    i_cache_misses++;
    return result;
}