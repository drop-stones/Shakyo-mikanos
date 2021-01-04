#!/usr/bin/python3

import argparse
import cv2
import numpy as np

def extract_from_img(bgr_array, height:int, width:int):
    img_height = bgr_array.shape[0]
    img_width  = bgr_array.shape[1]
    first_height = 0
    end_height = img_height
    first_width  = 0
    end_width = img_width
    if height < img_height:
        first_height = (img_height - height) // 2
        end_height = first_height + height
    if width < img_width:
        first_width  = (img_width - width) // 2
        end_width = first_width + width
    return bgr_array[first_height:end_height, first_width:end_width, :]

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
    parser.add_argument('-height', help='height of created img')
    parser.add_argument('-width', help='width of created img')
    args = parser.parse_args()

    with open(args.o, 'wb') as out:
        bgr_array = cv2.imread(args.img)
        resized_array = extract_from_img(bgr_array, int(args.height), int(args.width))
        out.write(compile(resized_array))

if __name__ == '__main__':
    main()