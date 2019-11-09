# Ninth Ring

Example input:
```
nand %alpha 0x2d
reset %beta; abc
 ;  test;a
```
bytecode output:
```
05     03    00 00 00 2d
inst  reg      imm
nand alpha    0x0000002d
07     04
inst  reg
reset beta
```