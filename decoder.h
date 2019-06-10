#ifndef MPEG_DECODER_DECODER_H
#define MPEG_DECODER_DECODER_H

#include <fstream>
#include <stdint.h>
#include <vector>
#include <queue>

#include "bit_reader.h"
#include "image_queue.h"

struct Sequence {

};

struct GroupOfPictures {

};

struct Picture {

};

struct Slice {

};

struct Macroblock {

};

struct Block {

};

class Decoder {
private:
	BitReader bit_reader;
	uint32_t picture_coding_type;
    std::shared_ptr<ImageQueue> image_queue;
public:
	Decoder(std::ifstream &f, std::shared_ptr<ImageQueue> image_queue): bit_reader(f), image_queue(image_queue) {};
	void start();
	void video_sequence();
	void sequence_header();

	void group_of_pictures();

	void picture();

	void slice();

	void macroblock();

	void block(int i);

};

#endif //MPEG_DECODER_DECODER_H
