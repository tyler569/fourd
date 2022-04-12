#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <cairo.h>
#include <SDL.h>

const int window_w = 640;
const int window_h = 480;

// We want:
// window vertical to be -2 to 2
// window horizontal to be square to that
// center of window to be 0, 0

const double eyemaxh = 2.0;

const double factor = window_h/(eyemaxh*2);
const double xmin = eyemaxh * ((double)window_w / window_h);

double speed = 1;
int paused = 0;
bool labels = false;
bool axes = false;

double d1 = 0, d2 = 0, d3 = 0, d4 = 0, d5 = 0, d6 = 0;
double t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0;

long nanosecond_now() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long)now.tv_sec * 1000000000 + now.tv_nsec;
}

long millisecond_now() {
    return nanosecond_now() / 1000000;
}

long max(long a, long b) {
    return a >= b ? a : b;
}

void render_frame(cairo_t *cr, int frame);

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    bool done = false;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
            "Rocket",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            window_w,
            window_h,
            SDL_WINDOW_SHOWN
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
    );

    int window_width;
    int window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);

    printf("window is %ix%i\n", window_width, window_height);

    int renderer_width;
    int renderer_height;
    SDL_GetRendererOutputSize(renderer, &renderer_width, &renderer_height);

    printf("renderer is %ix%i\n", renderer_width, renderer_height);

    SDL_Surface *sdl_surface = SDL_CreateRGBSurface(
            0,
            renderer_width,
            renderer_height,
            32,
            0x00FF0000,
            0x0000FF00,
            0x000000FF,
            0
    );

    printf("surface is %ix%i, pitch %i\n",
            sdl_surface->w, sdl_surface->h, sdl_surface->pitch);

    cairo_surface_t *cairo_surface = cairo_image_surface_create_for_data(
            (unsigned char *)sdl_surface->pixels,
            CAIRO_FORMAT_RGB24,
            sdl_surface->w,
            sdl_surface->h,
            sdl_surface->pitch
    );

    cairo_t *cr = cairo_create(cairo_surface);

    cairo_scale(cr, factor, factor);
    cairo_translate(cr, xmin, eyemaxh);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_Texture *texture;

    int frame = 0;
    long frame_start_ms;
    int desired_frame_rate = 60;
    long desired_frame_duration = 1000 / desired_frame_rate;

    while (!done) {
        frame_start_ms = millisecond_now();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                done = true;
                break;
            case SDL_KEYDOWN: {
                SDL_Keymod modstate = SDL_GetModState();
                bool ctrl = modstate & KMOD_CTRL;
                bool shft = modstate & KMOD_SHIFT;
                switch (event.key.keysym.sym) {
                case SDLK_c:
                    if (ctrl)
                        done = true;
                    else
                        if (shft)
                            d3 = -1;
                        else
                            d3 = 1;
                    break;
                case SDLK_a:
                    if (shft)
                        d1 = -1;
                    else
                        d1 = 1;
                    break;
                case SDLK_b:
                    if (shft)
                        d2 = -1;
                    else
                        d2 = 1;
                    break;
                case SDLK_d:
                    if (shft)
                        d4 = -1;
                    else
                        d4 = 1;
                    break;
                case SDLK_e:
                    if (shft)
                        d5 = -1;
                    else
                        d5 = 1;
                    break;
                case SDLK_f:
                    if (shft)
                        d6 = -1;
                    else
                        d6 = 1;
                    break;
                case SDLK_0:
                    t1 = t2 = t3 = t4 = t5 = t6 = 0;
                    break;
                case SDLK_UP:
                    d4 = -1;
                    break;
                case SDLK_DOWN:
                    d4 = 1;
                    break;
                case SDLK_LEFT:
                    d2 = -1;
                    break;
                case SDLK_RIGHT:
                    d2 = 1;
                    break;
                case SDLK_l:
                    labels = !labels;
                    break;
                case SDLK_k:
                    axes = !axes;
                    break;
                }
                break;
            }
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_a:
                    d1 = 0;
                    break;
                case SDLK_b:
                    d2 = 0;
                    break;
                case SDLK_c:
                    d3 = 0;
                    break;
                case SDLK_d:
                    d4 = 0;
                    break;
                case SDLK_e:
                    d5 = 0;
                    break;
                case SDLK_f:
                    d6 = 0;
                    break;
                case SDLK_UP:
                case SDLK_DOWN:
                    d4 = 0;
                    break;
                case SDLK_LEFT:
                case SDLK_RIGHT:
                    d2 = 0;
                    break;
                }
                break;
            }
        }

        render_frame(cr, frame);

        texture = SDL_CreateTextureFromSurface(renderer, sdl_surface);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(texture);

        long frame_duration = millisecond_now() - frame_start_ms;

        frame++;
        SDL_Delay(max(desired_frame_duration - frame_duration, 0));
    }

    SDL_FreeSurface(sdl_surface);

    cairo_destroy(cr);
    cairo_surface_destroy(cairo_surface);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

