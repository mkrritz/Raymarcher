#version 430 core

#define PI    3.1415925
#define TWOPI 6.2831850
#define ONE_CENTIMETER 0.01
#define TEN_CENTIMETER 0.1

in vec2 UV; // This useless?
in vec3 Direction;
out vec4 FragColor;

layout(location = 2)  uniform float Time;
layout(location = 3)  uniform float Far;
layout(binding = 4) uniform sampler2D Sampler;

layout(std430, binding = 6) readonly buffer CameraPosition {
    vec4 Camera;
};

const vec2 RegionMin = vec2(0.0, 0.0);
const vec2 RegionMax = vec2(300.0, 300.0);

struct Surface {
    vec3 Position;
    vec3 Normal;
    float Distance;
    bool hit;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

vec2 WorldToUV(vec2 Position) {
    return clamp((Position - RegionMin) / (RegionMax - RegionMin), 0.0, 1.0);
    //return fract((Position - RegionMin) / (RegionMax - RegionMin));
}

float SampleMap(vec2 Position) {
    vec2 uv = WorldToUV(Position);
    return texture(Sampler, uv).r * 20.0; // Fix this!
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

float GetSceneSDF(vec3 Position) {
    float Terrain = Position.y - SampleMap(Position.xz);
    return Terrain;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void March(out Surface surface) {
    surface = Surface(Camera.xyz,
                      vec3(0.0, 1.0, 0.0),
                      0.0,
                      false);
    
    const vec3 Dir = normalize(Direction);
    float Distance = 0.0;
    
    for(int Iteration = 0; Iteration < 64; Iteration++) {
        if(Distance > Far + 1000) break;
        
        vec3 Position = Camera.xyz + Dir * Distance;
        
        // Closest surface in any direction
        float ClosestSurface = GetSceneSDF(Position);
        
        if(ClosestSurface < ONE_CENTIMETER) {
            surface.hit = true;
            surface.Position = Position;
            surface.Distance = Distance;
            return;
        }
        Distance += ClosestSurface;
    }
    
    surface.Distance = Distance; // discard;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1 // Some basic lighting
vec3 SceneNormal(vec3);
vec3 SceneMaterial(vec3);
vec3 TreeMaterial(vec3);
vec3 TerrainNormal(vec3);
vec3 TerrainMaterial(vec3);
#endif

void main() {
    vec3 Color = vec3(0);
    
    Surface surface;
    March(surface);
    
    if(surface.hit) {
        surface.Normal = SceneNormal(surface.Position);
        vec3 baseColor = SceneMaterial(surface.Position);
        
        vec3 Sun = normalize(vec3(cos(Time), 2.0, sin(Time)));
        float diffuse = max(dot(surface.Normal, Sun), 0.5);
        Color = baseColor * diffuse;
    } else {
        float x = (sin(Time) + 1.0) * 0.5;
        Color = mix(vec3(x,0.5,0.2), vec3(0.6-0.3*x,0.3-0.2*x,0.5), UV.y);
    }
    
    FragColor = vec4(Color, 1.0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

vec3 TerrainNormal(vec3 Position) {
    vec2 uv = WorldToUV(Position.xz); // what?
    ivec2 texSize = textureSize(Sampler, 0);
    vec2 texelSize = 2.0 / vec2(texSize);

    float hL = texture(Sampler, uv + vec2(-texelSize.x, 0.0)).r;
    float hR = texture(Sampler, uv + vec2( texelSize.x, 0.0)).r;
    float hU = texture(Sampler, uv + vec2(0.0, -texelSize.y)).r;
    float hD = texture(Sampler, uv + vec2(0.0,  texelSize.y)).r;

    vec3 dx = vec3(2.0*texelSize.x, hR - hL, 0.0);
    vec3 dz = vec3(0.0, hD - hU, 2.0*texelSize.y);
    return normalize(cross(dz, dx));
}

vec3 TerrainMaterial(vec3 Position) {
    float h = Position.y;
    float slope = max(0.4, dot(TerrainNormal(Position), vec3(0.0,1.0,0.0)));

    vec3 water = vec3(0.0,0.2,0.6);
    vec3 dirt  = vec3(0.4,0.3,0.1);
    vec3 grass = vec3(0.1,0.5,0.1);
    vec3 rock  = vec3(0.25,0.25,0.25);
    vec3 ice  = vec3(0.9,0.9,0.9);

    vec3 base;
    if(h < 3.0 + 0.1 * cos(Time)) base = dirt;
    else if(h < 6.0 + 0.1 * cos(Time + PI)) base = water;
    else if(h < 12.0) base = dirt;
    else if(h < 14.0) base = grass;
    else if(h < 16.0) base = rock;
    else if(h < 100.0) base = dirt;
    else base = ice;

    return base / max(slope, 0.01);
}

vec3 SceneMaterial(vec3 Position) {
    float Terrain = Position.y - SampleMap(Position.xz);
    return TerrainMaterial(Position);
}

vec3 SceneNormal(vec3 Position) {
    float eps = 0.1;
    vec3 n;
    n.x = GetSceneSDF(Position + vec3(eps,0,0)) - GetSceneSDF(Position - vec3(eps,0,0));
    n.y = GetSceneSDF(Position + vec3(0,eps,0)) - GetSceneSDF(Position - vec3(0,eps,0));
    n.z = GetSceneSDF(Position + vec3(0,0,eps)) - GetSceneSDF(Position - vec3(0,0,eps));
    return normalize(n);
}
