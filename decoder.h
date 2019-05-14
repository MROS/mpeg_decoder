#ifndef MPEG_DECODER_DECODER_H
#define MPEG_DECODER_DECODER_H

#include <fstream>

class Decoder {
private:
	std::ifstream file;
public:
	Decoder(std::ifstream &f): file(std::move(f))
	{};
	void play();
};

#endif //MPEG_DECODER_DECODER_H
