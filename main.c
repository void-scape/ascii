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

// !!! Notes are preceded by three bangs

// (quickly show all of the different noise visaulizatons)
// In this video, I'm going to help you build an intution for generating and
// visualizing noise by implementing several algorithms from first principles.
//
// And to begin, I'll introduce the core principles of generating noise by 
// describing how we might go about producing a signal like this. (show plot)

// So I have a function, `sample_signal`, that produces a sample between 0 
// and 1 given the variable t, which stores the elapsed time of the program.
//
// float sample_signal(float t) {
//     return /* ... */;
// }
//
// If I plot the value 0 as a function over time, 
// return 0.0; (show plot)
// Then the signal is always 0.
//
// And if I return a scaled value of t,
// return t * 0.25; (show plot)
// Then the signal grows until it escapes the bounds.
// 
// But I don't want my signal to grow indefinitely. Instead, I want it to grow
// until it reaches 1, and then return back to 0.
//
// So I'll define a function, `fract`, that returns the fractional component of
// a float.
// float fract(float v) {
// 
// }
//
// And I'll do so by subtracting v from the floor of v, where floor is a function
// that truncates a float.
// float fract(float v) {
//     return v - floorf(v);
// }
//
// !!! ^ This is subtracting the floor of v from v, not the other way around.
// !!! You can say that v is subtracted _by_ the floor of v though.
//
// So you could imagine, if v is 6.9, then the floor of 6.9 will be 6, and 6.9 - 6 = 0.9.
// 6.9 - 6.9
//        ^^
// 6.9 - 6 = 0.9
// 
// And if I plot the `fract` of t,
// return fract(t); (show plot)
// Then the signal will just ignore the integer component.

// Now, I want to plot a signal that generates a random value for each integer
// divison of t.
// -----
//                -----
//           -----
//      -----
//                     -----
// 1    2    3    4    5
//
// Which means that I'll need a function who returns a random value between 0
// and 1. And I'm going to use `rand` from stdlib to generate my value.
// 
// float noise() {
//     return (float)rand()/RAND_MAX;
// }
//
// But, I need `noise` to be deterministic, that is, for every integer division of
// t I need `noise` to generate the same value. So, instead of `rand`, I'll use
// `rand_r` which accepts a pointer to a seed.
//
// !!! but why does it need to be deterministic? I'm not sure this is motivated yet
//
// float noise(int seed) {
//     return (float)rand_r(&seed)/RAND_MAX;
// }
//
// And now I can plot my signal,
// return noise(floorf(t));
// But, you can see that our samples are not very random, and this is because
// our seeds are so simple that `rand_r` isn't able to generate convincing
// randomness.
//
// If you try this yourself, I do not recommend using your language's
// standard library in your noise function, and you should instead use a hash
// function like this.
//
// !!! Why not?
//
// float noise(int seed) {
//     float n = sinf((float)seed * 12.9898) * 43758.5453123;
//     return fract(n);
// }
//
// And if I plot these together, with `rand_r` on the top, and our hash on the
// bottom, you can immediately see the difference in quality. (Not going to show
// the code for this, just the plot).

// (show an image of the random and fractional plots together, and annotate the
// image)
// If we look at the relationship between our random and fract signals, notice 
// how fract is 0 at the start of a random value and 1 at the end. That means 
// we can interpolate between any two random values as a function of fract.
//
// So I'll define a function `linear` that returns the weighted sum of `a` and 
// `b` by the interpolation factor `t`, where `t` is between 0 and 1.
//
// float linear(float a, float b, float t) {
//     return (1.0 - t) * a + b * t;
// }
//
// Then I'll return a signal that interpolates between the current random value,
// `a`, and the next random value `b`, with a factor of `fract` t.
//
// float sample_signal(float t) {
//     float a = noise(floorf(t));
//     float b = noise(floorf(t) + 1);
//     return linear(a, b, fract(t));
// }

#define TIME_SCALE 4
#define SCOPE_HEIGHT 25

