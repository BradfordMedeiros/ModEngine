#!/usr/bin/env python3
import argparse
import os

def generate_file_extension_list(num_chunks_width, num_chunks_height, num_chunks_depth):
  file_names = []
  
  offset_x = num_chunks_width // 2
  offset_y = num_chunks_height // 2
  offset_z = num_chunks_depth // 2
  
  for x in range(0, num_chunks_width):
    for y in range(0, num_chunks_height):
      for z in range(0, num_chunks_depth):
        file_extension = str(x - offset_x) + '.' + str(y - offset_y) + '.' + str(z - offset_z)
        file_names.append(file_extension)

  return file_names

def write_content(size, output_directory):
  for file_extension in generate_file_extension_list(size, size, size):
    with open(os.path.join(output_directory, file_extension), 'w') as filehandle:
      filehandle.write('\n')

parser = argparse.ArgumentParser()
parser.add_argument('--output', '-o', type=str, default='.', help='Directory to output generated chunk files')
parser.add_argument('--size', '-s', type=int, default=1, help='Number of chunks to make')
args = parser.parse_args()

write_content(args.size, args.output)