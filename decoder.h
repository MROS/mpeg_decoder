#ifndef MPEG_DECODER_DECODER_H
#define MPEG_DECODER_DECODER_H

#include <fstream>
#include <stdint.h>
#include <vector>
#include <queue>
#include <stdint.h>
#include <memory>

#include "bit_reader.h"
#include "image_queue.h"

static uint8_t scan[8][8] = {
		{ 0,  1,  5,  6, 14, 15, 27, 28},
		{ 2,  4,  7, 13, 16, 26, 29, 42},
		{ 3,  8, 12, 17, 25, 30, 41, 43},
		{ 9, 11, 18, 24, 31, 40, 44, 53},
		{10, 19, 23, 32, 39, 45, 52, 54},
		{20, 22, 33, 38, 46, 51, 55, 60},
		{21, 34, 37, 47, 50, 56, 59, 61},
		{35, 36, 48, 49, 57, 58, 62, 63}
};

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
	uint32_t address_increment;
	uint32_t macroblock_type;
	uint32_t quantizer_scale;
	uint32_t motion_horizontal_forward_code;
	uint32_t motion_horizontal_forward_r;
	uint32_t motion_vertical_forward_code;
	uint32_t motion_vertical_forward_r;

	uint32_t motion_horizontal_backward_code;
	uint32_t motion_horizontal_backward_r;
	uint32_t motion_vertical_backward_code;
	uint32_t motion_vertical_backward_r;

	uint32_t coded_block_pattern;
};

struct Slice {
	uint32_t vertical_position;
	uint32_t quantizer_scale;
	std::vector<uint32_t> extra_information_slice;

	std::vector<std::shared_ptr<Macroblock>> macroblocks;
};

struct Picture {
	uint32_t temporal_reference;
	uint32_t picture_coding_type;
	uint32_t vbv_delay;
	bool full_pel_forward_vector;
	uint32_t forward_f_code;
	bool full_pel_backward_vector;
	uint32_t backward_f_code;
	std::vector<uint32_t> extra_information_picture;
	std::vector<uint32_t> picture_extension_data;
	std::vector<uint32_t> user_data;

	std::vector<std::shared_ptr<Slice>> slices;
};

struct GroupOfPictures {
	uint32_t time_code;
	uint32_t closed_gop;
	uint32_t broken_link;
	std::vector<uint32_t> group_extension_data;
	std::vector<uint32_t> user_data;

	std::vector<std::shared_ptr<Picture>> pictures;
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
	std::vector<uint32_t> user_data;

	SequenceHeader() {
		// TODO: 改成 sizeof(default_intra_quantizer_matrix)
		std::copy(&default_intra_quantizer_matrix[0][0],
				  &default_intra_quantizer_matrix[0][0] + sizeof(uint8_t) * 64,
				  &this->intra_quantizer_matrix[0][0]);
		std::copy(&default_non_intra_quantizer_matrix[0][0],
				  &default_non_intra_quantizer_matrix[0][0] + sizeof(uint8_t) * 64,
				  &this->non_intra_quantizer_matrix[0][0]);
	}
};

class Decoder {
private:
	BitReader bit_reader;

	// 以一個 group_of_pictures 爲單位緩存
	SequenceHeader sequence_header;
	GroupOfPictures group_of_pictures;
	std::shared_ptr<Picture> cur_picture; // cur 前綴代表 current。
//	Slice *cur_slice;
//	Macroblock *cur_macroblock;
//	Block *cur_block;

	uint32_t picture_coding_type;
    std::shared_ptr<ImageQueue> image_queue;
public:
	Decoder(std::ifstream &f, std::shared_ptr<ImageQueue> image_queue): bit_reader(f), image_queue(image_queue) {};
	void start();
	void video_sequence();
	void read_sequence_header();

	void read_group_of_pictures();

	void read_picture();

	void read_slice();

	void read_macroblock(Slice &slice);

	void read_block(int i);

};

#endif //MPEG_DECODER_DECODER_H
