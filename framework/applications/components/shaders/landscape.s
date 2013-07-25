#ifdef GL_ES
precision highp float;
#endif

uniform int itime;

vec2 resolution = vec2(400, 300);
float time = float(itime)/10.;

const float water = 0.1;
const float scale = .2; 
float offTime = time*2.0+97.0;  // Some cool place in time.
float stime=sin(offTime);
vec3 light= normalize(vec3(.3, .5, -.3));
const float fracDistanceMult = 0.65;

vec3 getSky(in vec3 rd, in vec3 sun)
{
    float sunAmount = max( dot( rd, sun ), 0.0 );
    vec3  sky = mix( vec3(rd.y*0.3+0.7, rd.y*0.3+0.7, rd.y * 0.2+.8),
        vec3(0.9, 0.9, .8),
        sunAmount );
    return sky;
}

vec3 applyFog( in vec3  rgb,      // original color of the pixel
              in float dis,            // camera to point distance
              in vec3  rayOri,   // camera position
              in vec3  rayDir,
              in vec3 sun )          // camera to point vector
{
    float fogAmount = clamp((.15 * exp(-rayOri.y*.25) * (1.0-exp( -dis*rayDir.y*.25 ))/rayDir.y), 0.0, 1.0);
    fogAmount=clamp(fogAmount+ (exp(-(45.0-dis)*0.08))-0.1, 0.0, 1.0);
    return mix( rgb, getSky(rayDir, sun), fogAmount );
}

float random(in float a, in float b)
{
    float f = (cos(dot(vec2(a,b) ,vec2(12.9898,78.233))) * 43758.5453);
    return fract(f);
}

// A noise function...
float NoiseNonLinear(in float x, in float y) 
{
    float i = floor(x), j = floor(y);
    float u = x-i, v = y-j;

    float du = 30.*u*u*(u*(u-2.)+1.);
    float dv = 30.*v*v*(v*(v-2.)+1.);
    u=u*u*u*(u*(u*6.-15.)+10.);
    v=v*v*v*(v*(v*6.-15.)+10.);

    float a = random( i, j );
    float b = random( i+1., j);
    float c = random( i, j+1.);
    float d = random( i+1., j+1.);
    float k0 = a;
    float k1 = b-a;
    float k2 = c-a;
    float k3 = a-b-c+d;
    return (k0 + k1*u + k2*v + k3*u*v);
}

// Low Perlin level terrain search...
float terrainLow(in float x, in float y, in float thresh, in float dis)
{

    float w = 7.0;
    float v = NoiseNonLinear(x, y);
    float f = w * v;
    if (f > thresh)
    {
        w *= 0.50;
        x *= 2.55;
        y *= 2.55;
        f -= w * NoiseNonLinear(x, y);
    }
    if (f < water) f = water;
    return f;
}

// Get height map at varying subtractive Perlin noise levels...
float terrain(in float x, in float y, in float thresh, in float dis)
{
    float w = 7.0;
    float v = NoiseNonLinear(x, y);
    float f = w * v;
    float d = 65.0;

    if (f > thresh && dis < d)
    {           
        w *= 0.50;
        x *= 2.55;
        y *= 2.55;
        f -= w * NoiseNonLinear(x, y);
        d *= fracDistanceMult;    
        if (f > thresh && dis < d)
        {
            float dt = clamp((d-dis)*0.15, 0.0, 1.0);
            w *= 0.24;
            x *= 2.55;
            y *= 2.55;
            f -= w * NoiseNonLinear(x, y)*dt;
            d *= fracDistanceMult;    
            if (f > thresh && dis < d)
            {
                float dt = clamp((d-dis)*.15, 0.0, 1.0);
                w *= 0.34;
                x *= 2.55;
                y *= 2.55;
                f -= w * NoiseNonLinear(x, y) * dt;
                d *= fracDistanceMult;    
                if (f > thresh && dis < d)
                {
                    float dt = clamp((d-dis)*.15, 0.0, 1.0);
                    w *= 0.34;
                    x *= 2.55;
                    y *= 2.55;
                    f -= w * NoiseNonLinear(x, y)*dt;
                    d *= fracDistanceMult;    
                    if (f > thresh && dis < d)
                    {
                        float dt = clamp((d-dis)*.15, 0.0, 1.0);
                        w *= 0.34;
                        x *= 2.55;
                        y *= 2.55;
                        f -= w * NoiseNonLinear(x, y)*dt;
                        d *= fracDistanceMult;    
                        if (f > thresh && dis < d)
                        {
                            w *= 0.34;
                            x *= 2.55;
                            y *= 2.55;
                            f -= w * NoiseNonLinear(x, y);
                        }
                    }
                }
            }
        }
    }
    if (f < water) f = water;
    return f;
}

