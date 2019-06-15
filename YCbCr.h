#ifndef MPEG_DECODER_YCBCR_H
#define MPEG_DECODER_YCBCR_H

#include <SFML/Graphics/Image.hpp>

struct YCbCr {
	int Y, Cb, Cr;
};

struct YCbCrImage {
	int width, height;
	YCbCr **buffer;	
	void create(int width, int height);
	~YCbCrImage() {
        for (int i = 0; i < height; i++) {
            delete buffer[i];
        }
        delete buffer;
    }
	sf::Image to_image();
};

#endif
