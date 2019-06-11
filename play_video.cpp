#include <cstdio>
#include <string>
#include <iostream>
#include <algorithm>
#include <thread>
#include <memory>
#include <chrono>
#include <SFML/Graphics.hpp>

#include "play_video.h"
#include "image_queue.h"
#include <SFML/Graphics/Image.hpp>

using namespace std;


void play_video(shared_ptr<ImageQueue> image_queue) {
	// TODO: 根據圖片大小改變視窗大小
	sf::RenderWindow window(sf::VideoMode(640, 360), "SFML works!");

	while (true) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
				return;
			}
		}

		shared_ptr<sf::Image> image = image_queue->pop();
		sf::Texture texture;
		texture.loadFromImage(*image);
		sf::Sprite sprite(texture);

		window.clear();
		window.draw(sprite);
		window.display();

		this_thread::sleep_for(chrono::milliseconds(1000 / 24));
	}

}

