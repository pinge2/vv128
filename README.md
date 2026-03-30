# vv128 Crypto Hash function

`vv128` is a 256-bit hash function on C++ based on Merkle-Damgor construction. It uses custom non-linear NADD optrations.
`vv128` is open to analysis, hacking and playing.

DO NOT USE THIS HASH ALGORITHM IN REAL CASES VIA IT'S POSSIBLE UNSAFETY

## 1. Algorithm
`vv128` extends message into 256-bit. It adds padding bit (+ bit 1 and zeros) and message length in bits at the end.

This is my own custon cryptoprimitives designed as a non-linear and fast functions. The basic operations are:
- `W(a,b)` function: `W(a,b) = ~(a & b) + (b ^ (a <<< 19))`
- `Ql(a,b)` function: `Ql(a,b) = ~((a <<< 17) & (b <<< 23)) + ~(a & b)`
- `Nux(s,a,b)` function: `Nux(s,a,b) = ~(a & ~s) + ~(b & s)`
- `A(a)` function: `A(a) = a + (a <<< 11) + (a <<< 23) + (a <<< 41)`

This operations performs non-linearity transformations

`vv128` works with 256-bit blocks saved as 4 uint64 (block\[0]-block\[3]), 8 rounds each. Every round `vv128` uses 2 round constants that is generated with my PRNG generator. In it's state `vv128` has eight 64-bit variables (a,b,c,d,e,f,g,h). At the start they are equal to:
```
state->a = 0x6a09e667bb67ae85 ^ salt;  // π
state->b = 0x3c6ef372fe94f82b ^ salt;  // e
state->c = 0x9e3779b97f4a7c15 ^ salt;  // φ
state->d = 0x428a2f98d728ae22 ^ salt;  // sqrt(2)
state->e = 0x9ddfea08eb382d69 ^ salt;
state->f = 0xfea7b9b7745c4422 ^ salt;
state->g = 0x2bad44f21bdb6361 ^ salt;
state->h = 0x4c9b74c7693e68e3 ^ salt;
```
Where salt is usually zero.

for every round:
```c
for (int i of 8){
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
```
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
- `void vv_print_state(const vv_state*, bool)`:
Prints current 256-bit block and 512-bit state variables. Also prints round constants if bool parameter `print_consts` is true
