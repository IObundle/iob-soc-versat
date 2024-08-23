import sys
from PIL import Image
import struct

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Need operation")

    operation = sys.argv[1]

    # Simple way of checking if the image was processed right.
    # Altought still not sure on the order of things.
    # We are storing pixels directly from the order used by PIL
    if operation == "unpack":
        img = Image.new("RGB", (416, 416), (255, 255, 255))
        access = img.load()
        with open("output.rgb", "rb") as file:
            for i in range(3):
                for y in range(416):
                    for x in range(416):
                        value = file.read(4)
                        asFloat = struct.unpack("<f", value)[0]

                        if i == 0:
                            access[x, y] = (
                                int(asFloat * 255.0),
                                access[x, y][1],
                                access[x, y][2],
                            )
                        if i == 1:
                            access[x, y] = (
                                access[x, y][0],
                                int(asFloat * 255.0),
                                access[x, y][2],
                            )
                        if i == 2:
                            access[x, y] = (
                                access[x, y][0],
                                access[x, y][1],
                                int(asFloat * 255.0),
                            )

        img.show()

    if operation == "pack":
        if len(sys.argv) < 3:
            print("Need name of image")

        imageFilepath = sys.argv[2]
        img = Image.open(imageFilepath)
        img = img.resize((416, 416))

        asRGB = Image.new("RGB", img.size, (255, 255, 255))
        asRGB.paste(img)

        with open("output.rgb", "wb") as file:
            for i in range(3):
                channel = list(asRGB.getchannel(i).getdata())

                for color in channel:
                    assert color / 255 <= 1.0
                    file.write(struct.pack("<f", color / 255))
