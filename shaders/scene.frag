#version 120

uniform vec2 u_resolution;
uniform float u_time;
uniform float u_shape_blend;
uniform float u_sphere_radius;
uniform float u_repeat;
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

vec3 centered_repeat(vec3 point, float cell) {
    return mod(point + cell * 0.5, cell) - cell * 0.5;
}

float object_distance(vec3 point) {
    float radius = max(u_sphere_radius, 0.1);
    float cell = max(u_repeat, 0.6);
    float sphere = length(point) - radius;
    float box = sd_box(point - vec3(0.62, 0.05, 0.0), vec3(0.48));
    vec3 repeated = centered_repeat(point, cell);
    repeated.y += 0.08;
    float satellite = length(repeated) - radius * 0.34;
    float fused = op_smin(sphere, box, 0.08 + clamp(u_shape_blend, 0.0, 1.0) * 0.32);
    return op_smin(fused, satellite, 0.04 + clamp(u_shape_blend, 0.0, 1.0) * 0.12);
}

float scene_distance(vec3 point) {
    return min(object_distance(point), point.y + 1.02);
}

vec3 scene_normal(vec3 point) {
    vec2 offset = vec2(0.0025, 0.0);
    return normalize(vec3(
        scene_distance(point + offset.xyy) - scene_distance(point - offset.xyy),
        scene_distance(point + offset.yxy) - scene_distance(point - offset.yxy),
        scene_distance(point + offset.yyx) - scene_distance(point - offset.yyx)));
}

vec3 tone_map(vec3 color) {
    color = color / (1.0 + color);
    return pow(max(color, 0.0), vec3(1.0 / 2.2));
}

vec3 shade(vec3 origin, vec3 direction) {
    vec3 background = vec3(0.015 + 0.035 * (direction.y + 1.0), 0.025, 0.07 + 0.08 * (direction.y + 1.0));
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
        return background;
    }

    vec3 normal = scene_normal(point);
    vec3 light_direction = normalize(vec3(-0.45, 0.88, 0.38));
    vec3 view_direction = -direction;
    vec3 half_direction = normalize(light_direction + view_direction);
    float diffuse = max(dot(normal, light_direction), 0.0);
    float specular = pow(max(dot(normal, half_direction), 0.0), 48.0);
    float ambient = 0.18 + 0.18 * clamp(normal.y, 0.0, 1.0);
    float occlusion = clamp(1.0 - 7.0 * scene_distance(point + normal * 0.08), 0.0, 1.0);

    vec3 base_color;
    if (point.y < -0.99) {
        float tile = mod(abs(floor(point.x * 1.6) + floor(point.z * 1.6)), 2.0);
        base_color = tile < 1.0 ? vec3(0.06, 0.08, 0.14) : vec3(0.09, 0.12, 0.20);
    } else {
        float hue = 0.5 + 0.5 * sin((point.x + point.z) * (2.3 + u_shape_blend));
        base_color = vec3(0.08 + 0.42 * hue, 0.50 - 0.20 * hue, 0.82 + 0.10 * hue);
    }

    float energy = max(u_light_energy, 0.0);
    vec3 color = base_color * (ambient + diffuse * energy) * occlusion;
    color += vec3(0.18, 0.23, 0.38) * specular * energy;
    float rim = pow(1.0 - max(dot(normal, view_direction), 0.0), 3.0);
    color += vec3(0.12, 0.25, 0.55) * rim * 0.32;
    return tone_map(color);
}

void main() {
    vec2 centered = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / u_resolution.y;
    vec3 camera = vec3(3.45, 1.85, 4.35);
    vec3 target = vec3(0.0, -0.08, 0.0);
    vec3 forward = normalize(target - camera);
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, forward);
    vec3 direction = normalize(forward + right * centered.x * 0.88 + up * centered.y * 0.88);

    float orbit = sin(u_time * 0.32) * 0.035;
    direction.xz = mat2(cos(orbit), -sin(orbit), sin(orbit), cos(orbit)) * direction.xz;
    vec3 color = shade(camera, direction);
    float vignette = 1.0 - 0.22 * smoothstep(0.35, 1.25, length(centered));
    gl_FragColor = vec4(color * vignette, 1.0);
}
