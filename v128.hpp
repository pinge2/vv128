/*
vv128 Hash Algorithm
Copyright 2026 EtoPinge
SPDX-License-Identifier: Apache-2.0

DO NOT USE IN REAL CASES VIA IT'S POSSIBLE INSECURE
*/

#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <cstring>
#include <ctime>

#define PHI 0x9e3779b97f4a7c15
#define Q 0x9ddfea08eb382d69

#ifndef NO_PRINT_MACRO
    #define print(x) std::cout << x << std::endl
#else
    #define print(x)
#endif

#define hex(x) std::hex << x << std::dec

#ifdef DEBUG
    #define debug(x) std::cout << x << std::endl;
#else
    #define debug(x)
#endif

#ifndef NO_ERRORS
    #define error(x) std::cout << "[ERROR " << __FILE__ << ':' << __LINE__ << "] " << x << std::endl;
#else
    #define error(x)
#endif


static uint64_t __last = 0xFFFFFFFFFFFFFFFF;


uint64_t time_ns(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}



uint64_t rol(uint64_t a, uint64_t b){
    return (a << b) | (a >> (64 - b));
}


uint64_t mux(uint64_t s, uint64_t a, uint64_t b){
    return (a & ~s) | (b & s);
}


uint64_t A(uint64_t a){
    uint64_t a0 = rol(a, 11);
    uint64_t a1 = rol(a, 23);
    uint64_t a2 = rol(a, 41);
    return a + a0 + a1 + a2;
}


// By EtoPinge
uint64_t hrand(){
    uint64_t result = __last;
    __last = A(__last);
    return result;
}


uint64_t hrand2(){
    uint64_t l1 = (rol(~hrand(), 17) + 0x9ddfea08eb382d69) * 0x9e3779b97f4a7c15;
    uint64_t l2 = __last;
    return ~(l1 & l2) + (l1 ^ rol(l2, 29));
}


void shrand(uint64_t val){
    __last = 0x9e3779b97f4a7c15 * (val + 0x9ddfea08eb382d69);
    hrand();
}


uint64_t hgetstate(){
    return __last;
}


void hsetstate(uint64_t tmp){
    __last = tmp;
}


struct vv_state{
    uint64_t block[4];
    uint64_t a, b, c, d, e, f, g, h;
    uint64_t round_consts[16];
    uint64_t* ext_buffer;
    uint32_t buffer_length;
};


uint64_t mux4(uint64_t s1, uint64_t s2, uint64_t a, uint64_t b, uint64_t c, uint64_t d){
    return (a & ~s1 & ~s2) |
           (b & ~s1 & s2) |
           (c & s1 & ~s2) |
           (d & s1 & s2);
}



uint64_t W(uint64_t a, uint64_t b){
    return ~(a & b) + (b ^ rol(a, 19));
}


uint64_t Ql(uint64_t a, uint64_t b){
    return ~(rol(a, 17) & rol(b, 23)) + ~(a & b);
}


uint64_t Nux(uint64_t s, uint64_t a, uint64_t b){
    return ~(a & ~s) + ~(b & s);
}



int vv_init_state(vv_state* state, uint64_t salt = 0){
    if (!state){
        error("VV: Null poiner state input!");
        return -1;
    }

    state->a = 0x6a09e667bb67ae85 ^ salt;  // π
    state->b = 0x3c6ef372fe94f82b ^ salt;  // e
    state->c = 0x9e3779b97f4a7c15 ^ salt;  // φ
    state->d = 0x428a2f98d728ae22 ^ salt;  // sqrt(2)
    state->e = 0x9ddfea08eb382d69 ^ salt;
    state->f = 0xfea7b9b7745c4422 ^ salt;
    state->g = 0x2bad44f21bdb6361 ^ salt;
    state->h = 0x4c9b74c7693e68e3 ^ salt;

    uint64_t temp = hgetstate();
    shrand(salt);

    for (int i = 0; i < 16; ++i){
        state->round_consts[i] = hrand2();
    }

    hsetstate(temp);

    return 0;
}



int vv_core(vv_state* state, int i){
    if (!state){
        error("VV: Null pointer state input!");
        return -1;
    }

    uint64_t word1 = rol(Nux(state->round_consts[i & 15], state->block[0], state->block[1]), i & 63);
    uint64_t word2 = rol(Nux(state->round_consts[~i & 15], state->block[2], state->block[3]), ~i & 63);
    
    state->a = W(word1, Ql(state->a, state->f)) + word2;
    state->b = Ql(word1, W(state->b, state->g)) + word2;
    state->c = W(word1, Ql(state->c, state->h)) + word2;
    state->d = Ql(word1, W(state->d, state->e)) + word2;
    state->e = W(word2, Ql(state->e, state->b)) + word1;
    state->f = Ql(word2, W(state->f, state->c)) + word1;
    state->g = W(word2, Ql(state->g, state->d)) + word1;
    state->h = Ql(word2, W(state->h, state->a)) + word1;

    return 0;
}



