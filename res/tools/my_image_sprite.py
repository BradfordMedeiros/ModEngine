import cv2
import numpy as np

class myImageSprite:
    # This is the internal class where I will store images for manipulation within my sprite gui code
    # we have the following
    # base_image,filtered_image
    def __init__(self,x=200,p=4,file_name='your_mom'):

        self.base_image = []
        self.filtered_image=[]
        self.cell_size = x
        self.padding   = p
        self.file_name = file_name
        if file_name!='your_mom':
            self.loadImage(file_name)


    def imageSize(self):
        return self.cell_size-2*self.padding

    def imageStartX(self):
        return self.padding

    def imageEndX(self):
        return self.cell_size-self.padding

    def loadImage(self,file_name):
        self.file_name = file_name
        local_image = cv2.imread(file_name)
        if local_image.shape[2] == 3:
            b_channel, g_channel, r_channel = cv2.split(local_image)
            alpha_channel = np.ones(b_channel.shape,
                                    dtype=b_channel.dtype) * 255  # creating a dummy alpha channel image.
            local_image = cv2.merge((b_channel, g_channel, r_channel, alpha_channel))
        self.base_image = local_image
        self.filtered_image = local_image.copy()

    def outputImage(self):
        final_image = np.zeros((self.cell_size, self.cell_size, 4), dtype=np.uint8)
        final_image[self.imageStartX():self.imageEndX(), self.imageStartX():self.imageEndX(), :] = \
            cv2.resize(self.filtered_image,(self.imageSize(),self.imageSize()),interpolation=cv2.INTER_AREA)
        return final_image


class Error(Exception):
   """Base class for other exceptions"""
   pass
class TooManyImages(Error):
   """Raised when there are too many images for the size spritemap"""
   pass
class SpriteSheet:
    def gen_sprite_sheet(sprite_list: [myImageSprite],output_file: str,cell_size: int,n_col: int):
        total_size = cell_size*n_col
        index_y = 0
        index_x = 0
        final_image = np.zeros((total_size, total_size, 4), dtype=np.uint8)
        for sprite in sprite_list:
            if(index_x==n_col):
                raise TooManyImages
            current_y = index_x*cell_size
            current_x = index_y*cell_size
            final_image[current_y:(current_y+cell_size),current_x:(current_x+cell_size),:]=sprite.outputImage()
            index_y += 1
            if(index_y==n_col):
                index_y = 0
                index_x+= 1
        cv2.imwrite(output_file, final_image)