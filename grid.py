import os
import argparse
import subprocess
'''
This script will take a section of a tif image and segment it up according to the entered sizes
I have put in a guessed segment size for what size is too big and would shut down imagemagick

This script will work but at the same time it is really intensive on both processing power and memory
I honestly recommend not trying to use it unless necessary 
'''


os.chdir("/home/nelson/Desktop/SplitImages")

parser = argparse.ArgumentParser(description='commands to grid up out image')

parser.add_argument("-t", "--tif", default="cyto.tif", type=str, help="the main large tif image")
parser.add_argument("-H", "--height", default=10000, type=int, help="height")
parser.add_argument("-w", "--width", default=10000, type=int, help="width")

args = vars(parser.parse_args())

image = str(args["tif"])

if int(args["height"]) > 20000 or int(args["height"]) < 1000:
    print("do not use this image size")

if int(args["width"]) > 20000 or int(args["width"]) < 1000:
    print("do not use this image size")


temp = subprocess.Popen("identify " + str(args["tif"]), shell=True, stdout=subprocess.PIPE).stdout.read()



stuff = str(temp).split("\\n")

page_to_grab = 0
holder = 0
counter = 0
image_height = 0
image_width = 0

for item in stuff:
    temp = item.split(" ")
    if len(temp[0]) > 2:
        page = temp[2].split("x")
        if int(page[0]) > holder:
            holder = int(page[0])
            image_height = int(page[0])
            image_width = int(page[1])
            page_to_grab = counter

    counter += 1

print(image_height)
print(image_width)



height = int(args["height"])
width = int(args["width"])
step_right = 0
step_down = 0

for row in range(0, int(image_height/height)):
    for col in range(0, int(image_width/width)):
        convert_string = "convert -extract " + str(height) + "x" + str(width) + "+" + str(step_down) + "+" + str(step_right) + " " + image + "[" + str(0) + "] cytoOut" + str(step_down) + "x" + str(step_right) + ".png"
        print(convert_string)
        subprocess.Popen(convert_string, shell=True)
        step_right += width
    step_right = 0
    step_down += height




