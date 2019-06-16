#include "image_queue.h"
#include <memory>

using namespace std;

shared_ptr<sf::Image> ImageQueue::pop() {
    unique_lock<mutex> lock(this->mutex_lock);
    while (this->q.empty()) {
		this->cv.wait(lock);
    }
    auto ret = this->q.front();
    this->q.pop();
	return ret;
}

void ImageQueue::push(const shared_ptr<sf::Image>& image) {
	lock_guard<mutex> lock(this->mutex_lock);
	this->q.push(image);
	this->cv.notify_one();
}
