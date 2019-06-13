#include <stdint.h>
#include <iostream>
#include <fstream>
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

int sign(int x) {
	return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

void Decoder::start() {
	this->video_sequence();
}

void Decoder::video_sequence() {
	do {
		this->read_sequence_header();
		do {
			this->read_group_of_pictures();
		} while (this->bit_reader.peek_bits(32) == group_start_code);
	} while (this->bit_reader.peek_bits(32) == sequence_header_code);
	cout << "sequence 結束" << endl;
	exit(0);
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
	picture_counter++;
	this->image_queue->push(make_shared<sf::Image>(cur_picture->image));

	ofstream fout;
	fout.open("dp.ppm");
	int h = sequence_header.vertical_size;
	int w = sequence_header.horizontal_size;
	fout << "P6\n" << w << " " << h << "\n255\n";
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			auto pixel = cur_picture->image.getPixel(i, j);
			fout.write((char*)&pixel.r, 1);
			fout.write((char*)&pixel.g, 1);
			fout.write((char*)&pixel.b, 1);
		}
	}
	fout.close();

    cout << "Press enter to continue ...";
    cin.get();
     
}

void Decoder::read_picture() {
	if (this->bit_reader.eat_bits(32) != picture_start_code) {
		throw "picture 開頭不是 picture_start_code"s;
	}
	cout << endl << "###### 讀取 picture start code" << endl;

	auto picture = make_shared<Picture>();
	picture->image.create(sequence_header.horizontal_size, sequence_header.vertical_size);

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

	// 重置 dct_dc_y_past, dct_dc_cb_past, dct_dc_cr_past, past_intra_address
	this->dct_dc_y_past = 1024;
	this->dct_dc_cb_past = 1024;
	this->dct_dc_cr_past = 1024;
	this->past_intra_address = -2;

	Slice slice;
	slice.vertical_position = this->bit_reader.eat_bits(8);
	if (slice.vertical_position > 0xAF) {
		throw "slice vertical_positon 超出範圍";
	}
	cout << "vertical_position: " << slice.vertical_position << endl;

	cur_mb_width = (this->sequence_header.horizontal_size + 15) / 16;
	this->previous_macroblock_address = (slice.vertical_position - 1) * cur_mb_width - 1;
	cout << "reset previous_macroblock_address: " << this->previous_macroblock_address << endl;


	this->cur_quantizer_scale = this->bit_reader.eat_bits(5);
	cout << "quantizer_scale: " << this->cur_quantizer_scale << endl;

	while (this->bit_reader.peek_bits(1) == 1) {
		throw "尚不支援 extra information slice"s;
		this->bit_reader.eat_bits(1);
		uint32_t extra_information_slice = this->bit_reader.eat_bits(8);
		cout << "extra_information_slice: " << extra_information_slice << endl;
	}
	this->bit_reader.eat_bits(1);

	macroblock_counter = 0;
	do {
		this->read_macroblock(slice);
	} while(this->bit_reader.peek_bits(23) != 0);
	this->bit_reader.next_start_code();
}

