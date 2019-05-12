#include "play_video.h"

#include <stdint.h>

const uint32_t picture_start_code = 0x00000100;

const uint32_t slice_start_code_min = 0x00000101;
const uint32_t slice_start_code_max = 0x000001AF;

const uint32_t user_data_start_code = 0x000001B2;
const uint32_t sequence_header_code = 0x000001B3;
const uint32_t sequence_error_code = 0x000001B4;
const uint32_t extension_start_code = 0x000001B5;

const uint32_t sequence_end_code = 0x000001B7;
const uint32_t group_start_code = 0x000001B8;

const uint32_t system_start_code_min = 0x000001B9;
const uint32_t system_start_code_max = 0x000001FF;


int main()
{
	play_images();
	return 0;
}
