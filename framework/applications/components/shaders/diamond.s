#ifdef GL_ES
precision mediump float;
#endif

uniform int itime;
uniform vec2 n_Resolution;
uniform sampler2D iChannel0;

vec2 iResolution = n_Resolution;
vec2 iMouse = vec2(iResolution)*0.5;
float iGlobalTime = float(itime)/10.;

/* ------------------------------------------------------------------------- */
void intersect_plane(vec3 dir,vec3 pos,vec3 planenormal,float planeoffset,inout float near,inout float far,inout vec3 nearnormal,inout vec3 farnormal)
{
	float negheight=planeoffset-dot(pos,planenormal);
	float slope=dot(dir,planenormal);
	float t=negheight/slope;

	if(slope<0.0)
	{
		if(t>near)
		{
			near=t;
			nearnormal=planenormal;
		}
	}
	else
	{
		if(t<far)
		{
			far=t;
			farnormal=planenormal;
		}
	}
}

int find_intersection(vec3 dir,vec3 pos,out float near,out float far,out vec3 nearnormal,out vec3 farnormal)
{
	near=-10000.0;
	far=10000.0;
	nearnormal=vec3(0.0);
	farnormal=vec3(0.0);

	intersect_plane(dir,pos,normalize(vec3(0.0,1.0,0.0)),0.4,near,far,nearnormal,farnormal);
	if(far<0.0 || near>far) return 0;
	intersect_plane(dir,pos,normalize(vec3(-1.0, 1.0, 1.0)),1.0,near,far,nearnormal,farnormal);
	if(far<0.0 || near>far) return 0;
	intersect_plane(dir,pos,normalize(vec3( 1.0, 1.0, 1.0)),1.0,near,far,nearnormal,farnormal);
	if(far<0.0 || near>far) return 0;
	intersect_plane(dir,pos,normalize(vec3(-1.0,-1.0, 1.0)),1.0,near,far,nearnormal,farnormal);
	if(far<0.0 || near>far) return 0;
	intersect_plane(dir,pos,normalize(vec3( 1.0,-1.0, 1.0)),1.0,near,far,nearnormal,farnormal);
	if(far<0.0 || near>far) return 0;
	intersect_plane(dir,pos,normalize(vec3(-1.0, 1.0,-1.0)),1.0,near,far,nearnormal,farnormal);
	if(far<0.0 || near>far) return 0;
	intersect_plane(dir,pos,normalize(vec3( 1.0, 1.0,-1.0)),1.0,near,far,nearnormal,farnormal);
	if(far<0.0 || near>far) return 0;
	intersect_plane(dir,pos,normalize(vec3(-1.0,-1.0,-1.0)),1.0,near,far,nearnormal,farnormal);
	if(far<0.0 || near>far) return 0;
	intersect_plane(dir,pos,normalize(vec3( 1.0,-1.0,-1.0)),1.0,near,far,nearnormal,farnormal);
	if(far<0.0 || near>far) return 0;

	return 1;
}

vec3 environment(vec3 dir)
{
	vec3 rgb=texture2D(iChannel0, vec2(dir.x+0.5,dir.y+0.5)*1.0).xyz;
	float l=(rgb.x+rgb.y+rgb.z)/3.5;
	float l2=pow(l, 3.0)*2.5;
	return rgb/l*l2;
}

mat3 transpose(mat3 m)
{
		return mat3(m[0][0],m[1][0],m[2][0],
			    m[0][1],m[1][1],m[2][1],
			    m[0][2],m[1][2],m[2][2]);
}

void split_light(vec3 incomingdir,vec3 normal,float eta,out vec3 reflecteddir,out vec3 refracteddir,out float reflectedfraction)
{
	reflecteddir=incomingdir-2.0*dot(incomingdir,normal)*normal;

	float k=1.0-eta*eta*(1.0-dot(normal,incomingdir)*dot(normal,incomingdir));
	if(k>0.0)
	{
		refracteddir=eta*incomingdir-(eta*dot(normal,incomingdir)+sqrt(k))*normal;
		float f=pow(1.0-eta,2.0)/pow(1.0+eta,2.0);
		reflectedfraction=f+(1.0-f)*pow(1.0+dot(incomingdir,normal),5.0); // Approximate.
	}
	else
	{
		reflectedfraction=1.0;
	}
}

mat3 inv;

vec3 gather_light(vec3 dir,vec3 pos,vec3 normal,float eta)
{
	vec3 reflecteddir,refracteddir;
	float reflectedfraction;

	split_light(dir,normal,1.0/eta,reflecteddir,refracteddir,reflectedfraction);

	vec3 light=environment(inv*reflecteddir)*reflectedfraction;

	dir=refracteddir;
	float fraction=1.0-reflectedfraction;

	for(int i=0;i<32;i++)
	{
		float near,far;
		vec3 nearnormal,farnormal;
		find_intersection(dir,pos,near,far,nearnormal,farnormal);

		pos=pos+dir*far;

		split_light(dir,-farnormal,eta,reflecteddir,refracteddir,reflectedfraction);

		if(reflectedfraction!=1.0) light+=environment(inv*refracteddir)*(1.0-reflectedfraction)*fraction;

		dir=reflecteddir;
		fraction*=reflectedfraction;
	}
//	vec3 refractedlight=environment(refracteddir)*(1.0-reflectedfraction);

	return light;
}

void main()
{
	vec2 position=(1.0*gl_FragCoord.xy-iResolution.xy)/max(iResolution.x,iResolution.y)+0.5;
	vec3 pos=vec3(0.0,-1.5,-6.0);
	vec3 dir=normalize(vec3(position,1.8-sqrt(position.x*position.x+position.y*position.y)));
//	vec3 dir=normalize(vec3(position,1.0));

	float a=sin(iGlobalTime*0.3);
	float b=iGlobalTime*0.2;
	mat3 rot=mat3( cos(b),0.0,-sin(b),
		          0.0,1.0,    0.0,
		       sin(b),0.0, cos(b));
	rot*=mat3(1.0,    0.0,   0.0,
		      0.0, cos(a),sin(a),
		      0.0,-sin(a),cos(a));

	inv=transpose(rot);
	vec3 localdir=rot*dir;
	vec3 localpos=rot*pos;

	float near,far;
	vec3 localnearnormal,localfarnormal;
	int hit=find_intersection(localdir,localpos,near,far,localnearnormal,localfarnormal);
	if(hit!=0)
	{
		vec3 newlocalpos=localpos+localdir*near;
		vec3 light;
		// http://refractiveindex.info/?group=CRYSTALS&material=C
		light=gather_light(localdir,newlocalpos,localnearnormal,2.4105)*vec3(1.0,0.0,0.0); // 650 nm
		light+=gather_light(localdir,newlocalpos,localnearnormal,2.42602)*vec3(0.0,1.0,0.0); // 532 nm
		light+=gather_light(localdir,newlocalpos,localnearnormal,2.43883)*vec3(0.0,0.0,1.0); // 473 nm
		gl_FragColor=vec4(light,1.0);
	}
	else
	{
		gl_FragColor=vec4(environment(dir),1.0);
	}
}
