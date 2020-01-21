#!/usr/bin/env python3
import tkinter as tk
import tkinter.filedialog
from  PIL import ImageTk, Image
from res.tools.my_image_sprite import myImageSprite, SpriteSheet

def donothing():
    filewin = tk.Toplevel(root)
    button = tk.Button(filewin, text="Do nothing button")
    button.pack()

def open_picture():
    file_list.append(tk.filedialog.askopenfilename(initialdir="./res/textures/sprites", title="Select Sprite",filetypes=(("png files", "*.png"), ("all files", "*.*"))))
    added_image()

def resize(event):
    root.image = root.image_copy.resize((event.width, event.height), Image.ANTIALIAS)
    root.background_image = ImageTk.PhotoImage(root.image)
    root.the_sprites.configure(image=root.background_image)

def added_image():
    my_sprites.append(myImageSprite(x=200,p=PADDING,file_name=file_list[-1]))
    SpriteSheet.gen_sprite_sheet(my_sprites, output_file, 200, 5)
    root.image = Image.open(output_file)
    root.image_copy = root.image.copy()

PADDING  = 4
file_list = []
my_sprites = []
output_file = './res/textures/voxel.png'
temporary_directory = './res/textures/spritetmp/'
root = tk.Tk()
menubar = tk.Menu(root)
filemenu = tk.Menu(menubar, tearoff=0)
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
helpmenu = tk.Menu(menubar, tearoff=0)
helpmenu.add_command(label="Help Index", command=donothing)
helpmenu.add_command(label="About...", command=donothing)
menubar.add_cascade(label="Help", menu=helpmenu)

#canvas = tkinter.Canvas(root,width=400,height=400)
root.image = Image.open(output_file)
root.image_copy  = root.image.copy()
root.background_image = ImageTk.PhotoImage(root.image)
root.the_sprites = tk.Label(root,image=root.background_image)


#canvas.image = canvas.create_image(0, 0, anchor = tkinter.NW, image = filename)
root.the_sprites.pack(fill=tk.BOTH, expand=tk.YES)
root.the_sprites.bind("<Configure>",resize)
root.config(menu=menubar)
root.mainloop()