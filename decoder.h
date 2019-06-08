#ifndef MPEG_DECODER_DECODER_H
#define MPEG_DECODER_DECODER_H

#include <fstream>
#include <stdint.h>

#include "bit_reader.h"

class Decoder {
private:
	BitReader bit_reader;
	uint32_t picture_coding_type;
public:
	Decoder(std::ifstream &f): bit_reader(f) {};
	void play();
	void video_sequence();
	void sequence_header();

	void group_of_pictures();

	void picture();

	void slice();

	void macroblock();

	void block(int i);

};

#endif //MPEG_DECODER_DECODER_H
