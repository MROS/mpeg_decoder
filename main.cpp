#include <iostream>
#include <fstream>
#include <stdio.h>
#include <thread>

#include "image_queue.h"
#include "play_video.h"
#include "decoder.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 2) {
		cout << "用法： ./mpeg-decoder <mpeg file>" << endl;
		return 1;
	}

	shared_ptr<ImageQueue> image_queue = make_shared<ImageQueue>();

	// 開啓播放器圖形界面的執行緒
	thread player_thread(play_video, image_queue);

	// 主執行緒負責解碼
	ifstream mpeg_file(argv[1], ios::in | ios::binary);
	if (mpeg_file.is_open()) {
		Decoder decoder(mpeg_file, image_queue);
		try {
			decoder.start();
		} catch (string &e) {
			cerr << e << endl;
		}
	} else {
		cout << "無法開啓檔案：" << argv[1] << endl;
		return 1;
	}

	player_thread.join();
	return 0;
}
