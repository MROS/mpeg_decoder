#include <stdint.h>
#include <iostream>
#include <memory>
#include <SFML/Graphics/Image.hpp>
#include <cstring>
#include "decoder.h"
#include "util.h"
#include "image_queue.h"

using namespace std;

const uint32_t picture_start_code = 0x00000100;

const uint32_t slice_start_code = 0x000001;
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
		this->read_sequence_header();
		do {
			this->read_group_of_pictures();
		} while (this->bit_reader.peek_bits(32) == group_start_code);
	} while (this->bit_reader.peek_bits(32) == sequence_header_code);
}

void Decoder::read_sequence_header() {
	if (this->bit_reader.eat_bits(32) != sequence_header_code) {
		throw "sequence header 開頭不是 sequence header code"s;
	}
	cout << "###### 讀取 sequence header" << endl;
	this->sequence_header.horizontal_size = this->bit_reader.eat_bits(12);
	cout << "horizontal size: " << this->sequence_header.horizontal_size << endl;

	this->sequence_header.vertical_size = this->bit_reader.eat_bits(12);
	cout << "vertical size: " << this->sequence_header.vertical_size << endl;

	this->sequence_header.pel_aspect_ratio = this->bit_reader.eat_bits(4);
	cout << "pel aspect ratio: " << this->sequence_header.pel_aspect_ratio << endl;

	this->sequence_header.picture_rate = this->bit_reader.eat_bits(4);
	cout << "picture rate: " << this->sequence_header.picture_rate << endl;

	this->sequence_header.bit_rate = this->bit_reader.eat_bits(18);
	cout << "bit rate: " << this->sequence_header.bit_rate << endl;

	uint32_t marker_bit = this->bit_reader.eat_bits(1);
    if (marker_bit != 1) {
    	throw "marker_bit 不等於 1"s;
    }

	this->sequence_header.vbv_buffer_size = this->bit_reader.eat_bits(10);
	cout << "vbv buffer size: " << this->sequence_header.vbv_buffer_size << endl;

	this->sequence_header.constrained_parameter_flag = this->bit_reader.eat_bits(1);
	cout << "constrained_parameter_flag: " << this->sequence_header.constrained_parameter_flag << endl;


	uint32_t load_intra_quantizer_matrix = this->bit_reader.eat_bits(1);
	cout << "load intra quantizer matrix: " << load_intra_quantizer_matrix << endl;

	// TODO: 支援自定義量化矩陣
	uint8_t intra_quantizer_matrix[64];
	if (load_intra_quantizer_matrix == 1) {
		throw "尚不支援自定義量化矩陣"s;
		for (int i = 0; i < 64; i++) {
			intra_quantizer_matrix[i] = this->bit_reader.eat_bits(8);
		}
	}

	uint32_t load_non_intra_quantizer_matrix = this->bit_reader.eat_bits(1);
	cout << "load non intra quantizer matrix: " << load_non_intra_quantizer_matrix << endl;

	uint8_t non_intra_quantizer_matrix[64];
	if (load_non_intra_quantizer_matrix == 1) {
		throw "尚不支援自定義量化矩陣"s;
		for (int i = 0; i < 64; i++) {
			non_intra_quantizer_matrix[i] = this->bit_reader.eat_bits(8);
		}
	}

	this->bit_reader.next_start_code();

	if (this->bit_reader.peek_bits(32) == extension_start_code) {
		this->bit_reader.eat_bits(32);  // 待優化: 不用重新讀取，直接移動讀寫頭即可
		cout << "extension_start_code" << endl;
		while(this->bit_reader.peek_bits(24) != 1) {
			throw "尚不支援 sequence extension data"s;
			uint8_t sequence_extension_data = this->bit_reader.eat_bits(8);
			cout << "sequence_extension_data: " << sequence_extension_data << endl;
		}
		this->bit_reader.next_start_code();
	}

	if (this->bit_reader.peek_bits(32) == user_data_start_code) {
		this->bit_reader.eat_bits(32);
		cout << "user_data_start_code" << endl;
		while(this->bit_reader.peek_bits(24) != 1) {
			throw "尚不支援 sequence user data"s;
			uint8_t user_data = this->bit_reader.eat_bits(8);
			cout << "user_data: " << user_data << endl;
		}
		this->bit_reader.next_start_code();
	}
}

