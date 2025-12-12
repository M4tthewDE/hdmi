#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#define CHECK(x, msg) do { if (!(x)) { perror(msg); return NULL; } } while(0)
#define OUT_PIN 27
#define IN_PIN 17

static struct gpiod_line_request *out_request;
static struct gpiod_line_request *in_request;

static atomic_int pin_value = 0;
static atomic_int running = 1;

void set_pin(int on) {
    gpiod_line_request_set_value(out_request, OUT_PIN, on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

int get_pin(void) {
    return atomic_load(&pin_value);
}

void *pin_reader_thread(void *arg) {
    (void)arg;
    while (atomic_load(&running)) {
        int val = gpiod_line_request_get_value(in_request, IN_PIN);
        //printf("val: %d\n", val);
        if (val == 0) {
            printf("SUCCESS: %d\n", val);
        }
        atomic_store(&pin_value, val);
        usleep(100);  // 1ms polling interval - adjust as needed
    }
    return NULL;
}

struct gpiod_line_request *init_pin(const char *chip_path, unsigned pin, int output) {
    struct gpiod_chip *chip = gpiod_chip_open(chip_path);
    CHECK(chip, "chip");
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    CHECK(settings, "settings");
    gpiod_line_settings_set_direction(settings, output ? GPIOD_LINE_DIRECTION_OUTPUT : GPIOD_LINE_DIRECTION_INPUT);
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    CHECK(line_cfg, "line_cfg");
    gpiod_line_config_add_line_settings(line_cfg, &pin, 1, settings);
    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    CHECK(req_cfg, "req_cfg");
    gpiod_request_config_set_consumer(req_cfg, "gpio-util");
    return gpiod_chip_request_lines(chip, req_cfg, line_cfg);
}

int main() {
    out_request = init_pin("/dev/gpiochip0", OUT_PIN, 1);
    in_request = init_pin("/dev/gpiochip0", IN_PIN, 0);
    if (!out_request || !in_request) return 1;

    pthread_t reader;
    pthread_create(&reader, NULL, pin_reader_thread, NULL);
    sleep(1);

    set_pin(0);
    sleep(1);
    set_pin(1);
    sleep(5);

    // Example: read the background-updated value
    //printf("Current IN_PIN value: %d\n", get_pin());

    atomic_store(&running, 0);
    pthread_join(reader, NULL);

    gpiod_line_request_release(out_request);
    gpiod_line_request_release(in_request);
    return 0;
}
