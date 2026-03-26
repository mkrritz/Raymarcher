#version 430 core

#define PI    3.1415925
#define TWOPI 6.2831850
#define ONE_CENTIMETER 0.01
#define TEN_CENTIMETER 0.1

in vec2 UV; // This useless?
in vec3 Direction;
out vec4 FragColor;

#include "UniformBufferObjects/Timers.glsl"
layout(std140, binding = 2) uniform SceneUniformBufferObject {
    TimersUBO Scene;
};

layout(binding = 4) uniform sampler2D Sampler;

#include "UniformBufferObjects/Camera.glsl"
layout(std140, binding = 1) uniform CameraUniformBufferObject {       // Figure out naming of camera position and camera stuff
    CameraUBO Camera;                                                 // Figure out naming of camera position and camera stuff
};                                                                    // Figure out naming of camera position and camera stuff
                                                                      // Figure out naming of camera position and camera stuff
layout(std430, binding = 6) readonly buffer CameraPositionBuffer {    // Figure out naming of camera position and camera stuff
    vec4 CameraPosition;                                              // Figure out naming of camera position and camera stuff
};                                                                    // Figure out naming of camera position and camera stuff

layout(std430, binding = 8) readonly buffer VolumeBuffer {
    float VolumeData[];
};

float SampleVolume(vec3 pos) {
    ivec3 ipos = ivec3(floor(pos));
    ipos = clamp(ipos, ivec3(0), ivec3(255));
    
    int index = ipos.x + ipos.y * 256 + ipos.z * 256 * 256;
    return VolumeData[index];
}

const vec2 RegionMin = vec2(0.0, 0.0);
const vec2 RegionMax = vec2(256.0, 256.0);

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

vec3 GetSunPosition() {
    float speed = 2.0;
    return vec3(150,100,150) +
           vec3(100.0*cos(Scene.Timers.x * speed),
                150.0*cos(Scene.Timers.x * speed),
                100.0*sin(Scene.Timers.x * speed));
}

float SphereSDF(vec3 Position, vec3 Center) {
    return length(Position - Center) - 10;
}

float GetSceneSDF(vec3 Position) {
    float Terrain = Position.y - SampleMap(Position.xz);
    float Sphere  = SphereSDF(Position, GetSunPosition());
    
    // VOLUME, BRING A BETTER VOLUME
    // Region offset: Add multiples of 256.0 to shift the volume to different block regions
    vec3 RegionOffset = vec3(0.0, 0.0, 0.0);
    vec3 LocalPosition = Position - RegionOffset;
    // Evaluate the volume specifically for this region
    float VolumeDist = SampleVolume(LocalPosition);
    // Create a bounding box distance field for the region from [0, 256]
    // Center is (128,128,128), and Half-Size is (128,128,128)
    vec3 d = abs(LocalPosition - vec3(128.0)) - vec3(128.0);
    float BoxDist = length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
    // Combining a volume with a bounding box uses max() for intersection!
    float BoundedVolume = max(BoxDist, VolumeDist);
    
    return min(min(Terrain, Sphere), BoundedVolume);
    //return min(Terrain, Sphere);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void March(out Surface surface) {
    surface = Surface(CameraPosition.xyz,
                      vec3(0.0, 1.0, 0.0),
                      0.0,
                      false);
    
    const vec3 Dir = normalize(Direction);
    float Distance = 0.0;
    
    for(int Iteration = 0; Iteration < 64; Iteration++) {
        if(Distance > Camera.Attributes.z + 1000) break;
        
        vec3 Position = CameraPosition.xyz + Dir * Distance;
        
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
vec3 TerrainMaterial(vec3);
#endif

void main() {
    vec3 Color = vec3(0);
    
    Surface surface;
    March(surface);
    
    if(surface.hit) {
        surface.Normal = SceneNormal(surface.Position);
        vec3 baseColor = SceneMaterial(surface.Position);
        
        vec3 Sun = normalize(GetSunPosition());
        float diffuse = max(4*dot(surface.Normal, Sun), 0.25);
        Color = baseColor * diffuse;
    } else {
        float x = (sin(Scene.Timers.x) + 1.0) * 0.5;
        Color = mix(vec3(x,0.5,0.2), vec3(0.6-0.3*x,0.3-0.2*x,0.5), UV.y);
    }
    
    FragColor = vec4(Color, 1.0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////



vec3 TerrainMaterial(vec3 Position) {
    float h = Position.y;
    float slope = max(0.4, dot(SceneNormal(Position), vec3(0.0,1.0,0.0)));

    vec3 water = vec3(0.0,0.2,0.6);
    vec3 dirt  = vec3(0.4,0.3,0.1);
    vec3 grass = vec3(0.1,0.5,0.1);
    vec3 rock  = vec3(0.25,0.25,0.25);
    vec3 ice  = vec3(0.9,0.9,0.9);

    vec3 base;
    if(h < 3.0 + 0.1 * cos(Scene.Timers.x)) base = dirt;
    else if(h < 6.0 + 0.1 * cos(Scene.Timers.x + PI)) base = water;
    else if(h < 12.0) base = dirt;
    else if(h < 14.0) base = grass;
    else if(h < 16.0) base = rock;
    else if(h < 100.0) base = dirt;
    else base = ice;

    return base / max(slope, 0.01);
}

vec3 SceneMaterial(vec3 Position) {
    float Terrain = Position.y - SampleMap(Position.xz);
    float Sphere  = SphereSDF(Position, GetSunPosition());
    
    if(Sphere < Terrain) {
        return vec3(1.0, 0.9, 0.6);
    }

    return TerrainMaterial(Position);
}

vec3 SceneNormal(vec3 Position) {
    const float eps = ONE_CENTIMETER;
    vec3 n;
    n.x = GetSceneSDF(Position + vec3(eps,0,0)) - GetSceneSDF(Position - vec3(eps,0,0));
    n.y = GetSceneSDF(Position + vec3(0,eps,0)) - GetSceneSDF(Position - vec3(0,eps,0));
    n.z = GetSceneSDF(Position + vec3(0,0,eps)) - GetSceneSDF(Position - vec3(0,0,eps));
    return normalize(n);
}
