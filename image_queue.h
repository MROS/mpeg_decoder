#ifndef MPEG_DECODER_PICTURE_H
#define MPEG_DECODER_PICTURE_H
#include <vector>

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <SFML/Graphics/Image.hpp>

struct ImageQueue {
	// NOTE: 或許能改成 unique_ptr ?
	std::queue<std::shared_ptr<sf::Image>> q;
	std::mutex mutex_lock;
	std::condition_variable cv;

	std::shared_ptr<sf::Image> pop();
	void push(std::shared_ptr<sf::Image> image);

};

#endif
