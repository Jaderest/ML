import cv2 as cv
import numpy as np

# image = np.ones(shape=[512,512,3], dtype="float32")
# image_ref = cv.imread("outdoor.hdr",-1)
# image = image * 1.2
# cv.imwrite("const.hdr",image)

img = cv.imread("/home/jaderest/Moer-lite/examples/texture/images/pattern_big.png")
img =np.resize(img,[4096,4096,3])
cv.imwrite("/home/jaderest/Moer-lite/examples/texture/images/pattern_new_big.png",img)
