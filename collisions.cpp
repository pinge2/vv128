#include "vv128.hpp"
#define BLOCKS 1048576



int main(){
    shrand(time(NULL));

    vv_state state;
    uint64_t hash1[4], hash2[4];

    vv_init_state(&state);
    
    char* buff = (char*)malloc(BLOCKS * 32);

    for (int i = 0; i < BLOCKS * 4; ++i) ((uint64_t*)buff)[i] = hrand2() * (i + 1);

    state.buffer_length = BLOCKS;
    
    uint64_t start = time_ns();

    for (int j = 0; j < BLOCKS; ++j){
        memcpy(state.block, buff + j*32, 32);
        for (int i = 0; i < 10; ++i) vv_core(&state, i);
    }
    vv_finalize(&state, hash1);

    uint64_t elapsed = time_ns() - start;

    vv_init_state(&state);
    buff[hrand() & (BLOCKS * 32 - 1)] ^= 0x10;

    for (int j = 0; j < BLOCKS; ++j){
        memcpy(state.block, buff + j*32, 32);
        for (int i = 0; i < 10; ++i) vv_core(&state, i);
    }
    vv_finalize(&state, hash2);

    double gbs = (double)BLOCKS * 32.0 / (double)elapsed;

    print("Gb/s: " << gbs);

    print("Array size (in 256-bit blocks): " << BLOCKS);
    print("Original: " << hex(hash1[0]) << hex(hash1[1]) << hex(hash1[2]) << hex(hash1[3]));
    print("1 bit inverted: " << hex(hash2[0]) << hex(hash2[1]) << hex(hash2[2]) << hex(hash2[3]));

    int dist = __builtin_popcountll(hash1[0] ^ hash2[0]) + __builtin_popcountll(hash1[1] ^ hash2[1]) + __builtin_popcountll(hash1[2] ^ hash2[2]) + __builtin_popcountll(hash1[3] ^ hash2[3]);
    print("Hamming distance: popcnt(hash1 ^ hash2) = " << dist);

    vv_free_state(&state);
}