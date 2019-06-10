#include <stdint.h>
#include <iostream>
#include <memory>
#include <SFML/Graphics/Image.hpp>
#include "decoder.h"
#include "util.h"
#include "image_queue.h"

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


void Decoder::start() {
//	auto image_names = dir_list("../images");
//	for (int i = 2; i < image_names.size(); i++) {
//		string image_name = image_names[i];
//		sf::Image image;
//		if (!image.loadFromFile("../images/" + image_name)) {
//			cout << "無法開啓檔案: " << image_name << endl;
//		}
//		this->image_queue->push(make_shared<sf::Image>(image));
//	}
	this->video_sequence();
}

void Decoder::video_sequence() {
	do {
		this->sequence_header();
		do {
			this->group_of_pictures();
		} while (this->bit_reader.peek_bits(32) == group_start_code);
	} while (this->bit_reader.peek_bits(32) == sequence_header_code);
}

void Decoder::sequence_header() {
	if (this->bit_reader.eat_bits(32) != sequence_header_code) {
		throw "sequence header 開頭不是 sequence header code"s;
	}
	cout << "###### 讀取 sequence header" << endl;
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

	uint8_t intra_quantizer_matrix[64];
	if (load_intra_quantizer_matrix == 1) {
		for (int i = 0; i < 64; i++) {
			intra_quantizer_matrix[i] = this->bit_reader.eat_bits(8);
		}
	}

	uint32_t load_non_intra_quantizer_matrix = this->bit_reader.eat_bits(1);
	cout << "load non intra quantizer matrix: " << load_non_intra_quantizer_matrix << endl;

	uint8_t non_intra_quantizer_matrix[64];
	if (load_non_intra_quantizer_matrix == 1) {
		for (int i = 0; i < 64; i++) {
			non_intra_quantizer_matrix[i] = this->bit_reader.eat_bits(8);
		}
	}

	this->bit_reader.next_start_code();

	if (this->bit_reader.peek_bits(32) == extension_start_code) {
		this->bit_reader.eat_bits(32);  // 待優化: 不用重新讀取，直接移動讀寫頭即可
		cout << "extension_start_code" << endl;
		while(this->bit_reader.peek_bits(24) != 1) {
			uint8_t sequence_extension_data = this->bit_reader.eat_bits(8);
			cout << "sequence_extension_data: " << sequence_extension_data << endl;
		}
		this->bit_reader.next_start_code();
	}

	if (this->bit_reader.peek_bits(32) == user_data_start_code) {
		this->bit_reader.eat_bits(32);
		cout << "user_data_start_code" << endl;
		while(this->bit_reader.peek_bits(24) != 1) {
			uint8_t user_data = this->bit_reader.eat_bits(8);
			cout << "sequence_extension_data: " << user_data << endl;
		}
		this->bit_reader.next_start_code();
	}
}

void Decoder::group_of_pictures() {
	if (this->bit_reader.eat_bits(32) != group_start_code) {
		throw "group of pictures 開頭不是 group_start_code"s;
	}
	cout << endl << "###### 讀取 group start code" << endl;

	uint32_t time_code = this->bit_reader.eat_bits(25);
	cout << "time_code: " << time_code << endl;
	uint32_t closed_gop = this->bit_reader.eat_bits(1);
	cout << "closed_gop: " << closed_gop << endl;
	uint32_t broken_link = this->bit_reader.eat_bits(1);
	cout << "broken_link: " << broken_link << endl;

	this->bit_reader.next_start_code();

	if (this->bit_reader.peek_bits(32) == extension_start_code) {
		this->bit_reader.eat_bits(32);
		while (this->bit_reader.peek_bits(24) != 1) {
			uint32_t group_extension_data = this->bit_reader.eat_bits(8);
			cout << "group_extension_data:" << group_extension_data << endl;
		}
		this->bit_reader.next_start_code();
	}
	if (this->bit_reader.peek_bits(32) == user_data_start_code) {
		this->bit_reader.eat_bits(32);
		while (this->bit_reader.peek_bits(24) != 1) {
			uint32_t user_data = this->bit_reader.eat_bits(8);
			cout << "user_data:" << user_data << endl;
		}
		this->bit_reader.next_start_code();
	}
	do {
		this->picture();
	} while (this->bit_reader.peek_bits(32) == picture_start_code);
}

