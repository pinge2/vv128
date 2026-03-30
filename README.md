# vv128 Crypto Hash function

`vv128` is a 256-bit hash function on C++ based on Merkle-Damgor construction. It uses custom non-linear NADD optrations.
`vv128` is open to analysis, hacking and playing.

DO NOT USE THIS HASH ALGORITHM IN REAL CASES VIA IT'S POSSIBLE UNSECURE

## 1. Algorithm
`vv128` extends message into 256-bit. It adds padding bit (+ bit 1 and zeros) and message length in bits at the end.

This is my own custon cryptoprimitives designed as a non-linear and fast functions. The basic operations are:
- `W(a,b)` function: `W(a,b) = ~(a & b) + (b ^ (a <<< 19))`
- `Ql(a,b)` function: `Ql(a,b) = ~((a <<< 17) & (b <<< 23)) + ~(a & b)`
- `Nux(s,a,b)` function: `Nux(s,a,b) = ~(a & ~s) + ~(b & s)`
- `A(a)` function: `A(a) = a + (a <<< 11) + (a <<< 23) + (a <<< 41)`

This operations performs non-linearity transformations

`vv128` works with 256-bit blocks saved as 4 uint64 (block\[0]-block\[3]), 10 rounds each. Every round `vv128` uses 2 round constants that is generated with my PRNG generator. In it's state `vv128` has eight 64-bit variables (a,b,c,d,e,f,g,h). At the start they are equal to:
```c
a = 0x6a09e667bb67ae85 ^ salt;  // π
b = 0x3c6ef372fe94f82b ^ salt;  // e
c = 0x9e3779b97f4a7c15 ^ salt;  // φ
d = 0x428a2f98d728ae22 ^ salt;  // sqrt(2)
e = 0x9ddfea08eb382d69 ^ salt;
f = 0xfea7b9b7745c4422 ^ salt;
g = 0x2bad44f21bdb6361 ^ salt;
h = 0x4c9b74c7693e68e3 ^ salt;
```
Where salt is usually zero.

for every round:
```c
for (int i of 12){
  word1 = Nux(RC[i], block0, block1) <<< i
  word2 = Nux(RC[~i], block2, block3) <<< ~i

  a = W(word1, Ql(a, f)) + word2
  b = Ql(word1, W(b, g)) + word2
  c = W(word1, Ql(c, h)) + word2
  d = Ql(word1, W(d, e)) + word2
  e = W(word2, Ql(e, b)) + word1
  f = Ql(word2, W(f, c)) + word1
  g = W(word2, Ql(g, d)) + word1
  h = Ql(word2, W(h, a)) + word1
}
```

And, the finalizer that constructs 256-bit hash from 512-bit inner state and message length:
```c
ctrl = A(Ql(A(len), 0x9e3779b97f4a7c15))
t0 = Nux(ctrl, e, f)
t1 = Nux(ctrl, f, g)
t2 = Nux(ctrl, g, h)
t3 = Nux(ctrl, h, e)

hash[0] = a ^ W(t0, t2)
hash[1] = b ^ W(t1, t3)
hash[2] = c ^ W(t2, t0)
hash[3] = d ^ W(t3, t1)
```

## 2. API
`vv128`can be used in many variations. With salt it can be used as key generator, or as streaming hash function.

Base structure:
- `vv_state`: a container of current `vv128` state and pointer to block data

Main functions:

### `void vv_print_state(const vv_state*, bool)`
Prints current 256-bit block and 512-bit state variables. Also prints round constants if bool parameter `print_consts` is true
### `int vv_init_state(vv_state*, uint64_t)`
Initializes `vv_state` with round constants and base state values. Returns 0 on success and -1 on error. `salt` parameter is used to add a salt into initialized structure; keep it zero if you don't use.
### `int vv_extend(vv_state*, const char*, size_t)`
Extends message to 256 bit with padding. Creates new buffer and copies message into it with padding. Returns 0 on success and -1 on error. It calls `strlen()` function if you didn't provide `len` parameter.
### `int vv_free_state(vv_state*)`
Deletes inner buffer and clears states and constants. Returns 0 on success and -1 on error.
### `int vv_core(vv_state*, int)`
Performs one round with 256-bit block. Parameter `i` states for number of round. Returns 0 on success and -1 on error.
### `int vv_finalize(const vv_state*, uint64_t*)`
Performs finalization and stores ready 256-bit hash into a buffer in `res` parameter. Returns 0 on success and -1 on error.
## `int vv128(vv_state*, uint64_t*)`
Calculates full `vv128` hash algorithm and stores ready 256-bit hash into a buffer in `res` parameter. Returns 0 on success and -1 on error. Needs `vv_extend()` and `vv_init_state()` before using.

## 3. Usage
Example of usage:
```c
#include "vv128.hpp"

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
    print("Hamming distance: popcnt(hash1 ^ hash2) = " << dist << " (ideal is 128)");

    vv_free_state(&state1);
    vv_free_state(&state2);
}
```
Result:
```
Original: 911c9bab89442dd9f80c33a36f52e7017a0c38ddd73996c79a7793608cf8522
1 bit inverted: 1482157929b37153d3f3f1df16bc809b9b9f737144d49bf5e9aaa46345c956a4
Hamming distance: popcnt(hash1 ^ hash2) = 134 (ideal is 128)
```

Try to break it! And of course, DO NOT USE IT IN REAL CASES AS A SECURE HASH ALGORITHM
;)