// ======================================================================= //

struct matrix {
    int h, w;
    double v[36];
};
#define V(m, x, y) (m).v[(y)*(m).h+(x)]

void print(struct matrix);

struct matrix mul(struct matrix a, struct matrix b) {
    if (a.w != b.h) {
        printf("Invalid matrix multiplication:\n");
        print(a);
        printf("*\n");
        print(b);
        exit(1);
    }
    struct matrix out = { a.h, b.w, {} };
    for (int col = 0; col < b.h; col++) {
        for (int row = 0; row < a.w; row++) {
            double product = 0;
            for (int dp = 0; dp < a.h; dp++) {
                product += V(a, row, dp)*V(b, dp, col);
            }
            V(out, row, col) = product;
        }
    }
    return out;
}

struct matrix identity(int n) {
    struct matrix out = { n, n, {} };
    for (int i = 0; i < n; i++) {
        V(out, n, n) = 1;
    }
    return out;
}

void print(struct matrix a) {
    for (int x = 0; x < a.h; x++) {
        printf("[");
        for (int y = 0; y < a.w; y++) {
            printf("%g ", V(a, x, y));
        }
        printf("]\n");
    }
}

struct point {
    union {
        struct { double x, y, z, w; };
        double xyzw[4];
    };
};

struct point makepoint(double x, double y, double z, double w) {
    return (struct point){{{ x, y, z, w }}};
}

struct point pmul(struct point p, struct matrix m) {
    struct matrix mp = { 4, 1, { p.x, p.y, p.z, p.w } };
    struct matrix out = mul(m, mp);
    struct point np = makepoint(out.v[0], out.v[1], out.v[2], out.v[3]);
    return np;
}

struct matrix rotation(int ax1, int ax2, double factor) {
    struct matrix rotation = { 4, 4, {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    } };

    V(rotation, ax1, ax1) = cos(factor);
    V(rotation, ax1, ax2) = sin(factor);
    V(rotation, ax2, ax1) = -sin(factor);
    V(rotation, ax2, ax2) = cos(factor);

    return rotation;
}

struct point rotate(struct point in, [[maybe_unused]] int frame) {
    // double theta = M_PI/700 * frame;
    struct point p;
    p = pmul(in, rotation(0, 1, t1));
    p = pmul(p, rotation(0, 2, t2));
    p = pmul(p, rotation(0, 3, t3));
    p = pmul(p, rotation(1, 2, t4));
    p = pmul(p, rotation(1, 3, t5));
    p = pmul(p, rotation(2, 3, t6));
    return p;
}

double fov = M_PI/1.8;
double aspect = 1;
double near = 0.1;
double far = 10;

struct point project43(struct point point) {
    double tra = near * tan(fov/2); // top    right fourth-a
    double blb = -tra;              // bottom left  fourth-b

    struct matrix perspective = { 5, 5, {
        near,    0,    0, 0, 0,
           0, near,    0, 0, 0,
           0,    0, near, 0, 0,
           0,    0,    0, 1, 0,
           0,    0,    0, 0, 1,
    } };

    double scalf = 2/(tra-blb);

    struct matrix scale = { 5, 5, {
        scalf,     0,     0, 0, 0,
            0, scalf,     0, 0, 0,
            0,     0, scalf, 0, 0,
            0,     0,     0, 1, 0,
            0,     0,     0, 0, 1,
    } };

    double c1 = 2*far*near / (near - far);
    double c2 = (far + near) / (far - near);

