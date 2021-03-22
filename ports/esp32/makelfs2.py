from littlefs import LittleFS
import os
import sys

def make_image(path_in, path_out, image_size):
    if image_size % 4096 != 0:
        raise ValueError("image size must be divide 4096") 

    fs = LittleFS(block_size=4096, block_count=image_size // 4096)

    file_walk = os.walk(path_in)
    for base_path, path, file_list in file_walk:
        base_path = base_path  +  '/'
        out_path = base_path[len(path_in) + 1:]
        
        for d in path:
            fs.mkdir(out_path + d)

        for file_in in file_list:
            with fs.open(out_path + file_in, 'w') as fh:
                with open(base_path + file_in, 'rb') as fi:
                    fh.write(fi.read())

    with open(path_out, 'wb') as fh:
        fh.write(fs.context.buffer)

path_in = sys.argv[1]
path_out = sys.argv[2]

if sys.argv[3].startswith('0x'):
    image_size = int(sys.argv[3], 16)
else:
    image_size = int(sys.argv[3])

if not os.path.exists(path_in):
    print("Not found", path_in)
    sys.exit(1)

print("Create lfs2 image to", path_out)
make_image(path_in, path_out, image_size)
