#include <cstdio>
#include <string>
#include <iostream>
#include <dirent.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <SFML/Graphics.hpp>

using namespace std;

vector<string> dir_list(string dir_name) {
	vector<string> ret;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(dir_name.c_str())) != nullptr) {
		while ((ent = readdir(dir)) != nullptr) {
			ret.emplace_back(ent->d_name);
		}
		closedir(dir);
		sort(ret.begin(), ret.end());
		return ret;
	} else {
		perror("");
        throw "can't open dir";
	}
}

int main()
{

	sf::RenderWindow window(sf::VideoMode(640, 360), "SFML works!");

	auto images = dir_list("../images");

	for (int i = 2; i < images.size() && window.isOpen(); i++) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		string image = images[i];
		sf::Texture texture;
		if (!texture.loadFromFile("../images/" + image)) {
            cout << "無法開啓檔案: " << image << endl;
		}
		sf::Sprite sprite(texture);

		window.clear();
		window.draw(sprite);
		window.display();

		this_thread::sleep_for(chrono::milliseconds(1000 / 24));
	}
	window.close();

	return 0;
}
