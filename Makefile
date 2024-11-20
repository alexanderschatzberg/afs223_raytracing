# CFLAGS=-Wall -Wpedantic -Werror -Wshadow -Wformat=2 -std=c17 -lm -fsanitize=address,undefined -g
CFLAGS=-Wall -Wpedantic -Werror -Wshadow -Wformat=2 -std=c17 -lm
CC=gcc
RAYCAST_CORE=raycaster_util.c image.c
TEST_DIRS=images/sequential_results images/parallel_light_results images/parallel_row_results

raycaster: $(RAYCAST_CORE) main.c raycaster.c
	$(CC) $(CFLAGS) $^ -o $@

test_raycaster_util: $(RAYCAST_CORE) test_raycaster_util.c
	$(CC) $(CFLAGS) $^ -o $@

test_raycaster: $(RAYCAST_CORE) raycaster.c test_raycaster.c
	mkdir -p $(TEST_DIRS)
	$(CC) $(CFLAGS) $^ -o $@

timing: $(RAYCAST_CORE) timing.c raycaster.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(TEST_DIRS)
	rm -f raycaster test_raycaster_util test_raycaster timing
	rm -f *.o
	rm -f raycast.png