// (show interpolated plot & random plot)
//
// There is one thing missing from this signal, and although it could pull off
// something like a seismograph, more often we would prefer an eased interpolation,
// as opposed to linear.
//
// !!! ^ This feels a little clunky. Maybe qualifying it a bit could help:
// !!! eased interpolation is usually more pleasing.
//
// (show drawing pad with a linear plot of t, draw easing curve over it)
// Where the function of t is polynomial as opposed to linear.
//
// And we can achieve this by defining a function `ease` that transforms the
// interpolating factor `t` before applying the weighted sum.
//
// float ease(float lhs, float rhs, float t) {
//    return linear(lhs, rhs, ((6 * t - 15) * t + 10) * t * t * t);
// }
//
// And finally, in our sampling function, replacing the call to `linear` with 
// `ease`.
//
// float sample_signal(float t) {
//     float a = noise(floorf(t));
//     float b = noise(floorf(t) + 1);
//     return ease(a, b, fract(t));
// }
//
// (show plot, maybe all 3, random, linear, and eased)

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

// This idea of transforming a value `t` into a random signal can be 
// extrapolated into any number of dimensions. (showing plot as before)

// For instance, I have a function `sample_2d_value` that produces a sample, just
// as before, given an x and y coordinate, both of which are between 0 and 1.
// 
// (0, 0)        (1, 0)
//      +--------+  
//      |        | 
//      | SCREEN |
//      |        |
//      +--------+  
// (0, 1)        (1, 1)
//
// float sample_2d_value(float x, float y) {
//     return /* ... */;
// }
//
// And I map that signal into an ascii character here.
//
// char sample_to_ascii(float sample) {
//     if (sample > 0.5) {
//         return 'X';
//     } else {
//         return '.';
//     }
// }
//
// If I return 1 when x is greater than 0.5,
//
// float sample_2d_value(float x, float y) {
//     return x > 0.5; 
// }
//
// (show plot)
// Then the left hand side is periods and the right hand side is Xs.
//
// Let's try calling our `sample_signal` function on x and see what happens.
//
// float sample_2d_value(float x, float y) {
//     return sample_signal(x); 
// }
//
// (show plot)
// It's the same thing! Why is that? I think you'll see why if I map sample to 
// a gradient of characters.
//
// char sample_to_ascii(float sample) {
//     const char* gradient = "`.-^+*()&#";
//     int len = strlen(gradient);
//     return gradient[(int)(sample * len)];
// }
//
// (show plot)
// Hopefully now you can see that we are interpolating between two values, a
// sample taken at `x` = 0, and `x` = 1. And if we multiply `x` by two,
//
// float sample_2d_value(float x, float y) {
//     return sample_signal(x * 2); 
// }
//
// (show plot)
// We've divided the screen into a lattice which is defined by the integer divisons
// of the `x` coordinate, and we are interpolating between random values sampled
// at every boundary. 
// (draw this)
// (0, 0)
//      +---+---+
//      |   |   |
//      +---+---+
//              (2, 1)
//
// (all of this is going to be drawn because it is confusing as words)
// And now, a sample within this lattice is concerned, not by a single axis as 
// before with our signals, but two. That means that we have to interpolate 
// between the integer boundaries of both `x` and `y`, which define the corners 
// of a lattice cell.
//
// So I'll introduce a hash function `noise2` that returns a random value given
// two seeds.
//
// float noise2(int s1, int s2) {
//     float n = sinf((float)s1 * 12.9898 + (float)s2 * 78.233) * 43758.5453123;
//     return fract(n);
// }
//
// And in the `sample_2d_value` function, we need to sample from the corners
// of `x` and `y`.
//
// float sample_2d_value(float x, float y) {
//     float xwhole = floorf(x);
//     float ywhole = floorf(y);
// 
//     float xfract = x - xwhole;
//     float yfract = y - ywhole;
// 
//     float tl = noise2(xwhole, ywhole);
//     float tr = noise2(xwhole + 1, ywhole);
//     float bl = noise2(xwhole, ywhole + 1);
//     float br = noise2(xwhole + 1, ywhole + 1);
// }
//
// (draw this, explanation will change when actually recorded)
// The final sample is going to be the interpolation of the four corners. I'll
// start by interpolating the top left and top right corners by the fraction of
// `x`, and similarly the bottom left and bottom right. Then I'll interpolate
// between those values by the fraction of `y`.
//
// (0, 0)
//      <- tsample  ->
//      +-----+------+
//   v  |     |      |
//   s  |     |      |
//   a  +-----+------+
//   m  |     ^ (xfract, yfract)
//   p  |     |      |
//   l  |     |      |
//   e  +-----+------+
//      <- bsample  ->
//                   (1, 1)
//
// float sample_2d_value(float x, float y) {
//     /* ... */
//     float tsample = ease(tl, tr, xfract);
//     float bsample = ease(bl, br, xfract);
//     return ease(tsample, bsample, yfract);
// }
//
// To finish this off, I'll parameterize the scale of `x` and `y`. I'll call
// this `freq` because it refers to the total number of integer divisions on 
// the screen, and by extension the number of random samples.
//
// !!! Hm, isn't this more like resolution or scale maybe?
// !!! I guess frequency works better with the octave terminology....
//
// float sample_2d_value(float x, float y) {
//     float freq = 8.0;
//     x *= freq;
//     y *= freq;
//     /* ... */
// }
//
// (show plot)
// (quickly show pictures of things mentioned in this paragraph)
// What I've just implemented is called value noise and it is based on a lattice
// structure, similar to both perlin and simplex noise, however perlin and simplex
// define random gradients instead of single values and furthermore simplex uses
// a triangle lattice that reduces some artifacts, among other things.
//
// !!! ^ is this the first time we've mentioned perlin/simplex? it might be a little
// !!! abrupt to just namedrop them. This might need a touch of expansion or
// !!! possibly later development.
//
// Each algorithm is going to provide you a unique style but they share the same
// fundamental ideas, although exploring vector math and triangle lattices is
// beyond the scope of this video. If you are interested, there is a link to
// this repository in the description which contains a very simple perlin noise
// implementation, as well as some further reading.

