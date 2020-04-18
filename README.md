# Turing Machine
>Simulator of a Nondeterministic Turing machine (Single Tape). 

## Requirements
The program must be implemented in `C99` and using only the standard library (`libc`).
The program has to be developed considering time and memory efficiency and using data structures seen in the Algorithms and Principles of Computer Science course.

Other requirements and assumptions can be found [here].

## Input Example
```bash
tr
0 a a S 1
0 a a S 13
1 x x R 1
1 a x R 2
2 a a R 3
2 b b R 8
3 a a R 3
3 b b R 4
4 b b R 4
4 c c R 5
5 y y R 5
5 c c R 5
5 x x R 5
5 _ x L 6
6 y y L 6
6 x x L 6
6 c y L 7
7 c c L 7
7 y y L 7
7 b b L 7
7 x x L 7
7 a a L 7
7 _ x R 1
8 b b R 8
8 c y R 9
9 y y R 10
10 y y R 10
10 x x S 11
9 c c R 12
13 a x R 14
13 x x R 13
13 y y R 20
13 b b R 21
14 a a R 14
14 b b R 15
15 b b R 15
15 c c R 16
16 c c R 16
16 x x R 16
16 _ x L 17
17 c c L 17
17 x x L 17
17 y y L 18
18 y y L 18
18 b y L 19
19 b b L 19
19 a a L 19
19 x x L 19
19 _ x R 13
20 y y R 20
20 c c R 20
20 x x S 22
15 y y R 15
17 b y L 19
acc
11
22
max
4000
run
aabbcc
aabbccc
aaabbc
aabbbbcc
aabcc
aabbcccccc
aaaaaabbccccc
aaaaabbbbbc
aaabbbccccccccc
abbbccccc
aaabbbbcc
```

## Output Expected From The Previous Example
```bash
1
1
0
1
1
1
0
1
1
0
0
```


## Authors
* [Pietro Lentini]

## License
This project is distributed under the terms of the Apache License v2.0.
See file [LICENSE] for further information.

[here]: <https://github.com/lentinip/Turing-Machine/blob/master/Specifiche%20e%20Assunzioni.pdf>
[Pietro Lentini]: <https://github.com/lentinip>
[LICENSE]: <https://github.com/lentinip/Turing-Machine/blob/master/LICENSE>
