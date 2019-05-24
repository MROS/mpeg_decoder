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
	int step = 0;
	uint32_t ret = 0;

	while (counter > 0) {
		bool v = (1 << (7 - this->bit_head)) & this->current_byte;
		ret = (ret << 1u) + v;

		this->bit_head++;
		if (this->bit_head == 8) {
			this->bit_head = 0;
			this->current_byte = this->file.get();
			step++;
		}
		counter--;
	}

	if (!eat) {
		// 重置
		this->file.seekg(-step, this->file.cur);

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

void BitReader::next_start_code() {
	if (this->bit_head != 0) {
		this->bit_head = 0;
		this->current_byte = this->file.get();
		cout << "前進 1 byte" << endl;
	}
	while (this->peek_bits(24) != 1) {
		this->current_byte = this->file.get();
		cout << "前進 1 byte" << endl;
	}
}

void BitReader::show_head() {
    cout << "current_byte: " << int(this->current_byte) << endl;
	cout << "bit_head: " << this->bit_head << endl;
	cout << "file position: " << this->file.tellg() << endl;
}

int32_t BitReader::read_vlc(VlcTable table) {
	int len = table.min_len;
    uint32_t code = this->eat_bits(table.min_len);

    while (len <= table.max_len) {
    	if (code <= table.max_code[len] && code >= table.min_code[len]) {
			return table.table[len][code - table.min_code[len]];
    	}

		bool v = (1 << (7 - this->bit_head)) & this->current_byte;
    	code = (code << 1) + v;
		len++;
		this->bit_head++;
		if (this->bit_head == 8) {
			this->bit_head = 0;
			this->current_byte = this->file.get();
		}
    }

	return 0;
}


