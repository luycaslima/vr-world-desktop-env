#version 450

layout(set = 1, binding = 0) uniform PushConstants {
    mat4 u_mvp;
    //mat4 u_view_projection;
    //vec2 u_center;
    //vec2 u_padding1; // Align u_color (vec4) to 16-byte boundary
    vec4 u_color;
    float u_radius;
    float u_feather;
    vec2 u_padding2; // Pad block size to multiple of 16
} pc;

layout(location = 0) out vec2 v_local_pos; // Normalized [-1, 1] range

// Procedural Quad via gl_VertexIndex (6 vertices: 2 triangles)
const vec2 quad_offsets[6] = vec2[](
    vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2(-1.0,  1.0),
    vec2(-1.0,  1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0)
);

void main() {
    vec2 offset = quad_offsets[gl_VertexIndex];
    v_local_pos = offset;

    // Expand unit quad [-1, 1] to pixel size (radius + feather)
    float extent = pc.u_radius + pc.u_feather;
    vec2 local_pixel_pos = offset ;

    gl_Position = pc.u_mvp * vec4(local_pixel_pos, 0.0, 1.0);
}