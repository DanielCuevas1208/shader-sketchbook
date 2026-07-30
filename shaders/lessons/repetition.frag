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

vec3 centered_repeat(vec3 point, float cell) {
    return mod(point + cell * 0.5, cell) - cell * 0.5;
}

float scene_distance(vec3 point) {
    float radius = max(u_sphere_radius, 0.1);
    float cell = max(u_repeat, 0.6);
    vec3 repeated = centered_repeat(point, cell);
    float satellite = length(repeated + vec3(0.0, 0.08, 0.0)) - radius * 0.34;
    float core = length(point) - radius;
    return min(op_smin(core, satellite, 0.04 + clamp(u_shape_blend, 0.0, 1.0) * 0.12), point.y + 1.02);
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
        return vec3(0.02, 0.025, 0.09) + vec3(0.04, 0.02, 0.07) * (direction.y + 1.0);
    }

    vec3 normal = scene_normal(point);
    vec3 light_direction = normalize(vec3(-0.45, 0.88, 0.38));
    vec3 view_direction = -direction;
    float diffuse = max(dot(normal, light_direction), 0.0) * max(u_light_energy, 0.0);
    float rim = pow(1.0 - max(dot(normal, view_direction), 0.0), 3.0);
    vec3 cell = centered_repeat(point, max(u_repeat, 0.6));
    float stripe = 0.5 + 0.5 * sin((cell.x + cell.z) * 7.0 + u_time * 0.8);
    vec3 color = mix(vec3(0.06, 0.15, 0.36), vec3(0.18, 0.72, 0.86), stripe);
    color *= 0.24 + diffuse * 0.72;
    color += vec3(0.08, 0.18, 0.48) * rim * 0.45;
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
    float orbit = sin(u_time * 0.32) * 0.035;
    direction.xz = mat2(cos(orbit), -sin(orbit), sin(orbit), cos(orbit)) * direction.xz;
    gl_FragColor = vec4(shade(camera, direction), 1.0);
}