void Decoder::read_group_of_pictures() {
	if (this->bit_reader.eat_bits(32) != group_start_code) {
		throw "group of pictures 開頭不是 group_start_code"s;
	}
	cout << endl << "###### 讀取 group start code" << endl;

	this->group_of_pictures.time_code = this->bit_reader.eat_bits(25);
	cout << "time_code: " << this->group_of_pictures.time_code << endl;
	this->group_of_pictures.closed_gop = this->bit_reader.eat_bits(1);
	cout << "closed_gop: " << this->group_of_pictures.closed_gop << endl;
	this->group_of_pictures.broken_link = this->bit_reader.eat_bits(1);
	cout << "broken_link: " << this->group_of_pictures.broken_link << endl;

	this->bit_reader.next_start_code();

	if (this->bit_reader.peek_bits(32) == extension_start_code) {
		this->bit_reader.eat_bits(32);
		while (this->bit_reader.peek_bits(24) != 1) {
			throw "尚不支援 group extension data"s;
			uint32_t group_extension_data = this->bit_reader.eat_bits(8);
			cout << "group_extension_data:" << group_extension_data << endl;
		}
		this->bit_reader.next_start_code();
	}
	if (this->bit_reader.peek_bits(32) == user_data_start_code) {
		this->bit_reader.eat_bits(32);
		while (this->bit_reader.peek_bits(24) != 1) {
			throw "尚不支援 group user data"s;
			uint32_t user_data = this->bit_reader.eat_bits(8);
			cout << "user_data:" << user_data << endl;
		}
		this->bit_reader.next_start_code();
	}
	do {
		this->read_picture();
	} while (this->bit_reader.peek_bits(32) == picture_start_code);
}

void Decoder::read_picture() {
	if (this->bit_reader.eat_bits(32) != picture_start_code) {
		throw "picture 開頭不是 picture_start_code"s;
	}
	cout << endl << "###### 讀取 picture start code" << endl;

	auto picture = make_shared<Picture>();

	picture->temporal_reference = this->bit_reader.eat_bits(10);
	cout << "temporal_reference: " << picture->temporal_reference << endl;
	picture->picture_coding_type = this->bit_reader.eat_bits(3);
	cout << "picture_coding_type: " << picture->picture_coding_type << endl;
	picture->vbv_delay = this->bit_reader.eat_bits(16);
	cout << "vbv_delay: " << picture->vbv_delay << endl;

	if (picture->picture_coding_type == 2 || picture->picture_coding_type == 3) {
		throw "目前僅支援 I 幀"s;
		uint32_t full_pel_forward_vector = this->bit_reader.eat_bits(1);
		cout << "full_pel_forward_vector: " << full_pel_forward_vector << endl;
		uint32_t forward_f_code = this->bit_reader.eat_bits(3);
		cout << "forward_f_code: " << forward_f_code << endl;
	}
	if (picture->picture_coding_type == 3) {
		uint32_t full_pel_backward_vector = this->bit_reader.eat_bits(1);
		cout << "full_pel_backward_vector: " << full_pel_backward_vector << endl;
		uint32_t backward_f_code = this->bit_reader.eat_bits(3);
		cout << "backward_f_code: " << backward_f_code << endl;
	}
	while (this->bit_reader.peek_bits(1) == 1) {
		throw "尚不支援 extra information picture"s;
		this->bit_reader.eat_bits(1);
		uint32_t extra_information_picture = this->bit_reader.eat_bits(8);
		cout << "extra_information_picture: " << extra_information_picture << endl;
	}
	this->bit_reader.eat_bits(1);       // extra_bit_picture, "0"
	this->bit_reader.next_start_code();

	if (this->bit_reader.peek_bits(32) == extension_start_code) {
		this->bit_reader.eat_bits(32);
		while (this->bit_reader.peek_bits(24) != 1) {
			throw "尚不支援 picture extension data"s;
			uint32_t picture_extension_data = this->bit_reader.eat_bits(8);
			cout << "picture_extension_data:" << picture_extension_data << endl;
		}
		this->bit_reader.next_start_code();
	}
	if (this->bit_reader.peek_bits(32) == user_data_start_code) {
		this->bit_reader.eat_bits(32);
		while (this->bit_reader.peek_bits(24) != 1) {
			throw "尚不支援 picture user data"s;
			uint32_t user_data = this->bit_reader.eat_bits(8);
			cout << "user_data:" << user_data << endl;
		}
		this->bit_reader.next_start_code();
	}
	this->cur_picture = picture;
	do {
		this->read_slice();
	} while(this->bit_reader.peek_bits(32) <= slice_start_code_max && this->bit_reader.peek_bits(32) >= slice_start_code_min);
}

