#ifndef MPEG_DECODER_DECODER_H
#define MPEG_DECODER_DECODER_H

#include <fstream>

#include "bit_reader.h"

class Decoder {
private:
	BitReader bit_reader;
public:
	Decoder(std::ifstream &f): bit_reader(f) {};
	void play();
	void video_sequence();
	void sequence_header();

	void group_of_pictures();
};

#endif //MPEG_DECODER_DECODER_H
