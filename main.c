#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

struct {
    float x, y;
} typedef Vec2;

Vec2 vec2(float x, float y) {
    return (Vec2) {
        x, y
    };
}

Vec2 sub(Vec2 lhs, Vec2 rhs) {
    return vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

Vec2 add(Vec2 lhs, Vec2 rhs) {
    return vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

Vec2 mul(Vec2 lhs, Vec2 rhs) {
    return vec2(lhs.x * rhs.x, lhs.y * rhs.y);
}

// NOTE: I don't know how this behaves when p.x or p.y is greater than
// the maximum value of a u16
float seed(Vec2 p) {
    int a = floorf(p.x);
    int b = floorf(p.y);
    return (a + b) * (a + b + 1) / 2 + a;
}

float length(Vec2 v) {
    // c^2 = a^2 + b^2
    return sqrtf(v.x * v.x + v.y * v.y);
}

Vec2 norm(Vec2 v) {
    float l = length(v);
    if (l == 0) {
        return vec2(1, 0);
    }
    return vec2(v.x/l, v.y/l);
}

Vec2 rand_vec2(Vec2 p) {
    int s1 = seed(p);
    int s2 = seed(mul(p, vec2(100.0, 100.0)));
    Vec2 r = vec2(
            (float)rand_r(&s1)/RAND_MAX*2-1, 
            (float)rand_r(&s2)/RAND_MAX*2-1);
    return norm(r);
}

float lerp(float lhs, float rhs, float t) {
    return (1.0 - t) * lhs + rhs * t;
}

float dot(Vec2 lhs, Vec2 rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

float fade(float t) {
	return ((6*t - 15)*t + 10)*t*t*t;
}

float perlin(Vec2 p) {
    // start
    // +---------+
    // |         |
    // |   x     |
    // |    (pos)|
    // |         |
    // +---------+
    Vec2 start = vec2(floorf(p.x), floorf(p.y));
    Vec2 pos = sub(p, start);

    Vec2 tl = vec2(0, 0);
    Vec2 tr = vec2(1, 0);
    Vec2 bl = vec2(0, 1);
    Vec2 br = vec2(1, 1);

    Vec2 tl_grad = rand_vec2(add(start, tl));
    Vec2 tr_grad = rand_vec2(add(start, tr));
    Vec2 bl_grad = rand_vec2(add(start, bl));
    Vec2 br_grad = rand_vec2(add(start, br));

    float tl_dot = dot(tl_grad, sub(pos, tl));
    float tr_dot = dot(tr_grad, sub(pos, tr));
    float bl_dot = dot(bl_grad, sub(pos, bl));
    float br_dot = dot(br_grad, sub(pos, br));

    float fx = fade(pos.x);
    float fy = fade(pos.y);
    float per = lerp(
        lerp(tl_dot, tr_dot, fx),
        lerp(bl_dot, br_dot, fx),
        fy
    );
    return (per+1)/2;
}

float white_noise() {
    return (float)rand()/RAND_MAX;
}

float layered_perlin(Vec2 p, float* amp, float* freq, Vec2* time_offset, int len) {
    float scale = 0;
    float sample = 0;
    for (int i = 0; i < len; i++) {
        float a = amp[i];
        float f = freq[i];
        sample += perlin(add(mul(p, vec2(f, f)), time_offset[i])) * a;
        scale += a;
    }
    return sample / scale;
}

int main(void) {
    int s = 4;
    int width = 16*s;
    int height = 9*s;

    float dt = 1.0 / 30.0;
    float t = 0.0;

    float min = 1;
    float max = 0;

    int colors[100];
    char* letters = " `.-'^=!*#Q%@";
    int len = strlen(letters);
    int start_color = 232;
    for (int i = 0; i < len; i++) {
        colors[i] = start_color;
        start_color += 4;
    }

    printf("\e[?25l");
    while (1) {
        printf("\e[H");
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Vec2 p = vec2((float)x/width, (float)y/height);
                float amp[2] = {1.0, 1.0};
                float freq[2] = {3.0, 6.0};
                Vec2 time_offset[2] = {vec2(-t * 0.1, t * 0.1), vec2(t, -t)};
                float sample = layered_perlin(p, amp, freq, time_offset, sizeof(amp)/sizeof(*amp));

                if (sample > max) max = sample;
                if (sample < min) min = sample;

                // max: 0.5
                // min: 0.0
                // sample: 0.25
                //
                // (0.25 - 0.0) / (0.5 - 0.0) = 0.5
                // NOTE: adding an epsilon because, on the first sample, max == min
                int index = (float)((sample - min) / (max - min + 1e-6))*(len-1);

                printf("\e[38;5;%dm%c ", colors[index], letters[index]);
            }
            printf("\n");
        }
        usleep(dt * 1000 * 1000);
        t += dt;
    }
    return 0;
}
