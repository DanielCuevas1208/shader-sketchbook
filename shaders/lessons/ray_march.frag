#version 120

uniform vec2 u_resolution;
uniform float u_time;
uniform float u_sphere_radius;

varying vec2 v_uv;

float scene_distance(vec3 point) {
    float radius = max(u_sphere_radius, 0.1);
    return length(point) - radius;
}

void main() {
    vec2 centered = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / u_resolution.y;
    vec3 direction = normalize(vec3(centered.x, centered.y, 1.40));

    float travelled = 0.0;
    int steps = 0;
    vec3 point = vec3(0.0);
    bool hit = false;
    for (int step = 0; step < 96; ++step) {
        point = direction * travelled;
        float distance = scene_distance(point);
        steps = step;
        if (distance < 0.0015) {
            hit = true;
            break;
        }
        travelled += distance;
        if (travelled > 12.0) {
            break;
        }
    }

    vec3 background = vec3(0.04, 0.05, 0.12);
    if (!hit) {
        float march_glow = float(steps) / 96.0;
        gl_FragColor = vec4(background + vec3(0.02, 0.04, 0.10) * march_glow, 1.0);
        return;
    }

    float step_tint = float(steps) / 96.0;
    vec3 hit_color = vec3(0.15 + step_tint * 0.35, 0.55 - step_tint * 0.20, 0.90);
    hit_color = hit_color / (1.0 + hit_color);
    gl_FragColor = vec4(hit_color, 1.0);
}
