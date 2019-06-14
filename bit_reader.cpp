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
	this->p_macroblock_type = VlcTable<MacroblockType>("../huffman_tables/p_macroblock_type.txt");
	this->intra_macroblock_type = VlcTable<MacroblockType>("../huffman_tables/intra_macroblock_type.txt");
	this->b_macroblock_type = VlcTable<MacroblockType>("../huffman_tables/b_macroblock_type.txt");

	this->motion_vector = VlcTable<IntWrap>("../huffman_tables/motion_vector.txt");
	this->dct_dc_size_chrominance = VlcTable<IntWrap>("../huffman_tables/dct_dc_size_chrominance.txt");
	this->dct_dc_size_luminance = VlcTable<IntWrap>("../huffman_tables/dct_dc_size_luminance.txt");
	this->coded_block_pattern = VlcTable<IntWrap>("../huffman_tables/coded_block_pattern.txt");
	this->macroblock_addr = VlcTable<IntWrap>("../huffman_tables/macroblock_addr.txt");

	this->run_level = VlcTable<RunLevel>("../huffman_tables/run_level.txt");
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

template <typename T>
T BitReader::read_vlc(VlcTable<T> &table) {
	int len = table.min_len;
    uint32_t code = this->eat_bits(table.min_len);

    while (len <= table.max_len) {
//		cout << "len: " << len << ", code: " << code << endl;
    	if (code - table.min_code[len] < table.table[len].size()) {
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

    throw "無法匹配霍夫曼表"s;

//	return 0;
}

template IntWrap BitReader::read_vlc(VlcTable<IntWrap> &table);
template MacroblockType BitReader::read_vlc(VlcTable<MacroblockType> &table);
template RunLevel BitReader::read_vlc(VlcTable<RunLevel> &table);

RunLevel BitReader::read_run_level(bool coeff_next) {
	if (this->peek_bits(6) == 1) { // fixed length ，見 Table 2-B.5g
		this->eat_bits(6);
		RunLevel ret;
		ret.run = this->eat_bits(6);
		int tmp = this->eat_bits(8);
		if (tmp == 0b00000000) {
			// cout << "128 ~ 255" << endl;
			ret.level = this->eat_bits(8);
		} else if (tmp == 0b10000000) {
			// cout << "-128 ~ -256" << endl;
			ret.level = (int)this->eat_bits(8) - 256;
		} else {
			ret.level = tmp > 128 ? tmp - 256: tmp;
		}
		return ret;
	} else {
		RunLevel ret = this->read_vlc(this->run_level);
		// TODO: fixed length
		if (ret.run == 0 && ret.level == 1 && coeff_next) { // 怪異規定。Table 2-B.5c NOTE2, 3
			this->eat_bits(1);
		}
		uint32_t s = this->eat_bits(1);
		if (s == 1) { ret.level = -ret.level; }
		return ret;
	}
}