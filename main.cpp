#include <iostream>
#include <fstream>
#include <stdio.h>

#include "play_video.h"
#include "decoder.h"


using namespace std;


int main(int argc, char *argv[])
{
	if (argc != 2) {
		cout << "用法： ./mpeg-decoder <mpeg file>" << endl;
		return 1;
	}

	ifstream mpeg_file(argv[1], ios::in | ios::binary);
	if (mpeg_file.is_open()) {
		Decoder decoder(mpeg_file);
		decoder.play();
		return 0;
	} else {
		cout << "無法開啓檔案：" << argv[1] << endl;
		return 1;
	}

//	play_images();
}
