#ifndef MPEG_DECODER_BITREADER_H
#define MPEG_DECODER_BITREADER_H

#include <fstream>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include "util.h"

class RunLevel {
public:
	int run, level;
	void from_vector(std::vector<int> &source) {
		this->run = source[0];
		this->level = source[1];
	}
};

class IntWrap {
public:
	int value;
	void from_vector(std::vector<int> &source) {
		this->value = source[0];
	}
};

class MacroblockType {
public:
	int quant, motion_forward, motion_backward, pattern, intra;
	void from_vector(std::vector<int> &source) {
		this->quant = source[0];
		this->motion_forward = source[1];
		this->motion_backward = source[2];
		this->pattern = source[3];
		this->intra = source[4];
	}
};

template <typename T>
class VlcTable {
public:
	int min_len, max_len;
	std::vector<std::vector<T>> table;
	std::vector<int> min_code;
	VlcTable() {};
	explicit VlcTable(std::string filename);
};

struct Code {
	std::string key;
	std::vector<int> value;
};

template<typename T>
VlcTable<T>::VlcTable(std::string filename) {
	std::cout << "read file: " << filename << std::endl;
	std::ifstream file(filename);
	std::string line;

	std::vector<Code> codes;

	while (getline(file, line)) {
		auto ss = split(line);
		struct Code pair;
		pair.key = ss[0];
		std::vector<int> value;
		for (auto i = 1; i < ss.size(); i++) {
			value.push_back(std::stoi(ss[i]));
		}
        pair.value = value;
		codes.push_back(pair);
	}

	std::sort(codes.begin(), codes.end(),
			[](Code &a, Code &b) {
		if (a.key.length() == b.key.length()) {
			return std::stoi(a.key) < std::stoi(b.key);
		} else {
			return a.key.length() < b.key.length();
		}
	});

	this->min_len = codes[0].key.length();
	this->max_len = codes[codes.size() - 1].key.length();

	std::cout << "min_len: " << this->min_len << ", max_len: " << this->max_len << std::endl;

	this->table = std::vector<std::vector<T>>(this->max_len + 1);
	this->min_code = std::vector<int>(this->max_len + 1);


	for (auto i = 0; i < codes.size(); i++) {
		int len = codes[i].key.length();
//		std::cout << "len: " << len << ", key: " << codes[i].key << std::endl;
//		std::cout << "value length: " << codes[i].value.size() << std::endl;
		int prev_len = codes[i - 1].key.length();
		if (i == 0 || len > prev_len) {
			this->min_code[len] = std::stoi(codes[i].key, nullptr, 2);
		}
		T v;
		v.from_vector(codes[i].value);
		this->table[len].push_back(v);
	}
}

class BitReader {
private:
	std::ifstream file;
	uint bit_head;      // bit 層級的讀寫頭，取值在 0 ~ 7
	char current_byte;

public:
	VlcTable<MacroblockType> b_macroblock_type, intra_macroblock_type, p_macroblock_type;
	VlcTable<IntWrap> dct_dc_size_chrominance, dct_dc_size_luminance, motion_vector, coded_block_pattern, macroblock_addr;
	VlcTable<RunLevel> run_level;

	explicit BitReader(std::ifstream &f);

	BitReader();

	uint32_t read_bits(uint length, bool eat);
	// 只預看，不移動讀寫頭
	uint32_t peek_bits(uint length);
	// 讀取並移動
	uint32_t eat_bits(uint length);

	void show_head();

	void next_start_code();

	template <typename T>
	T read_vlc(VlcTable<T> &table);
};


#endif
