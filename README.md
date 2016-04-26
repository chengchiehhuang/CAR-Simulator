## Introduction

This simulator is modified from Christopher Lord's MIPS-CPU-Simulator project for 
the computer architecture coursework at the University of Edinburgh. The target of 
the coursework is to let students implement classic architecture ideas (branch 
predictors and hardware data prefetchers) and understand their impacts on the 
overall system performance. 

## Usage
The assembler `rasm` responds to the following command switches:

* `-f sourcefile` – assemble ASCII assembly code from file (required)
* `-t textstreamfile` – override default destination and output binary `.text` to specified file
* `-d datastreamfile` – override default destination and output binary `.data` to specified file

The simulator `rsim` responds to the following command switches:

* `-t textstreamfile` – load memory segment .text with the contents of a binary file (required)
* `-d datastreamfile` – load memory segment .data with contents of a binary file
* `-v` – very verbose CPU. Will echo every instruction, and the associated program counter.
* `-b` – select branch predictor (0: Always Not Taken, 1: Always Taken 2: 2-Bit 3: TwoLevel 3: GShare )
* `-s` – select cache size (0: no cache, 1: 1KB, 2:2KB, 4:4KB) 
* `-x` – select prefetchers (0: no prefetcher, 1: next-line, 2: stride)


# License

The MIT License

Copyright (c) 2006 Christopher Lord

Copyright (c) 2016 Cheng-Chieh Huang

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
