#include <stdint.h>
#include <iostream>
#include "decoder.h"

using namespace std;

const uint32_t picture_start_code = 0x00000100;

const uint32_t slice_start_code_min = 0x00000101;
const uint32_t slice_start_code_max = 0x000001AF;

const uint32_t user_data_start_code = 0x000001B2;
const uint32_t sequence_header_code = 0x000001B3;
const uint32_t sequence_error_code = 0x000001B4;
const uint32_t extension_start_code = 0x000001B5;

const uint32_t sequence_end_code = 0x000001B7;
const uint32_t group_start_code = 0x000001B8;

const uint32_t system_start_code_min = 0x000001B9;
const uint32_t system_start_code_max = 0x000001FF;


void Decoder::play() {
	this->video_sequence();
}

void Decoder::video_sequence() {
	do {
		this->sequence_header();
		do {
//			this->group_of_pictures();
		} while (this->bit_reader.peek_bits(32) == group_start_code);
	} while (this->bit_reader.peek_bits(32) == sequence_header_code);
}

void Decoder::sequence_header() {
	if (this->bit_reader.eat_bits(32) != sequence_header_code) {
		throw "sequence header 開頭不是 sequence header code";
	}
	cout << "讀取 sequence header" << endl;
	uint32_t horizonatal_size = this->bit_reader.eat_bits(12);
	cout << "horizontal size: " << horizonatal_size << endl;
	uint32_t vertical_size = this->bit_reader.eat_bits(12);
	cout << "vertical size: " << vertical_size << endl;
	uint32_t pel_aspect_ratio = this->bit_reader.eat_bits(4);
	cout << "pel aspect ratio: " << pel_aspect_ratio << endl;
	uint32_t picture_rate = this->bit_reader.eat_bits(4);
	cout << "picture rate: " << picture_rate << endl;
	uint32_t bit_rate = this->bit_reader.eat_bits(18);
	cout << "bit rate: " << bit_rate << endl;
	uint32_t marker_bit = this->bit_reader.eat_bits(1);
	cout << "marker bit: " << marker_bit << endl;
	uint32_t vbv_buffer_size = this->bit_reader.eat_bits(10);
	cout << "vbv buffer size: " << vbv_buffer_size << endl;
	uint32_t constrained_parameter_flag = this->bit_reader.eat_bits(1);
	cout << "constrained_parameter_flag: " << constrained_parameter_flag << endl;
	uint32_t load_intra_quantizer_matrix = this->bit_reader.eat_bits(1);
	cout << "load intra quantizer matrix: " << load_intra_quantizer_matrix << endl;
	if (load_intra_quantizer_matrix == 1) {

	}
}

//void Decoder::group_of_pictures() {
//
//}
