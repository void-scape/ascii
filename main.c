#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Controls what kind of noise is plotted
#define SIGNAL 0
#define SIGNALS 0
#define VALUE 0
#define GRADIENT 0
#define CELLULAR 1
#define WORLD 0

// Configure the scope
#define TIME_SCALE 4
#define SCOPE_HEIGHT 12

// Configure the grid dimensions
#define S 10
#define HEIGHT (9*S/2)
// #define WIDTH (HEIGHT*2)
#define WIDTH (16*S)
// #define ASPECT 1.0
#define ASPECT 1.69

float clampf(float v) {
    if (v > 1.0) return 1.0;
    if (v < 0.0) return 0.0;
    return v;
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

float fract(float v) {
    return v - floorf(v);
}

// https://thebookofshaders.com/11/
float noise(int seed) {
    float n = sinf((float)seed * 12.9898) * 43758.5453123;
    return fract(n);
}

float linear(float lhs, float rhs, float t) {
    return lhs + (rhs - lhs) * t;
}

float ease(float lhs, float rhs, float t) {
   return linear(lhs, rhs, ((6 * t - 15) * t + 10) * t * t * t);
}

//

#define TIME_SCALE 1
#define TIME_SCALE 4
#define SCOPE_HEIGHT 25

//   +
// S |
// A |    ...
// M | ../   \
// P |        ..../
// L |
// E |  
//   + ------------- +
//           t
//        
float sample_signal(float t) {
    float a = noise(floorf(t));
    float b = noise(floorf(t) + 1);
    return ease(a, b, fract(t));
}

typedef struct {
    float x, y;
} Samples;

Samples sample_signals(float t) {
    Samples samples;

    float whole = floorf(t);
    float fract = t - whole;

    float s1 = noise(whole);
    float s2 = noise(whole + 1);

    samples.x = linear(s1, s2, fract);
    samples.y = ease(s1, s2, fract);

    return samples;
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

// https://thebookofshaders.com/11/
float noise2(int s1, int s2) {
    float n = sinf((float)s1 * 12.9898 + (float)s2 * 78.233) * 43758.5453123;
    return fract(n);
}

#define S 10

float sample_value_noise(float x, float y, float freq, float t) {
    x *= freq;
    y *= freq;

    x += t;
    y += t;

    float xwhole = floorf(x);
    float ywhole = floorf(y);

    float xfract = x - xwhole;
    float yfract = y - ywhole;

    float tl = noise2(xwhole, ywhole);
    float tr = noise2(xwhole + 1, ywhole);
    float bl = noise2(xwhole, ywhole + 1);
    float br = noise2(xwhole + 1, ywhole + 1);

    float tsample = ease(tl, tr, xfract);
    float bsample = ease(bl, br, xfract);
    float sample = ease(tsample, bsample, yfract);

    return sample;
}

float sample_2d_value(float x, float y, float t) {
    return sample_value_noise(x, y, 8.0, t);

    float sample = 0;
    float freq = 8.0;
    float amplitude = 0.5;

    for (int i = 0; i < 2; i++) {
        sample += amplitude * sample_value_noise(x, y, freq, t);
        freq *= 2;
        amplitude /= 2;
    }

    return sample;
}

typedef struct {
    char c;
    int bg;
    int fg;
} Ascii;

//   +
//   |    ...
//   | ../   \        ->     'X'
//   |        ...
//   |  
//   + ----------- +
//        
Ascii sample_to_ascii(float sample) {
    Ascii ascii;

    if (sample == 69.0) {
        return (Ascii){' ', 27, 0};
    }

    static float min = 1;
    static float max = 0;

    if (sample < min) min = sample;
    if (sample > max) max = sample;

    const char* letters = " .,:;+=xX$&@";
    int len = strlen(letters);

    int index = (sample - min) / (max - min + 1e-6) * len;
    ascii.c = letters[index];
    ascii.fg = (float)index * 2.5 + 232;

    return ascii;
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

//

typedef struct {
    float x, y;
} vec;

vec vec2(float x, float y) {
    return (vec) {
        x, y
    };
}

vec splat(float v) {
    return vec2(v, v);
}

vec add(vec lhs, vec rhs) {
    return vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

vec sub(vec lhs, vec rhs) {
    return vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

vec mul(vec lhs, vec rhs) {
    return vec2(lhs.x * rhs.x, lhs.y * rhs.y);
}

float dot(vec lhs, vec rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

float length(vec v) {
    return sqrtf(dot(v, v));
}

vec norm(vec v) {
    float l = length(v);
    if (l == 0.0) return vec2(1, 0);
    return vec2(v.x/l, v.y/l);
}

vec vfract(vec v) {
    return vec2(v.x - floorf(v.x), v.y - floorf(v.y));
}

vec vsin(vec v) {
    return vec2(sinf(v.x), sinf(v.y));
}

vec vabs(vec v) {
    return vec2(fabs(v.x), fabs(v.y));
}

//

// https://thebookofshaders.com/edit.php#11/2d-gnoise.frag
vec vnoise2(vec seed) {
    vec st = vec2(
            dot(seed, vec2(127.1, 311.7)),
            dot(seed, vec2(269.5, 183.3)));
    return vfract(mul(vsin(st), splat(43758.5453123)));
}

vec vnoise2_norm(vec seed) {
    return norm(add(splat(-1.0), mul(splat(2.0), vnoise2(seed))));
}

float sample_gradient_noise(float x, float y, float freq, float t) {
    x *= freq;
    y *= freq;

    x += t;
    y += t;

    float xwhole = floorf(x);
    float ywhole = floorf(y);

    float xfract = x - xwhole;
    float yfract = y - ywhole;
    vec pos = vec2(xfract, yfract);

    vec tloffset = vec2(xwhole, ywhole);
    vec troffset = vec2(xwhole + 1, ywhole);
    vec bloffset = vec2(xwhole, ywhole + 1);
    vec broffset = vec2(xwhole + 1, ywhole + 1);

    vec tlgrad = vnoise2_norm(tloffset);
    vec trgrad = vnoise2_norm(troffset);
    vec blgrad = vnoise2_norm(bloffset);
    vec brgrad = vnoise2_norm(broffset);

    float tl = dot(vec2(xfract, yfract), tlgrad);
    float tr = dot(vec2(xfract - 1, yfract), trgrad);
    float bl = dot(vec2(xfract, yfract - 1), blgrad);
    float br = dot(vec2(xfract - 1, yfract - 1), brgrad);

    float tsample = ease(tl, tr, xfract);
    float bsample = ease(bl, br, xfract);
    float sample = ease(tsample, bsample, yfract);

    return (sample+1)/2;
}


float sample_2d_gradient(float x, float y, float t) {
    float sample = 0;
    float freq = 2.0;
    float amplitude = 0.5;

    for (int i = 0; i < 8; i++) {
        sample += amplitude * sample_gradient_noise(x, y, freq, t);
        freq *= 2;
        amplitude /= sqrtf(2);
    }

    return sample;
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

float distance(vec lhs, vec rhs) {
    return length(sub(lhs, rhs));
}

vec animate_point(vec p, float t) {
    return add(splat(0.5), mul(splat(0.5), vsin(add(splat(t), mul(splat(6.2831), p)))));
}

float sample_worley_noise(float x, float y, float freq, float t) {
    x *= freq;
    y *= freq;

    float xwhole = floorf(x);
    float ywhole = floorf(y);

    vec control_points[] = {
        vec2(xwhole, ywhole),
        vec2(xwhole + 1, ywhole),
        vec2(xwhole - 1, ywhole),
        vec2(xwhole, ywhole + 1),
        vec2(xwhole, ywhole - 1),
        vec2(xwhole + 1, ywhole + 1),
        vec2(xwhole - 1, ywhole + 1),
        vec2(xwhole + 1, ywhole - 1),
        vec2(xwhole - 1, ywhole - 1)
    };

    float min_dist = 2;
    for (int i = 0; i < 9; i++) {
        vec seed = control_points[i];
        vec point = animate_point(vnoise2(seed), t);
        point = add(point, seed);

        // NOTE: visualize the control points
        // if (length(sub(vec2(x, y), point)) < 1.0/20.0) {
        //     return 69.0;
        // }

        float dist = distance(point, vec2(x, y));
        min_dist = fmin(min_dist, dist);
    }

    return min_dist/sqrtf(2);
}

float sample_2d_point(float x, float y, float t) {
    return sample_worley_noise(x, y, 6.0, t);

    // float s1 = sample_worley_noise(x, y, 3.0, t);
    // float s2 = sample_worley_noise(x, y, 8.0, t);
    // return (s1+s2)/2;
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

float sample_overworld(float x, float y, float t) {
    float dur = 4.0;

    if (t < dur) {
        float s1 = 0.25 * sample_value_noise(x, y, 8.0, 0.0);
        float s2 = 0.75 * sample_gradient_noise(x, y, 4.0, 20.0);
        return s1 + s2;
    }

    if (t < dur * 2) {
        float s1 = 0.25 * sample_worley_noise(x, y, 8.0, 0.0);
        float s2 = 0.75 * sample_gradient_noise(x, y, 4.0, 20.0);
        return s1 + s2;
    }

    if (t < dur * 3) {
        float s1 = 0.25 * sample_worley_noise(x, y, 8.0, -4.0);
        float s2 = 0.5 - 0.50 * sample_gradient_noise(x, y, 4.0, 20.0);
        float s3 = 0.25 * sample_value_noise(x, y, 6.0, -2.0);
        return s1 + s2 + s3;
    }

    if (t < dur * 4) {
        return sample_worley_noise(x, y, 4.0, 0.0);
    }

    if (t < dur * 5) {
        return 1 - sample_worley_noise(x, y, 6.0, 8.0);
    }
}

Ascii overworld_sample_to_ascii(float sample) {
    Ascii ascii;

    static float min = 1;
    static float max = 0;

    if (sample < min) min = sample;
    if (sample > max) max = sample;

    const char* letters = " `.-^+*(&#";
    int len = strlen(letters);
    const int bg[] = { 17, 18, 19, 21, 75, 228, 118, 241, 246, 255 };

    int index = (sample - min) / (max - min + 1e-6) * len;
    ascii.c = letters[index];
    ascii.bg = bg[index];

    return ascii;
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

// implementation garbage if you are curious

void check_exit(void) {
    // why is this so weird?
    // https://stackoverflow.com/questions/717572/how-do-you-do-non-blocking-console-i-o-on-linux-in-c
    char buf[20];
    int flags = fcntl(0, F_GETFL);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    int r = read(0, buf, 20);
    fcntl(0, F_SETFL, flags);
    if (buf[0] == '\n' || r > 0) {
        printf("\e[?25h");
        exit(0);
    }
}

typedef struct {
    int hit;
    char c;
    int bg;
} Sample;

#define SCOPE_WIDTH (SCOPE_HEIGHT*6)

void render_scope(Sample *screen) {
    int last = 0;
    for (int y = 0; y < SCOPE_HEIGHT; y++) {
        for (int x = 0; x < SCOPE_WIDTH; x++) {
            Sample sample = screen[y * SCOPE_WIDTH + x];
            if (sample.hit) {
                last = 1;
                printf("\e[48;5;%dm%c", sample.bg, sample.c);
            } else {
                if (last) {
                    printf("\e[0m ");
                } else {
                    printf(" ");
                }
                last = 0;
            }
        }
        printf("\e[0m\n");
    }
}

void scope(void) {
    Sample screen[SCOPE_WIDTH * SCOPE_HEIGHT];
    Sample screen2[SCOPE_WIDTH * SCOPE_HEIGHT];
    float t = 0.0;
    float dt = 1.0/SCOPE_WIDTH * 2;
    while (1) {
        memset(&screen, 0, SCOPE_WIDTH * SCOPE_HEIGHT * sizeof(Sample));
        memset(&screen2, 0, SCOPE_WIDTH * SCOPE_HEIGHT * sizeof(Sample));
        for (int x = 0; x < SCOPE_WIDTH; x++) {
            for (int y = 0; y < SCOPE_HEIGHT; y++) {
                float time = (t + x * dt) * TIME_SCALE;
                if (!SIGNALS) {
                    float sample = sample_signal(time);
                    int y = sample * SCOPE_HEIGHT;
                    if (y >= 0 && y < SCOPE_HEIGHT) {
                        Sample* sample = &screen[(SCOPE_HEIGHT - 1 - y) * SCOPE_WIDTH + x];
                        sample->hit = 1;
                        sample->c = ' ';
                        sample->bg = 196 + y;
                    }
                } else {
                    Samples samples = sample_signals(time);

                    int yx = samples.x * SCOPE_HEIGHT;
                    if (yx >= 0 && yx < SCOPE_HEIGHT) {
                        Sample* sample = &screen[(SCOPE_HEIGHT - 1 - yx) * SCOPE_WIDTH + x];
                        sample->hit = 1;
                        sample->c = ' ';
                        sample->bg = 196 + yx;
                    }

                    int yy = samples.y * SCOPE_HEIGHT;
                    if (yy >= 0 && yy < SCOPE_HEIGHT) {
                        Sample* sample = &screen2[(SCOPE_HEIGHT - 1 - yy) * SCOPE_WIDTH + x];
                        sample->hit = 1;
                        sample->c = ' ';
                        sample->bg = 104 + yy;
                    }
                }
            }
        }
        t += dt;
        printf("\e[H");
        render_scope((void*)&screen);
        if (SIGNALS) {
            render_scope((void*)&screen2);
        }
        printf("t: %.2f\n", t);
        usleep(1000 * 20);
        check_exit();
    }
}

void render_world(Ascii *screen) {
    printf("\e[H");
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Ascii pos = screen[y * WIDTH + x];
            int set = 0;
            if (pos.bg) {
                set = 1;
                printf("\e[48;5;%dm", pos.bg);
            }
            if (pos.fg) {
                set = 1;
                printf("\e[38;5;%dm", pos.fg);
            }
            printf("%c", pos.c);
            if (set) {
                int set = 0;
                printf("\e[0m");
            }
        }
        printf("\n");
    }
}

void world(void) {
    Ascii screen[WIDTH * HEIGHT];
    float t = 0.0;
    float dt = 1.0/60.0;
    while (1) {
        memset(&screen, 0, WIDTH * HEIGHT * sizeof(Ascii));
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                float px = (float)x/WIDTH*ASPECT;
                float py = (float)y/HEIGHT;
                if (VALUE) {
                    float sample = sample_2d_value(px, py, t);
                    screen[y * WIDTH + x] = sample_to_ascii(sample);
                }
                if (GRADIENT) {
                    float sample = sample_2d_gradient(px, py, t);
                    screen[y * WIDTH + x] = sample_to_ascii(sample);
                }
                if (CELLULAR) {
                    float sample = sample_2d_point(px, py, t);
                    screen[y * WIDTH + x] = sample_to_ascii(sample);
                }
                if (WORLD) {
                    float sample = sample_overworld(px, py, t);
                    screen[y * WIDTH + x] = overworld_sample_to_ascii(sample);
                }
            }
        }
        render_world((void*)&screen);
        usleep(dt * 1000 * 1000);
        t += dt;
        check_exit();
    }
}

int main(void) {
    printf("\e[?25l\e[2J\e[?7l");
    if (SIGNAL || SIGNALS) scope();
    if (VALUE || GRADIENT || CELLULAR || WORLD) world();
    return 0;
}
