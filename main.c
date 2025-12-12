#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>

#define CHECK(x, msg) do { if (!(x)) { perror(msg); return NULL; } } while(0)
#define PIN 27

static struct gpiod_line_request *request;

void set_pin(int on) {
    gpiod_line_request_set_value(request, PIN, on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

struct gpiod_line_request *init_pin(const char *chip_path, unsigned pin) {
    struct gpiod_chip *chip = gpiod_chip_open(chip_path);
    CHECK(chip, "chip");

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    CHECK(settings, "settings");
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    CHECK(line_cfg, "line_cfg");
    gpiod_line_config_add_line_settings(line_cfg, &pin, 1, settings);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    CHECK(req_cfg, "req_cfg");
    gpiod_request_config_set_consumer(req_cfg, "line-toggle");

    return gpiod_chip_request_lines(chip, req_cfg, line_cfg);
}

int main() {
    request = init_pin("/dev/gpiochip0", PIN);
    if (!request) return 1;

    for (int i = 0; i < 10; i++) {
        set_pin(1);
        usleep(100000);
        set_pin(0);
        usleep(100000);
    }

    gpiod_line_request_release(request);
    return 0;
}