    struct matrix depthmap = { 5, 5, {
        1, 0, 0,   0,  0,
        0, 1, 0,   0,  0,
        0, 0, 1,   0,  0,
        0, 0, 0, -c2, c1,
        0, 0, 0,  -1,  0,
    } };

    struct matrix mpoint = { 5, 1, {
        point.x,
        point.y,
        point.z,
        point.w,
        1,
    } };

    struct matrix projection = mul(scale, mul(perspective, depthmap));
    struct matrix result = mul(projection, mpoint);

    struct point out = {{{
        result.v[0] / result.v[4],
        result.v[1] / result.v[4],
        result.v[2] / result.v[4],
        result.v[3] / result.v[4],
    }}};

    return out;
}


struct point project32(struct point point) {
    double tra = near * tan(fov/2); // top    right fourth-a
    double blb = -tra;              // bottom left  fourth-b

    struct matrix perspective = { 4, 4, {
        near,    0, 0, 0,
           0, near, 0, 0,
           0,    0, 1, 0,
           0,    0, 0, 1,
    } };

    double scalf = 2/(tra-blb);

    struct matrix scale = { 4, 4, {
        scalf,     0, 0, 0,
            0, scalf, 0, 0,
            0,     0, 1, 0,
            0,     0, 0, 1,
    } };

    double c1 = 2*far*near / (near - far);
    double c2 = (far + near) / (far - near);

    struct matrix depthmap = { 4, 4, {
        1, 0,   0,  0,
        0, 1,   0,  0,
        0, 0, -c2, c1,
        0, 0,  -1,  0,
    } };

    struct matrix mpoint = { 4, 1, {
        point.x,
        point.y,
        point.z,
        1,
    } };

    struct matrix projection = mul(scale, mul(perspective, depthmap));
    struct matrix result = mul(projection, mpoint);

    struct point out = {{{
        result.v[0] / result.v[3],
        result.v[1] / result.v[3],
        result.v[2] / result.v[3],
        point.w,
    }}};

    return out;
}

#define FOURDPOINTS 2*2*2*2
#define FOURDEDGES 12*2+8

