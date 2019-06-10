#ifndef MPEG_DECODER_DECODER_H
#define MPEG_DECODER_DECODER_H

#include <fstream>
#include <stdint.h>
#include <vector>
#include <queue>
#include <stdint.h>

#include "bit_reader.h"
#include "image_queue.h"

static uint8_t default_intra_quantizer_matrix[8][8] = {
		{8, 16, 19, 22, 26, 27, 29, 34},
		{16, 16, 22, 24, 27, 29, 34, 37},
		{19, 22, 26, 27, 29, 34, 34, 38},
		{22, 22, 26, 27, 29, 34, 37, 40},
		{22, 26, 27, 29, 32, 35, 40, 48},
		{26, 27, 29, 32, 35, 40, 48, 58},
		{26, 27, 29, 34, 38, 46, 56, 69},
		{27, 29, 35, 38, 46, 56, 69, 83}
};

static uint8_t default_non_intra_quantizer_matrix[8][8] = {
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
};

struct Block {

};

struct Macroblock {

};

struct Slice {

};

struct Picture {

};

struct GroupOfPictures {

};

struct SequenceHeader {
	uint32_t horizontal_size;
	uint32_t vertical_size;
	uint32_t pel_aspect_ratio;
	uint32_t picture_rate;
	uint32_t bit_rate;
	uint32_t vbv_buffer_size;
	uint32_t constrained_parameter_flag;
	uint8_t intra_quantizer_matrix[8][8];
	uint8_t non_intra_quantizer_matrix[8][8];

	SequenceHeader() {
		std::copy(&default_intra_quantizer_matrix[0][0],
				  &default_intra_quantizer_matrix[0][0] + sizeof(uint8_t) * 64,
				  &this->intra_quantizer_matrix[0][0]);
		std::copy(&default_non_intra_quantizer_matrix[0][0],
				  &default_non_intra_quantizer_matrix[0][0] + sizeof(uint8_t) * 64,
				  &this->non_intra_quantizer_matrix[0][0]);
	}
};

struct Sequence {
	SequenceHeader header;
	GroupOfPictures group_of_pictures;
};

class Decoder {
private:
	BitReader bit_reader;
	Sequence sequence;
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
