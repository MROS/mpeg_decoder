#ifndef MPEG_DECODER_DECODER_H
#define MPEG_DECODER_DECODER_H

#include <fstream>
#include <stdint.h>
#include <vector>
#include <queue>
#include <stdint.h>
#include <memory>

#include "image_queue.h"
#include "util.h"
#include "YCbCr.h"
#include "bit_reader.h"

static const int scan[8][8] = {
		{ 0,  1,  5,  6, 14, 15, 27, 28},
		{ 2,  4,  7, 13, 16, 26, 29, 42},
		{ 3,  8, 12, 17, 25, 30, 41, 43},
		{ 9, 11, 18, 24, 31, 40, 44, 53},
		{10, 19, 23, 32, 39, 45, 52, 54},
		{20, 22, 33, 38, 46, 51, 55, 60},
		{21, 34, 37, 47, 50, 56, 59, 61},
		{35, 36, 48, 49, 57, 58, 62, 63}
};

static const uint8_t default_intra_quantizer_matrix[8][8] = {
		{8, 16, 19, 22, 26, 27, 29, 34},
		{16, 16, 22, 24, 27, 29, 34, 37},
		{19, 22, 26, 27, 29, 34, 34, 38},
		{22, 22, 26, 27, 29, 34, 37, 40},
		{22, 26, 27, 29, 32, 35, 40, 48},
		{26, 27, 29, 32, 35, 40, 48, 58},
		{26, 27, 29, 34, 38, 46, 56, 69},
		{27, 29, 35, 38, 46, 56, 69, 83}
};

static const uint8_t default_non_intra_quantizer_matrix[8][8] = {
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
		{16, 16, 16, 16, 16, 16, 16, 16},
};

struct Macroblock {
	MacroblockType type;

	int motion_horizontal_forward_code;
	uint32_t motion_horizontal_forward_r;
	int motion_vertical_forward_code;
	uint32_t motion_vertical_forward_r;

	int motion_horizontal_backward_code;
	uint32_t motion_horizontal_backward_r;
	int motion_vertical_backward_code;
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

	YCbCrImage y_cb_cr_image;

	// sf::Image image;

	uint32_t forward_r_size() {
		return forward_f_code - 1;
	}
	uint32_t forward_f() {
		return 1 << forward_r_size();
	}
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
    std::shared_ptr<ImageQueue> image_queue;

	// 以一個 group_of_pictures 爲單位緩存
	SequenceHeader sequence_header;
	GroupOfPictures group_of_pictures;
	std::shared_ptr<Picture> cur_picture; // cur 前綴代表 current。
	std::shared_ptr<Picture> cur_I_frame; // cur 前綴代表 current。
	std::shared_ptr<Picture> cur_P_frame; // cur 前綴代表 current。
	Macroblock cur_macroblock;

	int cur_quantizer_scale;
	int past_intra_address;
	int cur_macroblock_address;

	int cur_mb_width;

	int dct_dc_y_past, dct_dc_cb_past, dct_dc_cr_past;

	int recon_right_for, recon_down_for; // for 代表 forward
	int recon_right_for_prev, recon_down_for_prev; // for 代表 forward
	int right_for, down_for;

	int picture_counter;
	int macroblock_counter;

public:
	Decoder(std::ifstream &f, std::shared_ptr<ImageQueue> image_queue): bit_reader(f), image_queue(image_queue), picture_counter(0), macroblock_counter(0) {};
	void start();
	void video_sequence();
	void read_sequence_header();

	void read_group_of_pictures();

	void read_picture();

	void read_slice();

	void read_macroblock();

	void recon_block(int dest[8][8], std::shared_ptr<int> dct_zz, int index);

	std::shared_ptr <int> read_block(int i, bool macroblock_intra);

	void calculate_motion_vector(int f, int code, int r, int &recon_prev, int &recon, int full_pel_vector, int &mv_component);
	// void calculate_forward_motion_vector();

	void compensate();
	void merge_blocks(double source[6][8][8]);
	void reset_forward_motion_vector() {
		right_for = 0;
		down_for = 0;
		recon_right_for_prev = 0;
		recon_down_for_prev = 0;
		recon_right_for = 0;
		recon_down_for = 0;
	}

};

#endif //MPEG_DECODER_DECODER_H
