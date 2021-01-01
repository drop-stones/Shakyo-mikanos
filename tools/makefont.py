#!/usr/bin/python3

import argparse
import collections
import functools    # higher-order function
import re           # regular expression
import sys          

BITMAP_PATTERN = re.compile(r'([.*@]+)')

def compile(src: str) -> bytes:
    src = src.lstrip()  # delete white-space
    result = []

    for line in src.splitlines():
        m = BITMAP_PATTERN.match(line)
        if not m:
            continue
        # example                                                 ...@@...
        bits = [(0 if x == '.' else 1) for x in m.group(1)]     # [0,0,0,1,1,0,0,0]
        bits_int = functools.reduce(lambda a, b: 2*a + b, bits) # 00011000
        result.append(bits_int.to_bytes(1, byteorder='little'))
    
    return b''.join(result)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("font", help="path to a font file")
    parser.add_argument("-o", help="path to an output file", default="font.out")
    args = parser.parse_args()
    print('from \"{0}\" to \"{1}\"'.format(args.font, args.o))

    # with: short try-catch (close file propely)
    with open(args.o, 'wb') as out, open(args.font) as font:
        src = font.read()
        out.write(compile(src))

if __name__ == '__main__':
    main()