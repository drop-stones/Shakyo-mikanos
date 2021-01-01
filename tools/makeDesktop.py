#!/usr/bin/python3

import argparse
import cv2
import numpy as np

def compile(bgr_array) -> bytes:
    result = []
    print('Height:\t{0}\nWidth:\t{1}\nColor:\t{2}'.format(bgr_array.shape[0], bgr_array.shape[1], bgr_array.shape[2]))
    for shape_info in bgr_array.shape:
        result.append(shape_info.to_bytes(4, byteorder='little'))

    for column in bgr_array:
        for dot in column:
            for color in dot:
                c = color.item()
                result.append(c.to_bytes(1, byteorder='little'))
    
    return b''.join(result)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('img', help='path to image file')
    parser.add_argument('-o', help='path to an output file', default='img.out')
    args = parser.parse_args()

    with open(args.o, 'wb') as out:
        bgr_array = cv2.imread(args.img)
        #n = 200
        #bgr = bgr_array[n][n]
        #print('[{0},{0}]: b = {1}, g = {2}, r = {3}'.format(n, bgr[0], bgr[1], bgr[2]))
        #return
        out.write(compile(bgr_array))

if __name__ == '__main__':
    main()