// I want to break out this value noise implementaion into a seperate function
// that accepts freq as an argument and also introduce `t`, the elapsed time.
// I'll use `t` to offset the `x` and `y` coordinate which will scroll through
// the value noise.
//
// float sample_value_noise(float x, float y, float freq, float t) {
//     x += t;
//     y += t;
//     /* ... */
// }
//
// And in the original function pass in `t` and call into `sample_value_noise`.
//
// float sample_2d_value(float x, float y, float t) {
//     return sample_value_noise(x, y, 8.0, t);
// }
//
// (show plot)
// Now that we have parameterized the value noise in another function I can
// take more than one sample at a time and combine them. I'll take two samples 
// with different frequencies and offsets then return the average.
//
// float sample_2d_value(float x, float y, float t) {
//     float s1 = sample_value_noise(x, y, 4.0, t);
//     float s2 = sample_value_noise(x, y, 8.0, -t * 2);
//     return (s1+s2)/2;
// }
//
// (show plot)
// And the result is much more dynamic because the value of a coordinate
// will change over time as the two samples interact over different offsets.
// `s1` and `s2` are layers of noise, also called octaves, and combining
// octaves is called layering.
//
// We can formalize this layering by constructing a for loop that accumulates
// into a sample. I'll pull out the frequency and define an `amplitude` and `octaves`.
// For each octave, I will add to `sample` the product of `amplitude` and
// the value noise, then double `freq` and half `amplitude`.
//
// float sample_2d_value(float x, float y, float t) {
//     float sample = 0;
//     float freq = 2.0;
//     float amplitude = 0.5;
//     int octaves = 1;
// 
//     for (int i = 0; i < octaves; i++) {
//         sample += amplitude * sample_value_noise(x, y, freq, t);
//         freq *= 2.0;
//         amplitude /= 2.0;
//     }
// 
//     return sample;
// }
//
// With only 1 octave, this plot should look the same, but as I increase the
// number of iterations, more detail begins to appear. 
// (increment octaves 1 by 1 and show plot)
// And this detail is actually self similar because we are sampling from the
// the same underlying value noise with an integer multiple of the frequency.
// As you might have guessed, this is called fractal noise, or more specifically,
// brown noise. And the color of noise, here, refers to how we scale the amplitude
// in each iteration.
//
// If we divide by the square root of 2 instead, then this becomes pink noise.
//
// float sample_2d_value(float x, float y, float t) {
//     /* ... */
//     amplitude /= sqrtf(2);
//     /* ... */
// }
//
// (show plot)
// And notice how much more detail suddenly emerges, this is because pink noise
// caries a lot of high frequency energy, and there are things all over the natural
// world that produce this kind of signal, it's such a fascinating topic.
//
// I want to clarify that this self similarity can apply to any other basis noise
// function and is not specific to our value noise implementation. When I call 
// this pink or brown noise, I am refering to the way that octaves are combined, 
// not the octaves themselves.
//
// (show perlin fbm)
// For instance, here is the same code as before but I've switched out the value
// noise for perlin, and we can see the same overall behaviour.


