# ascii

Demonstration of several noise algorithms in the terminal.

## Resources
- Value, Gradient, Worley, fBM, and hashing: https://thebookofshaders.com/11/
- Domain Warping: https://iquilezles.org/articles/warp/
- Colored noise power spectrum plots: https://en.wikipedia.org/wiki/Colors_of_noise
- Made it all happen: https://www.manim.community/

## Script

Lets define noise as a function `f` with any number of independent variables, 
bounded by some range, that produces a random signal.

$$
x_1, \ldots, x_n \mapsto s, \hspace{1cm} s_{min} \leq s \leq s_{max}
$$

For now, we'll supply a single variable `t`, and bound `s` between `0` and `1`.

$$
t \mapsto s, \hspace{1cm} 0 \leq s \leq 1
$$

```
show manim plot with a random signal
```

We can construct our signal programmatically by defining a function `sample_signal`
that returns a random value between `0` and `1`.

```c
float sample_signal(float t) {
    return (float)rand()/RAND_MAX;
}
```

However, with this approach, we fail to assign exactly one value to the input `t`. 
Every time we call `sample_signal` the result is completely random (`draw random 
signals over and over again`). Furthermore, the distribution of random samples 
depends on how often we call this function, instead of the scale of `t` (`change 
the scale of t in the x-axis, without changing the signal`). To fix this, I'll 
pass a seed into the random number generator by extracting the integer component 
of `t`.

```c
float sample_signal(float t) {
    int i = floorf(t);
    return (float)rand_r(&i)/RAND_MAX;
}
```

(`show sample_signal in scope`) But our signal becomes discontinuous between
integer boundaries, so I'll use the fractional component of `t` to linearly
interpolate between the current and upcoming random sample.

```c
float random(int seed) {
    return (float)rand_r(&seed)/RAND_MAX;
}

float interpolate(float a, float b, float t) {
    return a + (b - a) * t;
}

float sample_signal(float t) {
    float i = floorf(t);
    float f = t - i;

    float a = noise(i);
    float b = noise(i + 1);
    return interpolate(a, b, f);
}
```

(`show sample_signal in scope`) Finally, I'll replace our linear interpolation
with a fifth order smoothstep, removing any sharp transition points between 
random samples (`show t as linear transformed into the smoothstep`).

```c
float smooth5(float t) {
    return ((6 * t - 15) * t + 10) * t * t * t;
}

float sample_signal(float t) {
    /* ... */
    return interpolate(a, b, smooth5(f));
}
```

(`show sample_signal in scope`) I want to take a step back and try to
characterize the behavior of this noise. If we examine the frequencies contained
in our signal, we see that, on average, all frequencies are equally represented
(`show log log fft, as seen on the wikipedia pink noise page`). That is, the rate 
of change between intensity and frequency is zero (`draw line between endpoints`).
And the relationship between these properties, irregardless of source, defines 
the color of noise. In this case, our signal is said to be _white noise_ because
it exhibits an equal intensity across all frequencies. We'll discuss a method for
shaping this frequency distribution later when we start layering noise, but for
now, I want to introduce an algorithm for generating 2D noise.

Let's go back to our original function `f` and consider the case where two
variables are supplied, `x` and `y`.

$$
x, y \mapsto s, \hspace{1cm} 0 \leq s \leq 1
$$

(`show x and y plot with a plane, 0..1`) We can divide this space into a grid
with a cell size of one (`plot grows, checkerboard`), and for any sample `s`,
interpolate between random values generated at the corners of its bounding cell
(`show s point, draw lines between s and the cell corners`). 

First, we'll define a function to generate random values, accepting two seeds,
and I'll use a hash to scramble `x` and `y`.

```c
// https://thebookofshaders.com/11/
float random2(float x, float y) {
    float n = sinf(x * 12.9898 + y * 78.233) * 43758.5453123;
    return fract(n);
}
```

Then, we'll define our noise function, `sample_2d`, that generates a random 
value for each corner, defined by the integer components of `x` and `y`.

```c
float sample_2d(float x, float y) {
    float xi = floorf(x);
    float yi = floorf(y);

    float tl = noise2(xi,   yi);
    float tr = noise2(xi+1, yi);
    float bl = noise2(xi,   yi+1);
    float br = noise2(xi+1, yi+1);
}
```

I'll compute our sample by interpolating between the top and bottom corners with
the fractional component of `x`, then interpolate those edges by the fraction in 
`y`, using our smoothstep.

