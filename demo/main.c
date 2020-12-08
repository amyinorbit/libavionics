#include <ccore/log.h>
#include <ccore/tpool.h>
#include <libavionics/display.h>
#include "app.h"
#include "gl.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include <ccore/math.h>
#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef M_PI
#define M_PI (3.14)
#endif
#define WIDTH (512)
#define HEIGHT (691)
#define RATIO ((float)HEIGHT/(float)WIDTH)

#define degrees(v) ((v) * 180.0/M_PI)
#define radians(v) ((v) * M_PI/180.0)

typedef struct {
    mfd_display_t *efis;
    bool stop;
    pthread_t efis_thread;
    unsigned tex;
    FT_Library ft_library;
    FT_Face ft_font;
    cairo_font_face_t *font;

    // cairo_path_t *wpt;
} app_data_t;

static void efis_frame(app_data_t *data) {
    if(!data->efis) return;

    cairo_t *cr = display_get_cairo(data->efis);

    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);


    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_identity_matrix(cr);
    cairo_rectangle(cr, 0, 0, WIDTH, HEIGHT);
    cairo_fill(cr);
    cairo_translate(cr, WIDTH/2, HEIGHT/2);

    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 1, 0.4, 1);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, 0, -100);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, 0, -100);
    cairo_line_to(cr, 50, -150);
    cairo_line_to(cr, 0, -200);
    cairo_stroke(cr);

    cairo_arc(cr, 0, 0, 300, -radians(135), -radians(45));
    cairo_stroke(cr);
    cairo_arc(cr, 0, 0, 150, radians(-180), 0);
    cairo_stroke(cr);

    cairo_move_to(cr, 12, 12);
    cairo_set_font_face(cr, data->font);
    cairo_set_font_size(cr, 16);
    cairo_show_text(cr, "ESKDO");
    cairo_fill(cr);

    display_finish_back_buffer(data->efis);
}

static void *efis_thread(void *user_data) {
    app_data_t *data = user_data;

    while(!data->stop) {
        efis_frame(data);
        sleep(1);
    }

    return NULL;
}


static void init(void *user_data) {
    app_data_t *data = user_data;
    data->efis = display_new(WIDTH, HEIGHT);
    data->stop = false;

    FT_Init_FreeType(&data->ft_library);
    FT_New_Face(data->ft_library, "Q4XP_Displays.ttf", 0, &data->ft_font);
    data->font = cairo_ft_font_face_create_for_ft_face(data->ft_font, 0);
    pthread_create(&data->efis_thread, NULL, efis_thread, data);
}

static void fini(void *user_data) {
    app_data_t *data = user_data;
    data->stop = true;
    pthread_join(data->efis_thread, NULL);

    cairo_font_face_destroy(data->font);
    FT_Done_Face(data->ft_font);
    FT_Done_FreeType(data->ft_library);

    display_delete(data->efis);
}

void update(double delta, const renderer_t *r, void *user_data) {
    app_data_t *data = user_data;
    display_upload(data->efis);
    render_quad(r, display_get_texture(data->efis), 0, 0, WIDTH, HEIGHT);
}

int main(int argc, const char **argv) {
    cc_set_log_name("avionics");
    ccpool_start(4);

    app_desc_t desc;
    desc.name = "Q4XP EFIS test";
    desc.width = WIDTH;
    desc.height = HEIGHT;
    desc.version_major = 4;
    desc.version_minor = 0;

    app_data_t data = {
        .efis = NULL,
        .stop = false
    };
    app_main(&desc, init, update, fini, &data);
    ccpool_stop();
}
