#include <bits/time.h>
#include <stdatomic.h>
#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <pthread.h>

#define CHECK(x, msg) do { if (!(x)) { perror(msg); return NULL; } } while(0)
#define HOT_PLUG_PIN 27
#define SDA_PIN 17
#define SCL_PIN 22

static struct gpiod_line_request *hot_plug_request;
static struct gpiod_line_request *sda_request;
static struct gpiod_line_request *scl_request;

atomic_int last_scl = -1;

void set_pin(int on, unsigned int pin) {
    gpiod_line_request_set_value(hot_plug_request, HOT_PLUG_PIN, on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

void *pin_sda_reader_thread(void *arg) {
    (void)arg;

    int last_sda = -1;

    for(;;) {
        int val = gpiod_line_request_get_value(sda_request, SDA_PIN);
        if (val != -1) {
            int scl = atomic_load(&last_scl);
            if (last_sda == 1 && val == 0 && scl == 1) {
                printf("START!\n");
            }

            last_sda = val;
        }

        usleep(10000);
    }
}

void *pin_scl_reader_thread(void *arg) {
    (void)arg;

    for(;;) {
        int val = gpiod_line_request_get_value(scl_request, SCL_PIN);
        if (val != -1) {
            atomic_store(&last_scl, val);
        }

        usleep(10000);
    }
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
    hot_plug_request = init_pin("/dev/gpiochip0", HOT_PLUG_PIN, 1);
    sda_request = init_pin("/dev/gpiochip0", SDA_PIN, 0);
    scl_request = init_pin("/dev/gpiochip0", SCL_PIN, 0);
    if (!hot_plug_request || !sda_request || !scl_request) return 1;

    pthread_t reader;
    pthread_create(&reader, NULL, pin_sda_reader_thread, NULL);
    pthread_create(&reader, NULL, pin_scl_reader_thread, NULL);
    sleep(1);

    set_pin(0, HOT_PLUG_PIN);
    sleep(1);
    set_pin(1, HOT_PLUG_PIN);
    sleep(1);

    gpiod_line_request_release(hot_plug_request);
    gpiod_line_request_release(sda_request);
    gpiod_line_request_release(scl_request);
    return 0;
}