// TODO: SHOW IN INTRO!
// return clampf(ease(sample_signal(x * 2), sample_signal(y * 2), sinf(t)));
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

// (show a sketchpad with the title WORLEY NOISE)
//
// Let's look at a point based algorithm called Worley noise. I'll define a lattice, 
// just like before, with boundaries at the integer divisons of the x and y 
// coordinates.
//
// (draw lattice)
// 
// For each cell I'll define a control point with a random position.
//
// (draw a dot at a random spot in each cell)
//
// And to take a sample, given an x and y coordinate, I'll iterate over all of
// the control points and keep track of the minimum distance to the sample
// coordinate.
//
// (draw lines from sample to each control point)
//
// Finally, I'll return the minimum distance and scale it to be between 0 and 1.
// But before we implement this, I want you to notice that iterating over all
// of the control points is unnecessary. In fact, we only need to check the nine
// cells around our sample coordinate.
//
// So I've defined a function `sample_2d_point` that accepts a sample coordinate,
// `x` and `y`, as well as the elapsed time `t`. I'll multiply the `x` and `y`
// by a `freq`, just like before.
//
// float sample_2d_point(float x, float y, float t) {
//     float freq = 4.0;
//     x *= freq;
//     y *= freq;
// }
//
// Then, I'll grab the integer component of `x` and `y` and define a list of
// nine control points. Each control point is a 2D coordinate, so I've defined
// a `vec` struct that holds an `x` and `y` position. 
// (show the vec, already written)
//
// float sample_2d_point(float x, float y, float t) {
//     /* ... */
//
//     float xwhole = floorf(x);
//     float ywhole = floorf(y);
//
//     vec control_points[] = {
//         vec2(xwhole, ywhole),
//         vec2(xwhole + 1, ywhole),
//         vec2(xwhole - 1, ywhole),
//         vec2(xwhole, ywhole + 1),
//         vec2(xwhole, ywhole - 1),
//         vec2(xwhole + 1, ywhole + 1),
//         vec2(xwhole - 1, ywhole + 1),
//         vec2(xwhole + 1, ywhole - 1),
//         vec2(xwhole - 1, ywhole - 1)
//     };
// }
//
// Let's define a `min_dist` and iterate over the control points.
//
// float sample_2d_point(float x, float y, float t) {
//     /* ... */
//
//     float min_dist = freq;
//     for (int i = 0; i < 9; i++) {
//
//     }
// }
//
// For each point, we need to generate a random 2D position, so I've defined
// a function `vnoise2` that accepts a `vec` seed and returns a `vec` between
// 0 and 1.
// (show vnoise2, already written)
//
// So we can generate a `point` given our control point as a `seed`. Then, I
// will add the `seed` to the `point`.
//
// float sample_2d_point(float x, float y, float t) {
//     /* ... */
//     for (int i = 0; i < 9; i++) {
//         vec seed = control_points[i];
//         vec point = vnoise2(seed);
//         point = add(point, seed);
//     }
// }
//
// Now we can find the distance between `point` and our sample coordinate and
// store the minimum distance by taking the `min` of `min_dist` and `dist`.
//
// float sample_2d_point(float x, float y, float t) {
//     /* ... */
//     for (int i = 0; i < 9; i++) {
//         /* ... */
//         float dist = distance(point, vec2(x, y));
//         min_dist = fmin(min_dist, dist);
//     }
// }
//
// And then we can return the `min_dist`, divided by the maximum possible
// distance between a sample and control point, which in this case the the square
// root of 2.
//
// float sample_2d_point(float x, float y, float t) {
//     /* ... */
//     return min_dist/sqrtf(2);
// }
//
// (show plot for a second)
//
// (show plot with the control points highlighted)
// If I highlight the control points, you'll be able to see how the sample grows
// as the distance from the nearest control point increases.
//
// We can use the elapsed time to animate the position of our control points
// in their cell, like this.
//
// float sample_2d_point(float x, float y, float t) {
//     /* ... */
//     for (int i = 0; i < 9; i++) {
//         /* ... */
//         vec point = animate_point(vnoise2(seed), t);
//         /* ... */
//     }
//     /* ... */
// }
//
// (show plot)
// And watch how the samples adapt to their surrounding control points.
//
// Let's pull out and parameterize the worley noise in a function and combine
// combine two octaves with frequencies 3 and 8.
//
// float sample_2d_point(float x, float y, float t) {
//     float s1 = sample_worley_noise(x, y, 3.0, t);
//     float s2 = sample_worley_noise(x, y, 8.0, t);
//     return (s1+s2)/2;
// }
//
// (show plot)

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