void Decoder::decode_block(int dct_recon[8][8], shared_ptr<int> dct_zz, int index) {
	auto &quant = this->sequence_header.intra_quantizer_matrix;
	for (int m = 0; m < 8; m++) {
		for (int n = 0; n < 8; n++) {
			int i = scan[m][n];
			dct_recon[m][n] = (2 * (&*dct_zz)[i] * this->cur_quantizer_scale * (int)quant[m][n]) / 16;
			if ((dct_recon[m][n] & 1) == 0) {
				dct_recon[m][n] = dct_recon[m][n] - sign(dct_recon[m][n]);
			}
			if (dct_recon[m][n] > 2047) {
				dct_recon[m][n] = 2047;
			} else if (dct_recon[m][n] < -2048) {
				dct_recon[m][n] = -2048;
			}
		}
	}

	int *dct_dc_past;
	if (index < 4) {
		dct_dc_past = &dct_dc_y_past;
	} else if (index == 4) {
		dct_dc_past = &dct_dc_cb_past;
	} else {
		dct_dc_past = &dct_dc_cr_past;
	}

	if (index == 1 || index == 2 || index == 3) {  // 非第一個的 block
		dct_recon[0][0] = *dct_dc_past + (&*dct_zz)[0] * 8;
	} else { // 第一個 Y block, Cb block, Cr block

		dct_recon[0][0] = (&*dct_zz)[0] * 8;
		cout << "dct_recon[0][0]: " << dct_recon[0][0] << endl;

		if (this->cur_macroblock_address - this->past_intra_address > 1) {
			dct_recon[0][0] = 128 * 8 + dct_recon[0][0];
		} else {
			dct_recon[0][0] = *dct_dc_past + dct_recon[0][0];
		}
	}
	*dct_dc_past = dct_recon[0][0];
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
	cout << endl << "###### picture: " << picture_counter << endl;
	cout << endl << "###### 繼續讀取 macroblock: " << macroblock_counter << endl;
	macroblock_counter++;

	int macroblock_address_increment = this->bit_reader.read_vlc(this->bit_reader.macroblock_addr).value;
	cout << "macroblock_address_increment: " << macroblock_address_increment << endl;

	// NOTE: previous_macroblock_address 可拿掉
	this->cur_macroblock_address = this->previous_macroblock_address;
	this->cur_macroblock_address += escape_count * 33;
	this->cur_macroblock_address += macroblock_address_increment;
	this->previous_macroblock_address = this->cur_macroblock_address;
	cout << "macroblock_address: " << this->cur_macroblock_address << endl;


	// TODO: 支援 p 跟 b 幀
	MacroblockType macroblock_type = this->bit_reader.read_vlc(this->bit_reader.intra_macroblock_type);
	cout << "macroblock_quant: " << macroblock_type.quant << endl;
	cout << "macroblock_motion_forward: " << macroblock_type.motion_forward << endl;
	cout << "macroblock_motion_backward: " << macroblock_type.motion_backward << endl;
	cout << "macroblock_pattern: " << macroblock_type.pattern << endl;
	cout << "macroblock_intra: " << macroblock_type.intra << endl;

	if (macroblock_type.quant) {
		this->cur_quantizer_scale = this->bit_reader.eat_bits(5);
		cout << "quantizer_scale: " << this->cur_quantizer_scale << endl;
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
		int blocks[6][8][8];
		for (int i = 0; i < 6; i++) {
			if (pattern_code[i]) {
				shared_ptr<int> dct_zz = this->read_block(i, macroblock_type.intra, this->cur_picture->picture_coding_type);

				decode_block(blocks[i], dct_zz, i);
				// cout << "recon: " << endl;
				// for (int j = 0; j < 8; j++) {
				// 	for (int k = 0; k < 8; k++) {
				// 		cout << blocks[i][j][k] << " ";
				// 	}
				// 	cout << endl;
				// }

				double after_idct[8][8];
				idct(after_idct, blocks[i]);
				// cout << "after_idct: " << endl;
				// for (int j = 0; j < 8; j++) {
				// 	for (int k = 0; k < 8; k++) {
				// 		cout << after_idct[j][k] << " ";
				// 	}
				// 	cout << endl;
				// }

			}
		}
		this->past_intra_address = this->cur_macroblock_address;

		sf::Color dest[16][16];
		merge_blocks(dest, blocks);

		int mb_row = cur_macroblock_address / cur_mb_width;
		int mb_column = cur_macroblock_address % cur_mb_width;
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 16; j++) {
				// cout << (int)dest[i][j].b << " ";
				cur_picture->image.setPixel(mb_row + i, mb_column + j, dest[i][j]);
			}
			// cout << endl;
		}
		// exit(0);
	} else {
		// 重置 dct_dc_y_past, dct_dc_cb_past, dct_dc_cr_past
		this->dct_dc_y_past = 1024;
		this->dct_dc_cb_past = 1024;
		this->dct_dc_cr_past = 1024;
		throw "尚未支持 non intra"s;
	}

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
        RunLevel run_level = this->bit_reader.read_run_level(false);
        index = run_level.run;
		dct_zz[index] = run_level.level;
	}
	if (picture_coding_type != 4) {
		while (this->bit_reader.peek_bits(2) != 0b10) {
			RunLevel run_level = this->bit_reader.read_run_level(true);
			index = index + run_level.run + 1;
			if (index > 63) { throw "dct_coeff_next 不合理 index"s; }
			dct_zz[index] = run_level.level;
		}
		this->bit_reader.eat_bits(2);
	}
	// for (int i = 0; i < 8; i++) {
	// 	for (int j = 0; j < 8; j++) {
	// 		cout << dct_zz[i * 8 + j] << " ";
	// 	}
	// 	cout << endl;
	// }
	return shared_ptr<int>(dct_zz);
}