void Decoder::read_slice() {
	if (this->bit_reader.eat_bits(24) != slice_start_code) {
		throw "slice 開頭不是 slice_start_code"s;
	}
	cout << endl << "###### 讀取 slice start code: " << endl;
	Slice slice;
	slice.vertical_position = this->bit_reader.eat_bits(8);
	if (slice.vertical_position > 0xAF) {
		throw "slice vertical_positon 超出範圍";
	}
	cout << "vertical_position: " << slice.vertical_position << endl;

	uint32_t quantizer_scale = this->bit_reader.eat_bits(5);
	cout << "quantizer_scale: " << quantizer_scale << endl;

	while (this->bit_reader.peek_bits(1) == 1) {
		throw "尚不支援 extra information slice"s;
		this->bit_reader.eat_bits(1);
		uint32_t extra_information_slice = this->bit_reader.eat_bits(8);
		cout << "extra_information_slice: " << extra_information_slice << endl;
	}
	this->bit_reader.eat_bits(1);

	do {
		this->read_macroblock(slice);
	} while(this->bit_reader.peek_bits(23) != 0);
	this->bit_reader.next_start_code();
}

void Decoder::read_macroblock(Slice &slice) {
	while(this->bit_reader.peek_bits(11) == 15) {
		this->bit_reader.eat_bits(11);
		cout << endl << "###### 讀取 macroblock stuffing" << endl;
	}


	int escape_count = 0;
	while(this->bit_reader.peek_bits(11) == 8) {
		this->bit_reader.eat_bits(11);
		cout << endl << "###### 讀取 macroblock escape" << endl;
		escape_count += 1;
	}
	cout << endl << "###### 繼續讀取 macroblock" << endl;

	IntWrap macroblock_address_increment = this->bit_reader.read_vlc(this->bit_reader.macroblock_addr);
	cout << "macroblock_address_increment: " << macroblock_address_increment.value << endl;

	int mb_width = (this->sequence_header.horizontal_size + 15) / 16;
	int previous_macroblock_address = (slice.vertical_position - 1) * mb_width - 1;
	cout << "previous_macroblock_address: " << previous_macroblock_address << endl;

	uint32_t macroblock_address = previous_macroblock_address;
	macroblock_address += escape_count * 33;
	macroblock_address += macroblock_address_increment.value;
	cout << "macroblock_address: " << macroblock_address << endl;


	// TODO: 支援 p 跟 b 幀
	MacroblockType macroblock_type = this->bit_reader.read_vlc(this->bit_reader.intra_macroblock_type);
	cout << "macroblock_quant: " << macroblock_type.quant << endl;
	cout << "macroblock_motion_forward: " << macroblock_type.motion_forward << endl;
	cout << "macroblock_motion_backward: " << macroblock_type.motion_backward << endl;
	cout << "macroblock_pattern: " << macroblock_type.pattern << endl;
	cout << "macroblock_intra: " << macroblock_type.intra << endl;

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

	bool pattern_code[6] = {false, false, false, false, false, false};
	if (macroblock_type.pattern) {
		IntWrap coded_block_pattern = this->bit_reader.read_vlc(this->bit_reader.coded_block_pattern);
		cout << "coded_block_pattern: " << coded_block_pattern.value << endl;
		int cbp = coded_block_pattern.value;
		for (int i = 0; i < 6; i++) {
			if (cbp & (1 << (5 - i))) {
				pattern_code[i] = true;
			}
		}
	}

	if (macroblock_type.intra) {
		for (int i = 0; i < 6; i++) {
			pattern_code[i] = true;
		}
	}

	for (int i = 0; i < 6; i++) {
		if (pattern_code[i]) {
			this->read_block(i, macroblock_type.intra, this->cur_picture->picture_coding_type);
		}
	}
	exit(0);

	if (this->picture_coding_type == 1) {
		uint32_t end_of_macroblock = this->bit_reader.eat_bits(1);
		if (end_of_macroblock != 1) {
			throw "end of macroblock != 1"s;
		}
	}

}

