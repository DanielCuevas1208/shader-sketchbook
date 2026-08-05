#version 120

uniform vec2 u_resolution;
uniform float u_time;
uniform float u_shape_blend;
uniform float u_sphere_radius;
uniform float u_light_energy;

varying vec2 v_uv;

float op_smin(float left, float right, float width) {
    float safe_width = max(width, 0.0001);
    float blend = clamp(0.5 + 0.5 * (right - left) / safe_width, 0.0, 1.0);
    return mix(right, left, blend) - safe_width * blend * (1.0 - blend);
}

float sd_box(vec3 point, vec3 half_size) {
    vec3 distance = abs(point) - half_size;
    return length(max(distance, 0.0)) + min(max(distance.x, max(distance.y, distance.z)), 0.0);
}

float scene_distance(vec3 point) {
    float radius = max(u_sphere_radius, 0.1);
    float sphere = length(point) - radius;
    float box = sd_box(point - vec3(0.55, 0.0, 0.0), vec3(0.42));
    float blend_width = 0.06 + clamp(u_shape_blend, 0.0, 1.0) * 0.42;
    return op_smin(sphere, box, blend_width);
}

vec3 scene_normal(vec3 point) {
    vec2 offset = vec2(0.0025, 0.0);
    return normalize(vec3(
        scene_distance(point + offset.xyy) - scene_distance(point - offset.xyy),
        scene_distance(point + offset.yxy) - scene_distance(point - offset.yxy),
        scene_distance(point + offset.yyx) - scene_distance(point - offset.yyx)));
}

void main() {
    vec2 centered = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / u_resolution.y;
    vec3 direction = normalize(vec3(centered.x, centered.y, 1.35));
    direction.xz = mat2(cos(u_time * 0.25), -sin(u_time * 0.25), sin(u_time * 0.25), cos(u_time * 0.25)) * direction.xz;

    float travelled = 0.0;
    vec3 point = vec3(0.0);
    bool hit = false;
    for (int step = 0; step < 80; ++step) {
        point = direction * travelled;
        float distance = scene_distance(point);
        if (distance < 0.0015) {
            hit = true;
            break;
        }
        travelled += distance;
        if (travelled > 10.0) {
            break;
        }
    }

    vec3 background = vec3(0.03, 0.04, 0.10);
    if (!hit) {
        gl_FragColor = vec4(background, 1.0);
        return;
    }

    vec3 normal = scene_normal(point);
    vec3 light_direction = normalize(vec3(-0.35, 0.85, 0.45));
    float diffuse = max(dot(normal, light_direction), 0.0);
    float energy = max(u_light_energy, 0.0);
    vec3 base_color = vec3(0.18, 0.55, 0.92);
    vec3 color = base_color * (0.22 + diffuse * energy);
    color = color / (1.0 + color);
    gl_FragColor = vec4(color, 1.0);
}
