#!/usr/bin/env python3
import argparse
import glob
import cv2
import numpy as np
import math

def gen_sprite_sheet(input,output,padding):
    image_list = sorted(glob.glob(input + '/*.png'))

    images = []

    max_width = 0
    max_height = 0

    n_col = int(math.sqrt(len(image_list) - 1)) + 1
    n_row = -((-(1 + len(image_list))) // n_col)  # dark math magic

    for name in image_list:
        # open all images and find their sizes
        local_image = cv2.imread(name)
        if local_image.shape[2] == 3:
            b_channel, g_channel, r_channel = cv2.split(local_image)
            alpha_channel = np.ones(b_channel.shape,
                                    dtype=b_channel.dtype) * 255  # creating a dummy alpha channel image.
            local_image = cv2.merge((b_channel, g_channel, r_channel, alpha_channel))

        images.append(local_image)
        if images[-1].shape[1] > max_width:
            max_width = images[-1].shape[1]
        if images[-1].shape[0] > max_height:
            max_height = images[-1].shape[0]

    total_width = max_width * n_col + (n_col - 1) * padding
    total_height = max_height * n_row + (n_row - 1) * padding
    total_height = max(total_height, total_width)
    total_width = max(total_height, total_width)

    # create a new array with a size large enough to contain all the images
    final_image = np.zeros((total_height, total_width, 4), dtype=np.uint8)

    current_x = 0
    current_y = 0
    current_c = 0

    for image in images:
        # add an image to the final array and increment the y coordinate
        final_image[current_y:(image.shape[0] + current_y), current_x:(image.shape[1] + current_x), :] = image
        current_c += 1
        current_x += max_width + padding
        if current_c >= n_col:
            current_c = 0
            current_x = 0
            current_y += max_height + padding

    cv2.imwrite(output, final_image)

parser = argparse.ArgumentParser()
parser.add_argument('--output', '-o', type=str, default='./res/textures/voxel.png',
                    help='Output file to write combined spritesheet')
parser.add_argument('--input', '-i', type=str, default='./res/textures/sprites',
                    help='Directory to read textures from (reads sorted order based on name and adds to sheet)')
parser.add_argument('--padding', '-p', type=int, default=4,
                    help='Number of pixels to add as padding around each texture')
#parser.add_argument('--n_columns', '-c', type=int, default=4, help='Number of columns to use')

args = parser.parse_args()
gen_sprite_sheet(args.input,args.output,args.padding)