int castRay( in vec3 ro, in vec3 rd, out float dis)
{
    float delt = .1;
    float maxt = 45.0;
    int ret = 0;
    dis = 1.0;
    float lh = 0.0;
    float ly = 0.0;
    for (int l = 0; l < 120; l++)
    {
        if (ret == 0 && dis < maxt)
        {
            vec3 p = ro + rd*dis;
            float h = terrainLow(p.x*scale, p.z*scale,p.y,dis);
            if (p.y < h)
            {
                ret = 0;
                dis -= delt * 1.1;
                for (int c = 0; c < 14; c++)
                {
                    if (ret == 0 && dis < maxt)
                    {
                        vec3 p = ro + rd*dis;
                        float h = terrain(p.x*scale, p.z*scale,p.y, dis);
                        if (p.y < h)
                        {
                            float f = delt*0.1;
                            dis = dis - f + f *(lh-ly)/(p.y - ly - h + lh);
                            ret = 1;
                        }else
                        {
                            lh = h;
                            ly = p.y;
                            delt+= 0.003;
                            dis += delt*0.1;
                        }
                    }
                }
            }else
            {
                lh = h;
                ly = p.y;
                delt += 0.005;
                dis += delt;
            }
        }
    };

    return ret;
}

vec3 getTerrain(vec3 scrCoord, vec3 rd, float dis)
{
    vec3 pos = (scrCoord + rd * dis)*scale;
    float p = 5.0 * pow(0.36,9.0);
    vec3 nor = vec3(0.0, terrain(pos.x, pos.z,0.0,dis*scale), 0.0);
    vec3 v2 = vec3(p, terrain(pos.x+p, pos.z,0.0,dis*scale), 0.0);
    vec3 v3 = vec3(0.0, terrain(pos.x, pos.z-p,0.0,dis*scale), -p);
    v2 = nor-v2;
    v3 = nor-v3;
    nor = cross(v2,v3);
    nor = normalize(nor);
    float diffuse = dot(light,nor);
    vec3 mat;

    if (pos.y <= water)
    {
        // Water...
        mat = vec3(.0, .00, 0.1);
    }else
    {
        //Ground texture...
        float f = NoiseNonLinear(pos.x*4.0, pos.z*4.0)*0.4+0.3;
        mat = vec3(f*.5, f*.38, f*0.3);
        if (nor.y < .2 && nor.y > 0.0)
        {
            float v = clamp((.2-nor.y)*4.2, 0.0,1.0);
            mat = mix(mat, vec3(.8,.8,.8), v);
        }
        if (pos.y < .35 && nor.y > .1)
        {
            vec3 m = vec3(.0, clamp(NoiseNonLinear(pos.x*98.0, pos.z*98.0)*.55+.0, 0.0, .5), 0.0); 
            mat = mix(mat, m, clamp((nor.y-.1)*4.0, 0.0, 1.0));
        }
        mat.xyz = mat*vec3(.9,.9,.75)*diffuse + vec3(0.1,0.1,0.1);

        float v = clamp((.52-pos.y)*.55, 0.0,1.0);
        mat = mix(mat,  vec3(.1, .05, .0), v);

    }
    return vec3(applyFog(mat, dis, scrCoord, rd, light));
}

void main(void)
{
    //Camera animation
    vec2 vPos=(-1.0+2.0*(gl_FragCoord.xy/resolution.xy)) * 0.3;
    vec3 vuv=vec3(stime*0.01,.3,0);     //view up vector
    vec3 vrp=vec3(sin(offTime*0.027)*5.0, 1, cos(offTime*0.19)*20.0); //view reference point
    vec3 prp=vec3(sin(offTime*0.027)*40.0 + vrp.x+1.0, stime*.01+5.0+vrp.y+1.0, cos(offTime*0.06)*20.0+vrp.z+1.0); //camera position
    vec3 vpn=normalize(vrp-prp);     //Camera setup
    vec3 u=normalize(cross(vuv,vpn));
    vec3 v=cross(vpn,u);
    vec3 vcv=(prp+vpn);
    vec3 scrCoord=vcv+vPos.x*u*resolution.x/resolution.y+vPos.y*v;
    vec3 rd=normalize(scrCoord-prp);

    float dis = 0.0;
    if (castRay(scrCoord, rd, dis) == 1)
    {
        gl_FragColor = vec4(getTerrain(scrCoord, rd, dis), 1.0);
    }else
    {
        gl_FragColor= vec4(getSky(rd, light), 1.0);
    }
}