void tesseract(cairo_t *cr, int frame) {
    struct point points[FOURDPOINTS + 8];
    int i = 0;

    for (int x = -1; x <= 1; x += 2)
    for (int y = -1; y <= 1; y += 2)
    for (int z = -1; z <= 1; z += 2)
    for (int w = -1; w <= 1; w += 2) {
        points[i++] = (struct point){{{x, y, z, w}}};
    }

    points[i++] = (struct point){{{ 0.3,    0,    0,    0}}};
    points[i++] = (struct point){{{-0.3,    0,    0,    0}}};
    points[i++] = (struct point){{{   0,  0.3,    0,    0}}};
    points[i++] = (struct point){{{   0, -0.3,    0,    0}}};
    points[i++] = (struct point){{{   0,    0,  0.3,    0}}};
    points[i++] = (struct point){{{   0,    0, -0.3,    0}}};
    points[i++] = (struct point){{{   0,    0,    0,  0.3}}};
    points[i++] = (struct point){{{   0,    0,    0, -0.3}}};

    struct edge {
        int pointindex[2];
    };
    struct edge edges[FOURDEDGES + 4];
    int edgecount = 0;

    for (int p1 = 0; p1 < FOURDPOINTS; p1++)
    for (int p2 = 0; p2 < FOURDPOINTS; p2++) {
        // lines only differ in one dimension
        if (
            (points[p1].x == points[p2].x) +
            (points[p1].y == points[p2].y) +
            (points[p1].z == points[p2].z) +
            (points[p1].w == points[p2].w)
            != 3
        ) continue;

        // the reverse line already exists
        int continueouter = 0;
        for (int e = 0; e < edgecount; e++) {
            if (
                edges[e].pointindex[0] == p2 &&
                edges[e].pointindex[1] == p1
            ) {
                continueouter = 1;
                break;
            }
        }
        if (continueouter) continue;

        edges[edgecount++] = (struct edge){p1, p2};
    }

    edges[edgecount++] = (struct edge){FOURDPOINTS+0, FOURDPOINTS+1};
    edges[edgecount++] = (struct edge){FOURDPOINTS+2, FOURDPOINTS+3};
    edges[edgecount++] = (struct edge){FOURDPOINTS+4, FOURDPOINTS+5};
    edges[edgecount++] = (struct edge){FOURDPOINTS+6, FOURDPOINTS+7};

    // rotate points in x/y
    for (int i = 0; i < FOURDPOINTS+8; i++) {
        points[i] = rotate(points[i], frame);
    }

    struct point originalpoints[FOURDPOINTS];
    for (int i = 0; i < FOURDPOINTS; i++) {
        originalpoints[i] = points[i];
    }

    // project points with perspective 4d -> 2d
    for (int i = 0; i < FOURDPOINTS+8; i++) {
        points[i].w = points[i].w - 4;
        points[i] = project43(points[i]);
        points[i].z = points[i].z - 4.8;
        points[i] = project32(points[i]);
    }

    double maxw = 0;
    double minw = 1000;
    double maxz = 0;
    double minz = 1000;

    for (int i = 0; i < FOURDPOINTS; i++) {
        if (points[i].w > maxw) maxw = points[i].w;
        if (points[i].w < minw) minw = points[i].w;
        if (points[i].z > maxz) maxz = points[i].z;
        if (points[i].z < minz) minz = points[i].z;
    }

    cairo_set_line_width(cr, 0.03);

    if (axes)
    for (int i = 0; i < 8; i += 2) {
        struct point p1 = points[FOURDPOINTS+i];
        struct point p2 = points[FOURDPOINTS+i+1];
        if (i == 0)
            cairo_set_source_rgba(cr, 1, 0.3, 0.3, 1);
        else if (i == 2)
            cairo_set_source_rgba(cr, 0, 0.7, 0, 1);
        else if (i == 4)
            cairo_set_source_rgba(cr, 0, 0, 1, 1);
        else if (i == 6)
            cairo_set_source_rgba(cr, 0.9, 0, 0.8, 1);
        cairo_move_to(cr, p1.x, p1.y);
        cairo_line_to(cr, p2.x, p2.y);
        cairo_stroke(cr);
    }

    for (int i = 0; i < FOURDEDGES; i++) {
        struct point p1 = points[edges[i].pointindex[0]];
        struct point p2 = points[edges[i].pointindex[1]];

        double w1 = (p1.w - minw) / (maxw - minw);
        // double z1 = (p1.z - minz) / (maxz - minz);
        double w2 = (p2.w - minw) / (maxw - minw);
        // double z2 = (p2.z - minz) / (maxz - minz);

        cairo_pattern_t *pat = cairo_pattern_create_linear(
            p1.x, p1.y, p2.x, p2.y
        );
        cairo_pattern_add_color_stop_rgba(pat, 0, 1, w1, w1, 1);
        cairo_pattern_add_color_stop_rgba(pat, 1, 1, w2, w2, 1);
        cairo_set_source(cr, pat);

        cairo_move_to(cr, p1.x, p1.y);
        cairo_line_to(cr, p2.x, p2.y);
        cairo_stroke(cr);

        cairo_pattern_destroy(pat);
    }

    for (int i = 0; i < FOURDPOINTS; i++) {
        struct point p1 = points[i];
        struct point o1 = originalpoints[i];
        double w = (p1.w - minw) / (maxw - minw);
        // double z = (p1.z - minz) / (maxz - minz);

        cairo_set_source_rgba(cr, 1, w, w, 0.8);
        cairo_arc(cr, p1.x, p1.y, 0.05, 0, 2*M_PI);
        cairo_fill(cr);

        if (labels) {
            cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1);
            cairo_move_to(cr, p1.x + 0.05, p1.y - 0.02);
            char buf[128];
            sprintf(buf, "%.2f,%.2f,%.2f,%.2f", o1.x, o1.y, o1.z, o1.w);
            cairo_show_text(cr, buf);
            cairo_new_path(cr);
        }
    }
}

void render_frame(cairo_t *cr, [[maybe_unused]] int frame) {
    cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 1.0);
    cairo_paint(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 0.1);

    cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1);

    t1 += d1 * 0.01;
    t2 += d2 * 0.01;
    t3 += d3 * 0.01;
    t4 += d4 * 0.01;
    t5 += d5 * 0.01;
    t6 += d6 * 0.01;

    tesseract(cr, frame);
}
