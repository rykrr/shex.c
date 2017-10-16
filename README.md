# shex.c
A Simple Ncurses Hex Editor

![screenshot](https://github.com/rykrr/shex.c/blob/master/screenshot.png?raw=true)

## Usage
shexc (file)

| Key       | Action                                    |
|-----------|-------------------------------------------|
|  h,j,k,l  | Cursor Left, Down, Up, Right              |
|  0-9,A-F  | HEX Values                                |
|  n        | New next node                             |
|  i        | Insert prev node                          |
|  d        | Delete current node                       |
|  s        | Save                                      |
|  q        | Quit                                      |

### Compile
gcc -lncurses shex.c -o shexc

#### Copyright
Copyright (c) 2017 Ryan Kerr
