#version 450

layout(location = 0) in vec2 v_local_pos;
layout(location = 0) out vec4 frag_color;

// In SDL3 GPU SPIR-V conventions, fragment uniform slot 0 is bound to set = 2
layout(set = 2, binding = 0) uniform FragmentUniforms {
    mat4 u_view_projection;
    vec2 u_center;
    vec2 u_padding1;
    vec4 u_color;
    float u_radius;
    float u_feather;
    vec2 u_padding2;
};

void main() {
    float dist = length(v_local_pos) * (u_radius + u_feather);
    float sdf = dist - u_radius;

    float alpha = 1.0 - smoothstep(-u_feather, u_feather, sdf);

    if (alpha <= 0.0) discard;

    frag_color = vec4(u_color.rgb, u_color.a * alpha);
}