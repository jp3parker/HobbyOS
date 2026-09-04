#include <stdint.h>

#include <kernel/donut.h>
#include <kernel/renderer.h>
#include <kernel/timer.h>

void donut_demo_run(void) {
	float angle = 0.0f;
	uint32_t next_frame = timer_ticks();

	renderer_initialize();
	for (;;) {
		renderer_render(angle, angle * 0.71f);
		angle += 0.035f;
		next_frame += 2U;
		timer_wait_until(next_frame);
		if ((int32_t)(next_frame - timer_ticks()) < -2) {
			next_frame = timer_ticks();
		}
	}
}
