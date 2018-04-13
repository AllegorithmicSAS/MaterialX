#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_noise2d_vector3(vec3 amplitude, float pivot, vec2 texcoord, out vec3 result)
{
    vec3 value = sx_perlin_noise_vec3(texcoord);
    result = value * amplitude + pivot;
}