void Decoder::picture() {
	if (this->bit_reader.eat_bits(32) != picture_start_code) {
		throw "picture 開頭不是 picture_start_code"s;
	}
	cout << endl << "###### 讀取 picture start code" << endl;

	uint32_t temporal_reference = this->bit_reader.eat_bits(10);
	cout << "temporal_reference: " << temporal_reference << endl;
	this->picture_coding_type = this->bit_reader.eat_bits(3);
	cout << "picture_coding_type: " << picture_coding_type << endl;
	uint32_t vbv_delay = this->bit_reader.eat_bits(16);
	cout << "vbv_delay: " << vbv_delay << endl;

	if (picture_coding_type == 2 || picture_coding_type == 3) {
		uint32_t full_pel_forward_vector = this->bit_reader.eat_bits(1);
		cout << "full_pel_forward_vector: " << full_pel_forward_vector << endl;
		uint32_t forward_f_code = this->bit_reader.eat_bits(3);
		cout << "forward_f_code: " << forward_f_code << endl;
	}
	if (picture_coding_type == 3) {
		uint32_t full_pel_backward_vector = this->bit_reader.eat_bits(1);
		cout << "full_pel_backward_vector: " << full_pel_backward_vector << endl;
		uint32_t backward_f_code = this->bit_reader.eat_bits(3);
		cout << "backward_f_code: " << backward_f_code << endl;
	}
	while (this->bit_reader.peek_bits(1) == 1) {
		this->bit_reader.eat_bits(1);
		uint32_t extra_information_picture = this->bit_reader.eat_bits(8);
		cout << "extra_information_picture: " << extra_information_picture << endl;
	}
	this->bit_reader.eat_bits(1);       // extra_bit_picture, "0"
	this->bit_reader.next_start_code();

	if (this->bit_reader.peek_bits(32) == extension_start_code) {
		this->bit_reader.eat_bits(32);
		while (this->bit_reader.peek_bits(24) != 1) {
			uint32_t picture_extension_data = this->bit_reader.eat_bits(8);
			cout << "picture_extension_data:" << picture_extension_data << endl;
		}
		this->bit_reader.next_start_code();
	}
	if (this->bit_reader.peek_bits(32) == user_data_start_code) {
		this->bit_reader.eat_bits(32);
		while (this->bit_reader.peek_bits(24) != 1) {
			uint32_t user_data = this->bit_reader.eat_bits(8);
			cout << "user_data:" << user_data << endl;
		}
		this->bit_reader.next_start_code();
	}
	do {
		this->slice();
	} while(this->bit_reader.peek_bits(32) <= slice_start_code_max && this->bit_reader.peek_bits(32) >= slice_start_code_min);
}

void Decoder::slice() {
	uint32_t code = this->bit_reader.eat_bits(32);
	if (code > slice_start_code_max || code < slice_start_code_min) {
		throw "slice 開頭不是 slice_start_code"s;
	}
	cout << endl << "###### 讀取 slice start code: " << hex << code << dec << endl;

	uint32_t quantizer_scale = this->bit_reader.eat_bits(5);
	cout << "quantizer_scale: " << quantizer_scale << endl;

	while (this->bit_reader.peek_bits(1) == 1) {
		this->bit_reader.eat_bits(1);
		uint32_t extra_information_slice = this->bit_reader.eat_bits(8);
		cout << "extra_information_slice: " << extra_information_slice << endl;
	}
	this->bit_reader.eat_bits(1);

	do {
		this->macroblock();
	} while(this->bit_reader.peek_bits(23) != 0);
	this->bit_reader.next_start_code();
}

void Decoder::macroblock() {
	while(this->bit_reader.peek_bits(11) == 15) {
		this->bit_reader.eat_bits(11);
		cout << endl << "###### 讀取 macroblock stuffing" << endl;
	}
	while(this->bit_reader.peek_bits(11) == 8) {
		this->bit_reader.eat_bits(11);
		cout << endl << "###### 讀取 macroblock escape" << endl;
	}
	cout << endl << "###### 繼續讀取 macroblock" << endl;

	IntWrap macroblock_address_increment = this->bit_reader.read_vlc(this->bit_reader.macroblock_addr);
	cout << "macroblock_address_increment: " << macroblock_address_increment.value << endl;

	// TODO: 支援 p 跟 b 幀
	MacroblockType macroblock_type = this->bit_reader.read_vlc(this->bit_reader.intra_macroblock_type);
	if (macroblock_type.quant) {
		uint32_t quantizer_scale = this->bit_reader.eat_bits(5);
		cout << "quantizer_scale: " << quantizer_scale << endl;
	}
	if (macroblock_type.motion_forward) {
		// TODO
		throw "尚未處理 motion_forward"s;
	}
	if (macroblock_type.motion_backward) {
		// TODO
		throw "尚未處理 motion_backward"s;
	}
	if (macroblock_type.pattern) {
		IntWrap coded_block_pattern = this->bit_reader.read_vlc(this->bit_reader.coded_block_pattern);
		cout << "coded_block_pattern: " << coded_block_pattern.value << endl;
	}
	for (int i = 0; i < 6; i++) {
		this->block(i);
	}

	if (this->picture_coding_type == 1) {
		uint32_t end_of_macroblock = this->bit_reader.eat_bits(1);
		if (end_of_macroblock != 1) {
			throw "end of macroblock != 1"s;
		}
	}

}

void Decoder::block(int i) {
}
