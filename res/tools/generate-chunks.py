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


def content_for_filename(filename, chunksize):
  values = list(map(lambda value: int(value), filename.split('.')))
  template_file = ""
  x_pos = chunksize * values[0]
  y_pos = chunksize * values[1] + chunksize / 2
  z_pos = chunksize * values[2] 
  template_file = template_file + "unitcube:position:" + str(x_pos) + " " + str(y_pos) + " " + str(z_pos) + "\n"
  template_file = template_file + "unitcube:scale:" + str(chunksize) + " 1.0 " + str(chunksize) + "\n"
  template_file = template_file + "unitcube:mesh:./res/models/unit_rect/unit_rect.obj" + "\n"
  return template_file


def write_content(num, size, output_directory):
  for file_extension in generate_file_extension_list(num, num, num):
    with open(os.path.join(output_directory, file_extension), 'w') as filehandle:
      filehandle.write(content_for_filename(file_extension, size))

parser = argparse.ArgumentParser()
parser.add_argument('--output', '-o', type=str, default='.', help='Directory to output generated chunk files')
parser.add_argument('--num', '-n', type=int, default=1, help='Number of chunks to make')
parser.add_argument('--size', '-s', type=int, default=100, help='Size of the chunk (assumes x,y,z values are equal)')
args = parser.parse_args()

write_content(args.num, args.size, args.output)

