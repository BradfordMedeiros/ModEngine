#!/usr/bin/env python3
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--output', '-o', type=str, default='./res/textures/voxel.png', help='Output file to write combined spritesheet')
parser.add_argument('--input', '-i', type=str, default='./res/textures/sprites', help='Directory to read textures from (reads sorted order based on name and adds to sheet)')
parser.add_argument('--padding', '-p', type=int, default=4, help='Number of pixels to add as padding around each texture')

args = parser.parse_args()

