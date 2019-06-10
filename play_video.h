#ifndef MPEG_DECODER_PLAY_VIDEO_H
#define MPEG_DECODER_PLAY_VIDEO_H

#include <string>
#include <vector>
#include <memory>
#include "image_queue.h"

void play_video(std::shared_ptr<ImageQueue> image_queue);

#endif
