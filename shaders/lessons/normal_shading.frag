#version 120

uniform vec2 u_resolution;
uniform float u_time;
uniform float u_light_energy;

varying vec2 v_uv;

float sd_torus(vec3 point, vec2 radii) {
    vec2 q = vec2(length(point.xz) - radii.x, point.y);
    return length(q) - radii.y;
}

float scene_distance(vec3 point) {
    vec3 rotated = point;
    rotated.xz = mat2(cos(u_time * 0.30), -sin(u_time * 0.30), sin(u_time * 0.30), cos(u_time * 0.30)) * rotated.xz;
    return sd_torus(rotated, vec2(0.72, 0.22));
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
    vec3 direction = normalize(vec3(centered.x, centered.y, 1.50));

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

    vec3 background = vec3(0.03, 0.04, 0.09);
    if (!hit) {
        gl_FragColor = vec4(background, 1.0);
        return;
    }

    vec3 normal = scene_normal(point);
    vec3 light_direction = normalize(vec3(-0.50, 0.75, 0.40));
    float diffuse = max(dot(normal, light_direction), 0.0);
    float ambient = 0.18 + 0.22 * clamp(normal.y, 0.0, 1.0);
    float energy = max(u_light_energy, 0.0);
    vec3 base_color = vec3(0.85, 0.42, 0.28);
    vec3 color = base_color * (ambient + diffuse * energy);
    color = color / (1.0 + color);
    gl_FragColor = vec4(color, 1.0);
}