void vv_print_state(const vv_state* state, bool print_consts = false){
    print("Word 1: " << hex(state->block[0]));
    print("Word 2: " << hex(state->block[1]));
    print("Word 3: " << hex(state->block[2]));
    print("Word 4: " << hex(state->block[3]));
    print("\ta = " << hex(state->a));
    print("\tb = " << hex(state->b));
    print("\tc = " << hex(state->c));
    print("\td = " << hex(state->d));
    print("\te = " << hex(state->e));
    print("\tf = " << hex(state->f));
    print("\tg = " << hex(state->g));
    print("\th = " << hex(state->h));

    if (print_consts){
        print("\nRound constants:");
        for (int i = 0; i < 16; ++i) print("\tConst " << i << " = " << hex(state->round_consts[i]));
    }
}



int vv_extend(vv_state* state, const char* str, size_t len = 0){
    if (len == 0) len = strlen(str);

    if (!len || !state || !str){
        error("VV: Null pointer inputs!");
        return -1;
    }

    uint64_t bit_len = len * 8;
    uint64_t padded_len = len + 1 + 8;
    
    uint64_t rem = padded_len & 31;
    if (rem != 0) padded_len += (32 - rem);
    
    uint64_t val = padded_len >> 5;
    
    state->ext_buffer = (uint64_t*)malloc(32 * val);
    memset(state->ext_buffer, 0, 32 * val);
    memcpy(state->ext_buffer, str, len);
    
    ((uint8_t*)state->ext_buffer)[len] = 0x80;
    
    uint64_t* len_ptr = (uint64_t*)((uint8_t*)state->ext_buffer + padded_len - 8);
    *len_ptr = bit_len;

    state->buffer_length = val;

    return 0;
}



int vv_finalize(const vv_state* state, uint64_t* res){
    if (!state || !res){
        error("VV: Null pointer inputs!");
        return -1;
    }

    uint64_t ctrl = A(Ql(A(~state->buffer_length), 0x9e3779b97f4a7c15));
    
    uint64_t t0 = Nux(ctrl, state->e, state->f);
    uint64_t t1 = Nux(ctrl, state->f, state->g);
    uint64_t t2 = Nux(ctrl, state->g, state->h);
    uint64_t t3 = Nux(ctrl, state->h, state->e);

    res[0] = state->a ^ W(t0, t2);
    res[1] = state->b ^ W(t1, t3);
    res[2] = state->c ^ W(t2, t0);
    res[3] = state->d ^ W(t3, t1);

    return 0;
}



int vv_free_state(vv_state* state){
    if (!state){
        error("VV: Null pointer inputs!");
        return -1;
    }

    free(state->ext_buffer);

    memset(state->round_consts, 0, 16 * 8);

    state->block[0] = 0;
    state->block[1] = 0;
    state->block[2] = 0;
    state->block[3] = 0;

    state->a = 0;
    state->b = 0;
    state->c = 0;
    state->d = 0;
    state->e = 0;
    state->f = 0;
    state->g = 0;
    state->h = 0;

    return 0;
}



int vv128(vv_state* state, uint64_t* res){
    if (!state || !res){
        error("VV: Null pointer inputs!");
        return -1;
    }

    if (!state->buffer_length){
        error("VV: Zero-length extended message!");
        return -1;
    }

    for (int j = 0; j < state->buffer_length * 4; j += 4){
        state->block[0] = state->ext_buffer[j];
        state->block[1] = state->ext_buffer[j + 1];
        state->block[2] = state->ext_buffer[j + 2];
        state->block[3] = state->ext_buffer[j + 3];

        for (int i = 0; i < 12; ++i) vv_core(state, i);
    }

    vv_finalize(state, res);

    return 0;
}


#ifdef EXAMPLE
    int main(){
        vv_state state1, state2;
        uint64_t hash1[4], hash2[4];

        vv_init_state(&state1);
        vv_init_state(&state2);
        
        vv_extend(&state1, "Hello, thief! My name is Alex, and my password is 0xdeadbeef, but you cant see it cause it's my own hash VV128!");
        vv_extend(&state2, "Hello, thief! My name is alex, and my password is 0xdeadbeef, but you cant see it cause it's my own hash VV128!");
        
        vv128(&state1, hash1);
        vv128(&state2, hash2);

        print("Original: " << hex(hash1[0]) << hex(hash1[1]) << hex(hash1[2]) << hex(hash1[3]));
        print("1 bit inverted: " << hex(hash2[0]) << hex(hash2[1]) << hex(hash2[2]) << hex(hash2[3]));

        int dist = __builtin_popcountll(hash1[0] ^ hash2[0]) + __builtin_popcountll(hash1[1] ^ hash2[1]) + __builtin_popcountll(hash1[2] ^ hash2[2]) + __builtin_popcountll(hash1[3] ^ hash2[3]);
        print("Hamming distance: popcnt(hash1 ^ hash2) = " << dist);

        vv_free_state(&state1);
        vv_free_state(&state2);
    }
#endif
