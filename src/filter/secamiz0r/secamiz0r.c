
#define LIBSECAM_IMPLEMENTATION

#include <stddef.h>
#include "frei0r.h"
#include "libsecam.h"

struct secamiz0r
{
    libsecam_t *libsecam;
    unsigned int width;
    unsigned int height;
    double intensity;
};

static void update_libsecam_options(libsecam_t *libsecam, double intensity)
{
    libsecam_options_t *options = libsecam_options(libsecam);

    if (intensity < 0.25) {
        double const mul = intensity * 4.0;

        options->luma_noise_factor = 0.07 * mul;
        options->luma_fire_factor = 0.005 * mul;
        options->luma_loss_chance = 0.02 * mul;
        options->chroma_shift_chance = 9.0 * mul;
        options->chroma_noise_factor = 0.25 * mul;
        options->chroma_fire_factor = 0.1 * mul;
        options->chroma_loss_chance = 0.03 * mul;
        options->echo_offset = (int) ceilf(4.0 * mul);
        options->horizontal_instability = (intensity < 0.0625) ? 0 : 2;
    } else {
        double const mul = (intensity - 0.25) / 0.75;

        options->luma_noise_factor = 0.07 + (2.0 - 0.07) * mul;
        options->luma_fire_factor = 0.005 + (0.5 - 0.005) * mul;
        options->luma_loss_chance = 0.02 + (2.0 - 0.02) * mul;
        options->chroma_shift_chance = 9.0 + (40.0 - 9.0) * mul;
        options->chroma_noise_factor = 0.25 + (1.0 - 0.25) * mul;
        options->chroma_fire_factor = 0.1 + (10.0 - 0.1) * mul;
        options->chroma_loss_chance = 0.03 + (2.0 - 0.03) * mul;
        options->echo_offset = (int) ceilf(4.0 + (20.0 - 4.0) * mul);
        options->horizontal_instability = (int) ceilf(2.0 + (32.0 - 2.0) * mul);
    }
}

int f0r_init(void)
{
    return 1;
}

void f0r_deinit(void)
{
}

void f0r_get_plugin_info(f0r_plugin_info_t *info)
{
    info->name = "secamiz0r";
    info->author = "tuorqai";
    info->plugin_type = F0R_PLUGIN_TYPE_FILTER;
    info->color_model = F0R_COLOR_MODEL_RGBA8888;
    info->frei0r_version = FREI0R_MAJOR_VERSION;
    info->major_version = 1;
    info->minor_version = 0;
    info->num_params = 1;
    info->explanation = "SECAM Fire effect";
}

void f0r_get_param_info(f0r_param_info_t *info, int index)
{
    switch (index) {
    case 0:
        info->name = "Intensity";
        info->type = F0R_PARAM_DOUBLE;
        info->explanation = NULL;
        break;
    default:
        break;
    }
}

f0r_instance_t f0r_construct(unsigned int width, unsigned int height)
{
    struct secamiz0r *instance = calloc(1, sizeof(*instance));

    if (!instance) {
        return 0;
    }

    instance->libsecam = libsecam_init(width, height);

    if (!instance->libsecam) {
        free(instance);
        return 0;
    }

    instance->width = width;
    instance->height = height;
    instance->intensity = 0.25;

    update_libsecam_options(instance->libsecam, instance->intensity);

    return instance;
}

void f0r_destruct(f0r_instance_t instance)
{
    struct secamiz0r *secamiz0r = instance;

    libsecam_close(secamiz0r->libsecam);
    free(secamiz0r);
}

void f0r_set_param_value(f0r_instance_t instance, f0r_param_t param, int index)
{
    struct secamiz0r *secamiz0r = instance;

    switch (index) {
    case 0:
        secamiz0r->intensity = *((double *) param);
        break;
    }

    update_libsecam_options(secamiz0r->libsecam, secamiz0r->intensity);
}

void f0r_get_param_value(f0r_instance_t instance, f0r_param_t param, int index)
{
    struct secamiz0r *secamiz0r = instance;

    switch (index) {
    case 0:
        *((double *) param) = secamiz0r->intensity;
        break;
    }
}

void f0r_update(f0r_instance_t instance, double time, uint32_t const *input, uint32_t *output)
{
    struct secamiz0r *secamiz0r = instance;

    libsecam_filter_to_buffer(secamiz0r->libsecam,
        (unsigned char const *) input,
        (unsigned char *) output);
}

