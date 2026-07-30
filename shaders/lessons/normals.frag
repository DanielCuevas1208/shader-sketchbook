#version 120

uniform vec2 u_resolution;
uniform float u_time;
uniform float u_shape_blend;
uniform float u_sphere_radius;
uniform float u_repeat;
uniform float u_light_energy;

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
    float box = sd_box(point - vec3(0.62, 0.05, 0.0), vec3(0.48));
    float shape = op_smin(sphere, box, 0.08 + clamp(u_shape_blend, 0.0, 1.0) * 0.32);
    return min(shape, point.y + 1.02);
}

vec3 scene_normal(vec3 point) {
    vec2 offset = vec2(0.0025, 0.0);
    return normalize(vec3(
        scene_distance(point + offset.xyy) - scene_distance(point - offset.xyy),
        scene_distance(point + offset.yxy) - scene_distance(point - offset.yxy),
        scene_distance(point + offset.yyx) - scene_distance(point - offset.yyx)));
}

vec3 shade(vec3 origin, vec3 direction) {
    float travelled = 0.0;
    vec3 point = origin;
    bool hit = false;
    for (int step = 0; step < 96; ++step) {
        point = origin + direction * travelled;
        float distance = scene_distance(point);
        if (distance < 0.0015) {
            hit = true;
            break;
        }
        travelled += distance;
        if (travelled > 12.0) {
            break;
        }
    }

    if (!hit) {
        return vec3(0.02, 0.03, 0.08) + vec3(0.03, 0.02, 0.05) * (direction.y + 1.0);
    }

    vec3 normal = scene_normal(point);
    vec3 light_direction = normalize(vec3(-0.45, 0.88, 0.38));
    float diffuse = max(dot(normal, light_direction), 0.0) * max(u_light_energy, 0.0);
    float horizon = 0.5 + 0.5 * normal.y;
    float repeat_scale = 1.8 + 0.2 / max(u_repeat, 0.6);
    float pulse = 0.5 + 0.5 * sin(u_time * 0.6 + point.x * repeat_scale);
    vec3 normal_color = 0.5 + 0.5 * normal;
    vec3 color = mix(vec3(0.05, 0.10, 0.22), normal_color, 0.75);
    color *= 0.35 + diffuse * 0.65 + horizon * 0.18;
    color += vec3(0.10, 0.18, 0.45) * pulse * 0.12;
    color = color / (1.0 + color);
    return pow(max(color, 0.0), vec3(1.0 / 2.2));
}

void main() {
    vec2 centered = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / u_resolution.y;
    vec3 camera = vec3(3.45, 1.85, 4.35);
    vec3 target = vec3(0.0, -0.08, 0.0);
    vec3 forward = normalize(target - camera);
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, forward);
    vec3 direction = normalize(forward + right * centered.x * 0.88 + up * centered.y * 0.88);
    gl_FragColor = vec4(shade(camera, direction), 1.0);
}
