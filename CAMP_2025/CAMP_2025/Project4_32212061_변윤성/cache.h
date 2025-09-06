#ifndef CACHE_H
#define CACHE_H

// ��ü ��å ���� ��ũ��
#define POLICY_LRU    0
#define POLICY_RANDOM 1
#define POLICY_FIFO   2
#define POLICY_SCA    3

#define CURRENT_REPLACEMENT_POLICY POLICY_LRU

// ĳ�� ����(Mapping) ���� ��ũ��
#define MAPPING_SET_ASSOCIATIVE   0
#define MAPPING_DIRECT_MAPPED     1
#define MAPPING_FULLY_ASSOCIATIVE 2

#define CURRENT_MAPPING MAPPING_SET_ASSOCIATIVE

// ĳ�� �⺻ �Ķ����
#define CACHE_LINE_SIZE 64
#define I_CACHE_SIZE (1024 * 4)
#define D_CACHE_SIZE (1024 * 4)


// ���õ� ĳ�� ������ ���� WAYS�� SETS�� �ڵ����� ����
#if (CURRENT_MAPPING == MAPPING_SET_ASSOCIATIVE)
#define I_CACHE_WAYS 4
#define D_CACHE_WAYS 4
#elif (CURRENT_MAPPING == MAPPING_DIRECT_MAPPED)
#define I_CACHE_WAYS 1
#define D_CACHE_WAYS 1
#elif (CURRENT_MAPPING == MAPPING_FULLY_ASSOCIATIVE)
#define I_CACHE_WAYS (I_CACHE_SIZE / CACHE_LINE_SIZE)
#define D_CACHE_WAYS (D_CACHE_SIZE / CACHE_LINE_SIZE)
#endif

#define I_CACHE_SETS (I_CACHE_SIZE / (CACHE_LINE_SIZE * I_CACHE_WAYS))
#define D_CACHE_SETS (D_CACHE_SIZE / (CACHE_LINE_SIZE * D_CACHE_WAYS))


// �ڷᱸ�� ����
typedef struct {
    int hit;
    unsigned int data;
} CacheResult;

typedef struct {
    int valid;
    int dirty;
    unsigned int tag;
    int lru_counter;
    int second_chance_bit;
    unsigned char data[CACHE_LINE_SIZE];
} CacheLine;


// �Լ� ������Ÿ��
void init_cache();

// ���� �Լ�
CacheResult access_d_cache(unsigned int address, unsigned int write_data, int write_enable);
CacheResult access_i_cache(unsigned int address);

// ��ü ��å �Լ�
void update_lru(CacheLine* cache_set, int accessed_way, int ways);
int find_lru_victim(CacheLine* cache_set, int ways);
int find_random_victim();
int find_fifo_victim_d_cache(int set_index);
int find_fifo_victim_i_cache(int set_index);
int find_sca_victim_d_cache(int set_index);
int find_sca_victim_i_cache(int set_index);


#endif