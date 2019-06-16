#include <thread>
#include <memory>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

#include "play_video.h"
#include "image_queue.h"

using namespace std;

void play_video(shared_ptr<ImageQueue> image_queue) {
	sf::RenderWindow window(sf::VideoMode(1080, 240), "mpeg-1 播放器");

	while (true) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
				exit(0);
			}
		}

		shared_ptr<sf::Image> image = image_queue->pop();
		// window.setSize(image->getSize());
		sf::Texture texture;
		texture.loadFromImage(*image);
		sf::Sprite sprite(texture);

		window.clear();
		window.draw(sprite);
		window.display();

		this_thread::sleep_for(chrono::milliseconds(1000 / 24));
	}

}

