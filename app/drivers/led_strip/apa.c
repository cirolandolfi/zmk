/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @brief APA LED Strip Interface
 * @defgroup apa_led_strip_interface APA LED Strip Interface
 * @ingroup io_interfaces
 * @{
 */


#include <device.h>
#include <logging/log.h>

#include <drivers/led_strip.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define DT_DRV_COMPAT zmk_apa_led_strip

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

typedef int (*apa_led_api_update_rgb)(const struct device *dev,
				  struct led_rgb *pixels,
				  size_t num_pixels,
				  const uint8_t * points,
				  uint8_t num_points
		          );

struct apa_led_strip_driver_api {
	apa_led_api_update_rgb update_rgb;
};


struct apa_led_strip_data {
    struct led_rgb * pixels_buffer;
    uint8_t num_pixels;
    const struct device *led_strip;
};

static int apa_led_strip_update_rgb (  const struct device * apa_device,
				                struct led_rgb *pixels,
				                size_t num_pixels,
                                const uint8_t * points,
                                uint8_t num_points ) {
    struct apa_led_strip_data * apa_data = (struct apa_led_strip_data * ) apa_device->data;
    const struct led_strip_driver_api *api =
		(const struct led_strip_driver_api *)apa_data->led_strip->api;

    for ( int pixel_index = 0;
        pixel_index < num_pixels && pixel_index < num_points;
        pixel_index ++ ) {
        if ( points[pixel_index] < apa_data->num_pixels ) {
            apa_data->pixels_buffer[points[pixel_index]]=
                pixels[pixel_index];
        }
    }
    return api->update_rgb(apa_data->led_strip, apa_data->pixels_buffer, apa_data->num_pixels);
}

static int apa_led_strip_init(const struct device *_arg) {
    return 0;
}
static const struct apa_led_strip_driver_api apa_driver_api = {
	.update_rgb = apa_led_strip_update_rgb,
};

#define RGB_APA_LED_STRIP_INIT(inst)                                                                             \
    static struct led_rgb pixels_buffer_##inst[DT_INST_PROP_BY_PHANDLE(inst, led_strip, chain_length)];          \
    static struct apa_led_strip_data apa_led_strip_device_##inst##_data = {                                      \
        .pixels_buffer = pixels_buffer_##inst,                                                                   \
        .num_pixels = DT_INST_PROP_BY_PHANDLE(inst, led_strip, chain_length),                                    \
        .led_strip = DEVICE_DT_GET(DT_INST_PHANDLE(inst, led_strip))                                             \
    };                                                                                                           \
    DEVICE_DT_INST_DEFINE(inst, apa_led_strip_init, device_pm_control_nop,                                       \
                      &apa_led_strip_device_##inst##_data, NULL, APPLICATION,                                    \
                      CONFIG_APPLICATION_INIT_PRIORITY, &apa_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RGB_APA_LED_STRIP_INIT);

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT zmk_apa_led_strip_subs

struct apa_led_strip_subs_config_data {
    const uint8_t * points;
    uint8_t num_of_points;
    const struct device * apa_led_strip;
};

static int apa_led_strip_subs_update_rgb ( const struct device * dev,
				                struct led_rgb *pixels,
				                size_t num_pixels ) {
    const struct apa_led_strip_subs_config_data * cfg_data = (const struct apa_led_strip_subs_config_data *) dev->config;
    const struct device * apa_dev = cfg_data->apa_led_strip;
    const struct apa_led_strip_driver_api * api = (const struct apa_led_strip_driver_api * )apa_dev->api;
    return api->update_rgb(apa_dev, pixels, num_pixels, cfg_data->points, cfg_data->num_of_points);
}

static int apa_led_strip_subs_update_channels(const struct device *dev,
					uint8_t *channels,
					size_t num_channels)
{
	LOG_ERR("update_channels not implemented");
	return -ENOTSUP;
}

static const struct led_strip_driver_api apa_subs_driver_api = {
	.update_rgb = apa_led_strip_subs_update_rgb,
	.update_channels = apa_led_strip_subs_update_channels,
};

static int apa_led_strip_subs_init(const struct device *_arg) {
    return 0;
}

#define RGB_APA_LED_STRIP_SUBS_INIT(inst)                                                                        \
    static const uint8_t points_##inst##_data[DT_INST_PROP_LEN(inst, points)]=                                   \
        DT_INST_PROP(inst, points);                                                                              \
    static const struct apa_led_strip_subs_config_data apa_led_strip_subs_config_##inst##_data= {                \
        .points = points_##inst##_data,                                                                          \
        .num_of_points = DT_INST_PROP_LEN(inst, points),                                                         \
        .apa_led_strip = DEVICE_DT_GET(DT_PARENT(DT_DRV_INST(inst)))                                             \
    };                                                                                                           \
    DEVICE_DT_INST_DEFINE(inst, apa_led_strip_subs_init, device_pm_control_nop,                                  \
                      NULL, &apa_led_strip_subs_config_##inst##_data, APPLICATION,                               \
                      CONFIG_APPLICATION_INIT_PRIORITY, &apa_subs_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RGB_APA_LED_STRIP_SUBS_INIT);


#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
/**
 * @}
 */