#include <cassert>
#include <memory>
#include <iostream>
#include <vector>
#include "bit_reader.h"

using namespace std;

BitReader::BitReader(std::ifstream &f) {
	this->file = std::move(f);
	this->bit_head = 0;
	this->current_byte = this->file.get();
}

uint32_t BitReader::read_bits(uint length, bool eat) {
	assert(length > 0);

	char original_current_byte = this->current_byte;
	char original_bit_head = this->bit_head;

	uint counter = length;

	vector<char> buf;

	uint32_t ret = 0;

	while (counter > 0) {
		bool v = (1 << (7 - this->bit_head)) & this->current_byte;
		cout << (v ? 1 : 0);
		ret = (ret << 1u) + v;

		this->bit_head++;
		if (this->bit_head == 8) {
			this->bit_head = 0;
			this->current_byte = this->file.get();
			buf.push_back(this->current_byte);
		}
		counter--;
	}

	if (!eat) {
		// 重置
		for (auto it = buf.rbegin(); it != buf.rend(); it++) {
			this->file.putback(*it);
		}

		this->current_byte = original_current_byte;
		this->bit_head = original_bit_head;
	}

	return ret;
}

uint32_t BitReader::eat_bits(uint length) {
	return this->read_bits(length, true);
}

uint32_t BitReader::peek_bits(uint length) {
	return this->read_bits(length, false);
}


