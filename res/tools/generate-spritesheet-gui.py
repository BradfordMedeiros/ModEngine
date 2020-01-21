#!/usr/bin/env python3
import tkinter
import tkinter.filedialog
from  PIL import ImageTk, Image
from my_image_sprite import myImageSprite
import cv2
import os
import shutil
import math
import glob
import numpy as np

def gen_sprite_sheet(input,output,padding):
    image_list = sorted(glob.glob(input + '/*.png'))

    images = []

    max_width = 0
    max_height = 0
    if(len(image_list)>1):
        n_col = int(math.sqrt(len(image_list) - 1)) + 1
    else:
        n_col = 1
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

def donothing():
    filewin = tkinter.Toplevel(root)
    button = tkinter.Button(filewin, text="Do nothing button")
    button.pack()

def open_picture():
    file_list.append(tkinter.filedialog.askopenfilename(initialdir="./res/textures/sprites", title="Select Sprite",filetypes=(("png files", "*.png"), ("all files", "*.*"))))
    added_image()

def resize(event):
    root.image = root.image_copy.resize((event.width, event.height), Image.ANTIALIAS)
    root.background_image = ImageTk.PhotoImage(root.image)
    root.the_sprites.configure(image=root.background_image)

def added_image():
    try:
        shutil.rmtree(temporary_directory)
    except:

        print("Directory ", temporary_directory, " Created ")
    try:
        # Create target Directory
        os.mkdir(temporary_directory)
        print("Directory ", temporary_directory, " Created ")
    except FileExistsError:
        print("Directory ", temporary_directory, " already exists")
    quant = len(file_list)
    for i in range(quant):
        name = file_list[i]
        save_name = temporary_directory+output_name(i,quant)+".png"
        print(save_name)
        local_image = cv2.imread(name)
        cv2.imwrite(save_name, local_image)
    if(quant>1):
        gen_sprite_sheet(temporary_directory,output_file,PADDING)
        root.image = Image.open(output_file)
        root.image_copy = root.image.copy()




def output_name(number, quant):
    digits = math.ceil(math.log(quant,5))+1
    digital_list = 'abcde'
    name = ''
    for i in range(digits):
        name+=(digital_list[((number)//(5**(digits-i-1)))%5])
    return name


PADDING  = 4
file_list = []
output_file = './res/textures/voxel.png'
temporary_directory = './res/textures/spritetmp/'
root = tkinter.Tk()
menubar = tkinter.Menu(root)
filemenu = tkinter.Menu(menubar, tearoff=0)
filemenu.add_command(label="New", command=donothing)
filemenu.add_command(label="Open", command=open_picture)
filemenu.add_command(label="Save", command=donothing)
filemenu.add_command(label="Save as...", command=donothing)
filemenu.add_command(label="Close", command=donothing)

filemenu.add_separator()

filemenu.add_command(label="Exit", command=root.quit)
menubar.add_cascade(label="File", menu=filemenu)
editmenu = tkinter.Menu(menubar, tearoff=0)
editmenu.add_command(label="Undo", command=donothing)

editmenu.add_separator()

editmenu.add_command(label="Cut", command=donothing)
editmenu.add_command(label="Copy", command=donothing)
editmenu.add_command(label="Paste", command=donothing)
editmenu.add_command(label="Delete", command=donothing)
editmenu.add_command(label="Select All", command=donothing)

menubar.add_cascade(label="Edit", menu=editmenu)
helpmenu = tkinter.Menu(menubar, tearoff=0)
helpmenu.add_command(label="Help Index", command=donothing)
helpmenu.add_command(label="About...", command=donothing)
menubar.add_cascade(label="Help", menu=helpmenu)

#canvas = tkinter.Canvas(root,width=400,height=400)
root.image = Image.open(output_file)
root.image_copy  = root.image.copy()
root.background_image = ImageTk.PhotoImage(root.image)
root.the_sprites = tkinter.Label(root,image=root.background_image)


#canvas.image = canvas.create_image(0, 0, anchor = tkinter.NW, image = filename)
root.the_sprites.pack(fill=tkinter.BOTH, expand=tkinter.YES)
root.the_sprites.bind("<Configure>",resize)
root.config(menu=menubar)
root.mainloop()