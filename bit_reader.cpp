#include <cassert>
#include <memory>
#include <iostream>
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

	uint bytes = ( length - (8 - this->bit_head) - 1 ) / 8 + 1;

	cout << "bytes: " << bytes << endl;

	// 因爲 get 會多塞一個 '\0' ，所以 bytes 要 + 1
	uint8_t *cs = new uint8_t(bytes + 1);
	this->file.get((char*)cs, bytes + 1);
	for (int i = 0; i < bytes; i++) {
		cout << std::hex << int(cs[i]) << std::dec << endl;
	}

	uint counter = length;

	// 讀取 current byte
	uint32_t ret = 0;
	for (int i = this->bit_head; i < 8; i++) {
		bool v = (1 << (7 - i)) & this->current_byte;
		cout << v ? 1 : 0;
		ret = (ret << 1u) + v;
	}

	counter -= (8 - this->bit_head);

	// 讀取偷窺的那幾個 byte
	int i = 0;
	this->bit_head = 0;
	this->current_byte = cs[0];
	while (counter > 0) {
//		cout << "i: " << i << endl;
//		cout << "counter: " << counter << endl;
//		cout << cur << endl;

		bool v = (1 << (7 - this->bit_head)) & this->current_byte;
		cout << v ? 1 : 0;
		ret = (ret << 1u) + v;

		this->bit_head++;
		if (this->bit_head == 8) {
			this->bit_head = 0;
			i++;
			this->current_byte = cs[i];
		}
		counter--;
	}

	if (eat && this->bit_head == 0) {
		this->current_byte = this->file.get();
	}

	if (!eat) {
		// 重置
		for (int i = bytes - 1; i >= 0; i--) {
			this->file.putback(cs[i]);
		}

		this->current_byte = original_current_byte;
		this->bit_head = original_bit_head;
	}

	delete cs;
	return ret;
}

uint32_t BitReader::eat_bits(uint length) {
	return this->read_bits(length, true);
}

uint32_t BitReader::peek_bits(uint length) {
	return this->read_bits(length, false);
}


