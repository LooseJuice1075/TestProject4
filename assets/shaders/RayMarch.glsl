#type vertex
#version 450 core

layout (location = 0) in vec3 a_Position;

layout (std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

void main()
{
    gl_Position = vec4(a_Position.x, a_Position.y, a_Position.z, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

precision highp float;

uniform vec2 u_resolution;// Width & height of the shader
uniform int u_time;// Time elapsed

// Constants
#define MAX_OBJECTS 30
#define OBJECT_SIZE 18 // Number of floats per object
#define PI 3.1415925359
#define MAX_STEPS 100// Mar Raymarching steps
#define MAX_DIST 55.// Max Raymarching distance
#define SURF_DIST.01// Surface Distance

#define SPHERE 0
#define CUBE 1
#define TEXTURED_BOX 2
#define CYLINDER 3

layout (binding = 0) uniform sampler2D u_ObjectBufferTex;

uniform float u_objects[MAX_OBJECTS * OBJECT_SIZE];
uniform int u_object_elements_count;

uniform vec4 u_ray_origin;
uniform vec3 u_ray_direction;
uniform float u_fov;

uniform float u_light_angle;

uniform vec4 u_floor_color;

const vec4 BoxColor = vec4(1,0,0,1);
const vec4 SphereColor = vec4(0,1,0,1);
const vec4 GroundColor = vec4(1);
 
float colorIntensity = 1.;
vec3 difColor = vec3(1.0, 1.0, 1.0); // Diffuse Color
 
mat2 Rotate(float a) {
    float s=sin(a); 
    float c=cos(a);
    return mat2(c,-s,s,c);
}

// VORONOI
//vec2 random2(vec2 p) {
//    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
//}
//
//float cellular(vec2 p) {
//    vec2 i_st = floor(p);
//    vec2 f_st = fract(p);
//    float m_dist = 10.;
//    for (int j=-1; j<=1; j++ ) {
//        for (int i=-1; i<=1; i++ ) {
//            vec2 neighbor = vec2(float(i),float(j));
//            vec2 point = random2(i_st + neighbor) * sin(u_time);
//            point = 0.5 + 0.5*sin(6.2831*point);
//            vec2 diff = neighbor + point - f_st;
//            float dist = length(diff);
//            if( dist < m_dist ) {
//                m_dist = dist;
//            }
//        }
//    }
//    return m_dist;
//}
// VORONOI

// FBM
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitud = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitud * noise(st);
        st *= 2.;
        amplitud *= .5;
    }
    return value;
}
// FBM
 
///////////////////////
// Primitives
///////////////////////
 
// Sphere - exact
float sphereSDF( vec3 p, float s ) {
  float amplitude = 10;
  float period = 0.4;
  float speed = 0.008;
  float sinMagic = (sin(p.x * amplitude + u_time * speed) * period) * (sin(p.y * amplitude + u_time * speed) * period) * (sin(p.z * amplitude + u_time * speed) * period);
  return (length(p)-s) - sinMagic;
}

// Box - exact
float boxSDF( vec3 p, vec3 b ) {
  vec3 q = abs(p) - b;
  //return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - fbm(p.xz + u_time * 0.6) * 0.2;
  float amplitude = 10;
  float period = 0.4;
  float sinMagic = (sin(p.x * amplitude + u_time) * period) * (sin(p.y * amplitude + u_time) * period) * (sin(p.z * amplitude + u_time) * period);
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}
 
// Plane - exact
float planeSDF(vec3 p,vec4 n){
    // n must be normalized
    //return dot(p,n.xyz)+n.w - fbm(p.xz + u_time * 0.6) * 0.2;
    return dot(p,n.xyz)+n.w;
}
 
// Triangular Prism - exact
float triPrismSDF(vec3 p,vec2 h){
    const float k=sqrt(3.);
    h.x*=.5*k;
    p.xy/=h.x;
    p.x=abs(p.x)-1.;
    p.y=p.y+1./k;
    if(p.x+k*p.y>0.)p.xy=vec2(p.x-k*p.y,-k*p.x-p.y)/2.;
    p.x-=clamp(p.x,-2.,0.);
    float d1=length(p.xy)*sign(-p.y)*h.x;
    float d2=abs(p.z)-h.y;
    return length(max(vec2(d1,d2),0.))+min(max(d1,d2),0.);
}
 
// Rounded Cylinder - exact
float roundedCylinderSDF(vec3 p,float ra,float rb,float h){
    vec2 d=vec2(length(p.xz)-2.*ra+rb,abs(p.y)-h);
    return min(max(d.x,d.y),0.)+length(max(d,0.))-rb;
}
     
///////////////////////
// Boolean Operators
///////////////////////
 
vec4 intersectSDF(vec4 a, vec4 b) {
    float d = max(a.w, b.w);
    return d == a.w? a : b;
}
  
vec4 unionSDF(vec4 a, vec4 b) {
    float d = min(a.w, b.w);
    return d == a.w? a : b;
}

vec4 smoothUnionSDF(vec4 a, vec4 b, float k ) 
{
  float h = clamp(0.5 + 0.5*(a.w-b.w)/k, 0., 1.);
  vec3 c = mix(a.rgb,b.rgb,h);
  float d = mix(a.w, b.w, h) - k*h*(1.-h); 
   
  return vec4(c,d);
}

vec4 differenceSDF(vec4 a, vec4 b) {
    float d = max(a.w, -b.w);
    return d == a.w ? a : vec4(b.rgb,-b.w);
}

void GetObjectArray(inout float objects[OBJECT_SIZE], int row)
{
    ivec2 objectIndex = ivec2(0, row);

    for (int i = 0; i < OBJECT_SIZE; i++)
    {
        objectIndex.x = i;
        objects[i] = texelFetch(u_ObjectBufferTex, objectIndex, 0).r;
    }
}

float GetObjectElement(int row, int column)
{
    ivec2 objectIndex = ivec2(column, row);
    return texelFetch(u_ObjectBufferTex, objectIndex, 0).r;
}

vec4 GetDist(vec3 p, out int out_entityID)
{
    // Scene 
    vec4 scene = vec4(0);

    // Box
    //vec3 bSize = vec3(.8,.8,.8); //box size
    //vec3 bPos = vec3(0,1,0); // box position
    //bPos = p-bPos;
    //bPos.xz*=Rotate(-u_time);
    //bPos.xy*=Rotate(-u_time);
    //vec4 b0 = vec4(BoxColor.rgb,boxSDF(bPos,bSize));
   
    // Sphere.
    //vec3 sPos=vec3(0,1,0);
    //sPos=p-sPos;
    //sPos.xz*=Rotate(-u_time);
    //sPos.xy*=Rotate(-u_time);
    //vec4 s0 = vec4(SphereColor.rgb,sphereSDF(sPos,1.));
     
    // Plane
    float planeScale = 1;
    vec2 q = floor(p.xz * planeScale);
    vec3 planeColor = vec3(mod(q.x + q.y, 2.0));
    //planeColor *= u_floor_color.rgb;

    float mixfac = -(planeColor.r + 1) + 2;
    planeColor = mix(planeColor, u_floor_color.rgb, mixfac);
    //planeColor = vec3(mixfac);

    //planeColor.r += 0.7;
    //planeColor.b += 1.0;

    vec4 p0 = vec4(planeColor.rgb,planeSDF(p,vec4(0,1,0,0)));

    scene = p0;
    int row = 0;

    for (int i = 0; i < u_object_elements_count / OBJECT_SIZE; i += 1)
    {
        float objects[OBJECT_SIZE] = float[OBJECT_SIZE](0.0);
        GetObjectArray(objects, row);
        
        float type = objects[0];
        vec3 translation = vec3(objects[1], objects[2], -objects[3]);
        vec3 rotation = vec3(-objects[4], objects[5], objects[6]);
        vec3 scale = vec3(objects[7], objects[8], objects[9]);
        vec4 color = vec4(objects[10], objects[11], objects[12], objects[13]);
        float blending = objects[14];
        float entityID = objects[15];

        out_entityID = -1;

        if (type == SPHERE)
        {
            translation = p-translation;
            vec4 s0 = vec4(color.rgb,sphereSDF(translation,objects[16]));

            scene = smoothUnionSDF(scene, s0, blending);
        }
        else if (type == CUBE)
        {
            vec3 bSize = scale; //box size
            translation = p-translation;

            translation.yz *= Rotate(radians(rotation.x));
            translation.xz *= Rotate(radians(rotation.y));
            translation.xy *= Rotate(radians(rotation.z));

            vec2 uv_xy = p.xy - vec2(1,0);
            vec2 uv_yz = p.yz;
            vec2 uv_xz = p.xy - vec2(1,0);
            float scale = 25.0;
            vec3 boxColor = vec3(fbm((uv_xy + u_time * 0.0001) * scale), fbm((uv_xy + u_time * 0.0001) * scale), fbm((uv_xz + u_time * 0.0001) * scale));
            boxColor *= color.rgb;

            //vec3 bq = floor(p * 2);
            //boxColor = vec3(mod(bq.x + bq.y + bq.z, 2.0));
            vec4 b0 = vec4(boxColor, boxSDF(translation,bSize));
            
            scene = smoothUnionSDF(scene, b0, blending);
        }
        else if (type == TEXTURED_BOX)
        {
            vec3 bSize = scale; //box size
            translation = p-translation;

            translation.yz *= Rotate(radians(rotation.x));
            translation.xz *= Rotate(radians(rotation.y));
            translation.xy *= Rotate(radians(rotation.z));

            vec2 uv_xy = p.xy - vec2(1,0);
            vec2 uv_yz = p.yz;
            vec2 uv_xz = p.xy - vec2(1,0);
            float texture_scale = objects[17];

            vec3 bq = floor(p * texture_scale);
            vec3 boxColor = vec3(mod(bq.x + bq.y + bq.z, 2.0));
            float textured_box_mixfac = -(boxColor.r + 1) + 2;
            boxColor = mix(boxColor, color.rgb, textured_box_mixfac);
            vec4 b0 = vec4(boxColor, boxSDF(translation,bSize));
            
            scene = smoothUnionSDF(scene, b0, blending);
        }
        else if (type == CYLINDER)
        {
            translation = p-translation;

            translation.yz *= Rotate(radians(rotation.x));
            translation.xz *= Rotate(radians(rotation.y));
            translation.xy *= Rotate(radians(rotation.z));

            vec4 c0 = vec4(color.rgb, roundedCylinderSDF(translation, 0.1, 0.1, objects[16]));

            scene = smoothUnionSDF(scene, c0, blending);
        }

        row++;
    }
 
    return scene;
}

float RayMarch(vec3 ro,vec3 rd, inout vec3 difColor, out int entityID)
{
    float dO=0.;//Distance Origin
    for(int i=0;i<MAX_STEPS;i++)
    {
        if(dO>MAX_DIST)
            break;
 
        vec3 p=ro+rd*dO;
        vec4 ds=GetDist(p, entityID);// ds is Distance Scene
 
        if(ds.w<SURF_DIST)
        {
            difColor = ds.rgb;
            break;
        }
        dO+=ds.w;
         
    }
    return dO;
}

vec3 GetNormal(vec3 p)
{
    int entityID;

    float d=GetDist(p, entityID).w;// Distance
    vec2 e=vec2(.01,0);// Epsilon
     
    vec3 n=d-vec3(
        GetDist(p-e.xyy, entityID).w,// e.xyy is the same as vec3(.01,0,0). The x of e is .01. this is called a swizzle
        GetDist(p-e.yxy, entityID).w,
        GetDist(p-e.yyx, entityID).w);
         
    return normalize(n);
}
                                 
vec3 GetLight(vec3 p, vec3 c)
{
    // Diffuse Color
    vec3 color = c.rgb * colorIntensity;
 
    // Directional light
    vec3 lightPos=vec3(5.*sin(radians(u_light_angle)),5.,5.*cos(radians(u_light_angle)));// Light Position
 
    vec3 l=normalize(lightPos-p);// Light Vector
    vec3 n=GetNormal(p);// Normal Vector
     
    float dif=dot(n,l);// Diffuse light
    dif=clamp(dif,0.4,1.);// Clamp so it doesnt go below 0
     
    // Shadows
    int entityID;
    float d=RayMarch(p+n*SURF_DIST*2.,l,difColor, entityID);
     
    // Re-enable this for self-shadowing
    //if(d<length(lightPos-p))dif*=.1;
     
    return color * dif;
}

void main()
{
    vec2 uv=(gl_FragCoord.xy-.5*u_resolution.xy)/u_resolution.y;
     
    vec3 ro = vec3(u_ray_origin.x, u_ray_origin.y, -u_ray_origin.z); // Ray Origin/Camera position
    vec3 rd = normalize(vec3(uv.x,uv.y,u_fov)); // Ray Direction
    
    rd.zy *= Rotate(-radians(u_ray_direction.x)); // Rotate camera down on the x-axis
    rd.zx *= Rotate(radians(u_ray_direction.y)); // Rotate camera down on the y-axis
    rd.xy *= Rotate(radians(u_ray_direction.z)); // Rotate camera down on the z-axis
    
    int entityID;
    float d=RayMarch(ro, rd, difColor, entityID);// Distance
     
    vec3 p=ro+rd*d;
    vec3 color=GetLight(p,difColor);// Diffuse lighting
     
    // Set the output color
    o_Color = vec4(color, 1.0);

    o_EntityID = entityID;

    //ivec2 objectIndex = ivec2(gl_FragCoord.x, gl_FragCoord.y);
    //float pixel = texelFetch(u_ObjectBufferTex, objectIndex, 0).r;
    //
    //o_Color=vec4(pixel, 0.0, 0.0, 1.0);
}
