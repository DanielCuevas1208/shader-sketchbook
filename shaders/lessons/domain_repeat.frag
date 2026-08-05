#version 120

uniform vec2 u_resolution;
uniform float u_time;
uniform float u_repeat;
uniform float u_sphere_radius;
uniform float u_light_energy;

varying vec2 v_uv;

vec3 centered_repeat(vec3 point, float cell) {
    return mod(point + cell * 0.5, cell) - cell * 0.5;
}

float scene_distance(vec3 point) {
    float cell = max(u_repeat, 0.6);
    float radius = max(u_sphere_radius, 0.1) * 0.28;
    vec3 repeated = centered_repeat(point, cell);
    repeated.y += 0.12 * sin(u_time * 0.6 + point.x * 0.4);
    return length(repeated) - radius;
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
    vec3 direction = normalize(vec3(centered.x, centered.y, 1.45));
    direction.xz = mat2(cos(u_time * 0.18), -sin(u_time * 0.18), sin(u_time * 0.18), cos(u_time * 0.18)) * direction.xz;

    float travelled = 0.0;
    vec3 point = vec3(0.0);
    bool hit = false;
    for (int step = 0; step < 96; ++step) {
        point = direction * travelled;
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

    vec3 background = vec3(0.02, 0.03, 0.08);
    if (!hit) {
        gl_FragColor = vec4(background, 1.0);
        return;
    }

    vec3 normal = scene_normal(point);
    vec3 light_direction = normalize(vec3(-0.40, 0.80, 0.35));
    float diffuse = max(dot(normal, light_direction), 0.0);
    float energy = max(u_light_energy, 0.0);
    float band = 0.5 + 0.5 * sin(point.x * 2.1 + point.z * 2.1);
    vec3 base_color = mix(vec3(0.12, 0.42, 0.78), vec3(0.22, 0.72, 0.58), band);
    vec3 color = base_color * (0.20 + diffuse * energy);
    color = color / (1.0 + color);
    gl_FragColor = vec4(color, 1.0);
}