shared_ptr<int> Decoder::read_block(int i, bool macroblock_intra, int picture_coding_type) {
	cout << endl << "###### 讀取 block " << i << endl;

	int *dct_zz = new int[64];
    memset(dct_zz, 0, 64 * sizeof(int));
	int index = 0;

	if (macroblock_intra) {
		// 取得 dct_dc_size
		uint32_t dct_dc_size;
		if (i < 4) {
			dct_dc_size = this->bit_reader.read_vlc(this->bit_reader.dct_dc_size_luminance).value;
			cout << "dct_dc_size_luminance: " << dct_dc_size << endl;
		} else {
			dct_dc_size = this->bit_reader.read_vlc(this->bit_reader.dct_dc_size_chrominance).value;
			cout << "dct_dc_size_chrominance: " << dct_dc_size << endl;
		}

		// 根據 size 計算 dct_zz[0]
		if (dct_dc_size == 0) {
			dct_zz[0] = 0;
		} else {
			uint32_t dct_dc_differential = this->bit_reader.eat_bits(dct_dc_size);
			cout << "dct_dc_differential: " << dct_dc_differential << endl;
			if (dct_dc_differential & (1 << (dct_dc_size - 1))) {
				dct_zz[0] = dct_dc_differential;
			} else {
				dct_zz[0] = (-1 << (dct_dc_size)) | (dct_dc_differential + 1);
			}
		}
		cout << "dct_zz[0]: " << dct_zz[0] << endl;
	} else {
        RunLevel run_level = this->bit_reader.read_vlc(this->bit_reader.run_level);
        index = run_level.run;
        uint32_t s = this->bit_reader.eat_bits(1);
        if (s == 0) { dct_zz[index] = run_level.level; }
        else if (s == 1) { dct_zz[index] = -run_level.level; }
	}
	if (picture_coding_type != 4) {
		while (this->bit_reader.peek_bits(2) != 0b10) {
			RunLevel run_level = this->bit_reader.read_vlc(this->bit_reader.run_level);
			index = index + run_level.run + 1;
			if (index > 63) { throw "dct_coeff_next 不合理 index"s; }
			uint32_t s = this->bit_reader.eat_bits(1);
			if (s == 0) { dct_zz[index] = run_level.level; }
			else if (s == 1) { dct_zz[index] = -run_level.level; }
		}
		this->bit_reader.eat_bits(2);
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			cout << dct_zz[i * 8 + j] << " ";
		}
		cout << endl;
	}

	return shared_ptr<int>(dct_zz);
}
