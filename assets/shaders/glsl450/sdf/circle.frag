#version 450

layout(location = 0) in vec2 v_local_pos;
layout(location = 0) out vec4 outColor;

// In SDL3 GPU SPIR-V conventions, fragment uniform slot 0 is bound to set = 3
layout(set = 3, binding = 0) uniform FragmentUniforms {
    mat4 u_mvp;
    //vec2 u_center;
    //vec2 u_padding1;
    vec4 u_color;
    float u_radius;
    float u_feather;
    vec2 u_padding2;
} pc;

void main() {
    float extent = pc.u_radius + pc.u_feather;
    float dist = length(v_local_pos) * extent;

    // Anti-aliased edge calculation
    float alpha = 1.0 - smoothstep(pc.u_radius - pc.u_feather, pc.u_radius + pc.u_feather, dist);
    if (alpha <= 0.0) discard;

    outColor = pc.u_color * alpha;
}