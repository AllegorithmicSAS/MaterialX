#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_fractal3d_vector3(vec3 amplitude, int octaves, float lacunarity, float diminish, vec3 position, out vec3 result)
{
    vec3 value = sx_fractal_noice_vec3(position, octaves, lacunarity, diminish);
    result = value * amplitude;
}