// (still showing worley plot)
// I want to close this video by combining all of the techniques we've explored
// to procedurally generate an overworld with oceans, grasslands, and mountains.
//
// I'll start by augmenting the way that we render our samples. I'll introduce
// a list of colors that will be indexed just like the ascii gradient. This
// palette will transition from blues to greens to greys. That is, with a sample
// of 0.0, we will render the deepest part of the ocean, and with a sample of
// 1.0, the peaks of mountains.
//
// Ascii overworld_sample_to_ascii(float sample) {
//     /* ... */
//     const int bg[] = { 17, 18, 19, 21, 75, 228, 118, 241, 246, 255 };
//     /* ... */
//     ascii.bg = bg[index];
// 
//     return ascii;
// }
//
// Next, in this `sample_overworld` function, I'll sample from different 
// noise functions every four seconds. 
//
// float sample_overworld(float x, float y, float t) {
//     float dur = 4.0;
// 
//     if (t < dur) {
// 
//     }
// 
//     if (t < dur * 2) {
// 
//     }
// 
//     if (t < dur * 3) {
// 
//     }
// 
//     if (t < dur * 4) {
// 
//     }
// 
//     if (t < dur * 5) {
// 
//     }
// 
//     return 0;
// }
//
// (show the `sample_overworld` function, no need to write this out live)
// In each of these code blocks, I've just defined some octaves with random 
// parameters and layered them in a way that I think looks nice.
//
// (show plot)
// And I want to highlight the distinction between _generating_ and _visualizing_ 
// noise to achieve different aesthetics. In the overworld code, I've defined a 
// function that _maps_ samples into ascii, but despite the same mapping, each
// _combination_ of noise is exploring a unique topology.
//
// !!! You could probably drop the "and"s if you want.
//
// (start playing some music)
//
// I want you to think about noise algorithms as a set of primary colors that
// we, as artists, can mix and match to compose graphics that express our
// ultimate vision. We've only scratched the surface, and I'll leave you with 
// a taste of what lies below...
//
// !!! "I want you to" is a bit of an authoritative tone for my tastes.
// !!! I would go with "You can" personally, but of course you can strike
// !!! whatever tone you want.
//
// (music dies down, voice is lowpassed and bit crushed... slight pause...)
//
// (music explodes into a climax, brilliant flashes of noise fly across the
// screen! the viewer bursts into tears of astonishment, shrouded by the
// beauty of pseudorandom ascii art!)
//
// !!! waor,,,

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
