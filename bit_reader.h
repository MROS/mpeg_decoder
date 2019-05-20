#ifndef MPEG_DECODER_BITREADER_H
#define MPEG_DECODER_BITREADER_H

#include <fstream>
#include <stdint.h>

class BitReader {
private:
	std::ifstream file;
	uint bit_head;      // bit 層級的讀寫頭，取值在 0 ~ 7
	char current_byte;
public:
	explicit BitReader(std::ifstream &f);

	uint32_t read_bits(uint length, bool eat);
	// 只預看，不移動讀寫頭
	uint32_t peek_bits(uint length);
	// 讀取並移動
	uint32_t eat_bits(uint length);
};


#endif //MPEG_DECODER_BITREADER_H
