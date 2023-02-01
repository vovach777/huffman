# huffman
Huffman coding in C

```
Copyright (c) 2013 the authors listed at the following URL, and/or
the authors of referenced articles or incorporated external code:
http://en.literateprograms.org/Huffman_coding_(C_Plus_Plus)?action=history&offset=20090129100015

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from: http://en.literateprograms.org/Huffman_coding_(C_Plus_Plus)?oldid=16057
```

```
static int dataset[] = {
 108,111,108,110,105,107,109,109,108,106,106,109,109,104,107,107,
 107,109,105,107,108,104,107,107,106,104, 99,106,107,105,108,107,
 103,103,109,101,111,118,137,126,141,137,140,141,140,133,121,139,
 131,116,105,110,103,105,105,105,110,108,109,109,109,110,108,102,
 125,137,139,132,138,141,141,141,138,131,117,125,132,140,138,135,
 120,120,116,112,117,110,113,116,114,113,110,112,115,117,118,114,
 113,113,109,111,112,109,109,113,110,109,108,106,107,108,112,109,
 112,112,114,113,114,114,123,118,136,140,141,141,138,127,119,135,
 118,116,114,116,113,112,112,112,111,110,109,109,109,112,113,110,
 107,108,107,106,106,106,108,107,108,109,108,108,108,108,108,108,
 108,108,106,107,106,106,105,106,106,106,108,108,108,107,108,108,
 108,108,109,108,109,109,110,110,111,112,112,111,109,109,107,107,
 111,108,111,111,112,117,121,114,117,132,141,127,113,109,110,112,
 111,126,136,118,132,142,138,138,140,142,136,137,138,142,139,140,
 139,138,130,139,128,111, 96,112,101,102,109,104,101,112,127,113,
 130,137,137,135,136,140,140,139,140,133,127,135,137,138,136,137
 };
```

```
sampleHuffman.exe
size: 256, symbols: 40
  96: 01111110
  99: 01111111
 101: 001010
 102: 0110100
 103: 001011
 104: 011110
 105: 10100
 106: 0011
 107: 1011
 108: 010
 109: 000
 110: 11111
 111: 11110
 112: 1000
 113: 11101
 114: 01100
 115: 01110001
 116: 111001
 117: 110001
 118: 110110
 119: 01110000
 120: 1001100
 121: 1001101
 123: 01110011
 125: 0111110
 126: 1101110
 127: 011101
 128: 01110010
 130: 1001111
 131: 1001110
 132: 110000
 133: 0110101
 135: 011011
 136: 111000
 137: 10010
 138: 11001
 139: 00100
 140: 11010
 141: 10101
 142: 1101111

tree size: 79 bit
compressed: 2048 -> 1850 bit
\-*
  |-*
  | |-*
  | | |-109
  | | \-*
  | |   |-*
  | |   | |-139
  | |   | \-*
  | |   |   |-101
  | |   |   \-103
  | |   \-106
  | \-*
  |   |-108
  |   \-*
  |     |-*
  |     | |-114
  |     | \-*
  |     |   |-*
  |     |   | |-102
  |     |   | \-133
  |     |   \-135
  |     \-*
  |       |-*
  |       | |-*
  |       | | |-*
  |       | | | |-119
  |       | | | \-115
  |       | | \-*
  |       | |   |-128
  |       | |   \-123
  |       | \-127
  |       \-*
  |         |-104
  |         \-*
  |           |-125
  |           \-*
  |             |-96
  |             \-99
  \-*
    |-*
    | |-*
    | | |-112
    | | \-*
    | |   |-137
    | |   \-*
    | |     |-*
    | |     | |-120
    | |     | \-121
    | |     \-*
    | |       |-131
    | |       \-130
    | \-*
    |   |-*
    |   | |-105
    |   | \-141
    |   \-107
    \-*
      |-*
      | |-*
      | | |-*
      | | | |-132
      | | | \-117
      | | \-138
      | \-*
      |   |-140
      |   \-*
      |     |-118
      |     \-*
      |       |-126
      |       \-142
      \-*
        |-*
        | |-*
        | | |-136
        | | \-116
        | \-113
        \-*
          |-111
          \-110
```
