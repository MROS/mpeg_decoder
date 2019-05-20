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
	uint32_t a = this->bit_reader.peek_bits(31);
	cout << "read: " << a << endl;
	a = this->bit_reader.peek_bits(32);
	cout << "read: " << a << endl;
	a = this->bit_reader.peek_bits(32);
	cout << "read: " << a << endl;
	a = this->bit_reader.peek_bits(32);
	cout << "read: " << a << endl;
	this->video_sequence();
}

void Decoder::video_sequence() {
/*	do {
		this->sequence_header();
		do {
			this->group_of_pictures();
		}
	}*/
}

//void Decoder::sequence_header() {
//
//}
//
//void Decoder::group_of_pictures() {
//
//}
