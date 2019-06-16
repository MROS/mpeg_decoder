#include <cmath>
#include <memory.h>
#include "YCbCr.h"

static unsigned char chomp(double x) {
	if (x > 255.0) {
		return 255;
	} else if (x < 0) {
		return 0;
	} else {
		return (unsigned char) round(x);
	}
}

void YCbCrImage::create(int width, int height) {
    this->width = width;
    this->height = height;
    buffer = new YCbCr*[height];
    for (int i = 0; i < height; i++) {
        buffer[i] = new YCbCr[width];
        memset(buffer[i], 0, sizeof(YCbCr) * width);
    }
}

// NOTE: 不傳值可加速
sf::Image YCbCrImage::to_image() {
    sf::Image image;
    image.create(width, height);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            YCbCr &p = buffer[i][j];
            sf::Color color;
			// 公式來源： http://softpixel.com/~cwright/programming/colorspace/yuv/
            color.r = chomp(p.Y + 1.4075 * (p.Cr-128));
            color.g = chomp(p.Y - 0.3455*(p.Cb-128) - 0.7169*(p.Cr-128));
            color.b = chomp(p.Y + 1.779*(p.Cb-128));
            image.setPixel(j, i, color);
        }
    }
    return image;
}