```c
float sample_2d(float x, float y) {
    /* ... */
    float top    = interpolate(tl,  tr,     smooth5(xfract));
    float bottom = interpolate(bl,  br,     smooth5(xfract));
    float sample = interpolate(top, bottom, smooth5(yfract));
}
```

To visualize our noise, I'll use the sample as an index into an array of ascii
characters and assign a grayscale color. In this way, a sample of `0` will be
"fully dark", while a sample of `1` is "fully bright" [^1].

```c
typedef struct {
    char c;
    int bg;
    int fg;
} Ascii;

Ascii sample_to_ascii(float sample) {
    Ascii ascii;

    const char* characters = " .,:;+=xX$&@";
    int len = strlen(characters);

    int index = sample * (len-1);
    ascii.c   = characters[index];
    ascii.fg  = index + 232;

    return ascii;
}
```

And voila, 2D noise (`show plot`). Both our `sample_signal` and `sample_2d` functions 
interpolate between random _values_, so we call this _value_ noise. I'll implement
another 2D noise algorithm later (`preview worley`) but for now let's shift gears.

Earlier, I said that our signal was white, that all frequencies had equal intensity,
but what happens if they don't (`show earlier spectral plot, shift the curve`)?

For example, brownian noise is characterized as having an intensity inversely
proportional to $f^2$, where $f$ is frequency (`show brownian spectral plot`). 
In other words, every time the frequency is doubled, the amplitude of that
frequency is halved (`show brownian signal building from harmonics`). This simple
relationship allows us to approximate a brownian signal through the repeated
addition of scaled sine waves. And you can see a broad, low frequency pattern 
begin to emerge that is refined, over and over, by the addition of higher 
frequencies. In fact, if you were to zoom in on any section of this signal, you'd
find the same kind of structure repeating at every scale (`use that manim example
with the zoomed window`). This property is called self-similarity, and, in my next 
video, I'll explore this topic in much greater detail.

Zooming back out, we can apply this same idea to our value noise, that is, sum
different layers, sampled at increasing frequencies, with decreasing amplitude,
to produce what's known as fractal Brownian motion.

I'll start by pulling out the value noise implementation into a separate function
and pass in some additional parameters. The first is `freq`, which will represent 
the frequency that we sample random values and we apply it by scaling `x` and `y`. 
The second is `offset`, which will simply add an offset to `x` and `y`.

```c
float sample_value_noise(float x, float y, float freq, float offset) {
    x *= freq;
    y *= freq;

    x += offset;
    y += offset;

    /* ... */
}
```

In `sample_2d`, I'll accept an additional parameter `t`, storing the total
elapsed time of the program, and use it to offset the call to `sample_value_noise`.
Then, I'll iterate over some number of layers, accumulate a value noise sample,
double the frequency, and half the amplitude.

```c
float sample_2d(float x, float y, float t) {
    float sample = 0;
    float freq = 2;
    float amplitude = 0.5;

    for (int i = 0; i < 4; i++) {
        sample += amplitude * sample_value_noise(x, y, freq, t);
        freq *= 2.0;
        amplitude /= 2.0;
    }

    return sample;
}
```

(`show plot`) Of course, we can play with the amplitude's attenuation to change
the frequency distribution (`show various attenuations`). While were at it, what 
happens if you parameterize the attenuation with our `sample_signal` function?

```c
float sample_2d(float x, float y, float t) {
    /* ... */
    for (int i = 0; i < 4; i++) {
        /* ... */
        amplitude /= 1.25 + sample_signal(t);
    }
}
```

(`show plot`) Notice how we are using the noise to shape itself. We can take 
this idea further. I'll pull out the fractal Brownian motion into a function 
`fbm` and use it to add an offset to the `x` and `y` coordinates before passing
them into `fbm`. This is called _domain warping_.

```c
float fbm(float x, float y, float t) {
    /* ... */
}

float sample_2d(float x, float y, float t) {
    return fbm(
        x + fbm(x, y, t), 
        y + fbm(x + 9.2, y + 12.4, t), 
        t
    );
}
```

[^1]: Not all noise functions will produce values across the entire [`0..1`] range,
so I'll go ahead and stretch the index across the encountered range.
    ```c
     Ascii sample_to_ascii(float sample) {
         /* ... */
         static float min = 1;
         static float max = 0;
         if (sample < min) min = sample;
         if (sample > max) max = sample;
         int index = (sample - min) / (max - min + 1e-6) * len;
         /* ... */
     }
    ```
