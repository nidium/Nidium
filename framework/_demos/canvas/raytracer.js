var w = 1024,
	H = 90,

	// Ambient light. Hardcoding it in the lighting code feels like cheating so I won't
	A = 1,

	/*
	 * Spheres : radius, [cx,  cy,  cz], R,  G,  B, specular exponent, reflectiveness 
	 * R, G, B in [0, 9], reflectiveness in [0..9]
	 */
	S = [
		H,		[ 0.0, 	-H,		0.0],	9, 9, 0,	w,		2,	// Yellow sphere.
		0.8, 	[ 0.0, 	0.0,	3.0], 	0, 0, 0, 	H,		0,	// black sphere

		1.4,	[ 0.0,  1.5,	3.0],	9, 3, 3, 	200,	2,	// Big sphere

		1, 		[-2.0,  1.4,	3.0],	0, 4, 0, 	9,		0,	// Left Green sphere
		1, 		[ 2.0,  1.4,	3.0], 	0, 0, 9,	H,		3	// Right Blue sphere
	],

	/*
	 *  Point lights : intensity, [x,  y,  z]
	 *  Intensities should add to 10, including ambient
	 */
	T = [
		6, [-2, 5, -8],
		2, [0, 7.5, 3]
	];

// -----------------------------------------------------------------------------

// Shorten some names
var a = 0,
	G = Math,
	Q = G.sqrt,
	v = canvas,
	O = canvas,
	V = O.getImageData(0, 0, w, w),
	P = V.data;

v.width = 300;
v.height = 300;

// Dot product
function d(Y, Z) {
	return Y[0]*Z[0] + Y[1]*Z[1] + Y[2]*Z[2];
}


/*
 * Find nearest intersection of the ray from B in direction D with any sphere
 * "Interesting" parameter values must be in the range [L, U]
 * Returns the index in S of the center of the hit sphere, or 0 if none
 * The parameter value for the hit is in the global variable W
 */

function N(B, D, L, U) {
	W = H;	// Min distance found
	
	// For each sphere
	for (s = n = 0; r = S[n++];)	// Get the radius and test for end of array at the same time; S[n] == undefined ends the loop
	{
		// Compute quadratic equation coefficients K1, K2, K3
		F = 2*(d( C = S[n] , D) - d(B, D));	// -K2, also store the center in C
		J = 2*d(D, D);	// 2*K1
		
		// Compute sqrt(Discriminant) = sqrt(K2*K2 - 4*K1*K3), go ahead if there are solutions
		if ( M = Q( F*F - 2*J*(d(B, B) + d(C, C) - r*r - 2*d(B, C)) ) )
		{
			// Compute the two solutions
			for (e = 2; e--; M = -M)	// TODO : I have a feeling this loop can be minimized further, but I can't figure it out
			{
				t = (F + M)/J;
				if (L < t && t < U && t < W) 
					s=n, W=t;
			}
		}
		
		n += 6;
	}

	// Return index of closest sphere in range; W is global
	return s;
}


// Helper : f(Y, Z, k)  =  Y - Z*k.
function f(Y, Z, k){
	return [Y[0] - Z[0]*k, Y[1] - Z[1]*k, Y[2] - Z[2]*k];
}

/*
 * Trace the ray from B with direction D considering hits in [L, U]
 * If p > 0, trace recursive reflection rays
 * Returns the value of the current color channel as "seen" through the ray
 * q is a fake parameter used to avoid using "var" below
 */

function R (B, D, L, U, p, q) {
	// Find nearest hit; if no hit, return black
	if (!(g = N(B, D, L, U)))
		return 0;
	
	// Compute "normal" at intersection : o = X - S[g]
	o = f( 
		X = f(B, D, -W), 	// Compute intersection : X = B + D*W = B - D*(-W)
		S[g], 1);

	// Start with ambient light only
	i = A;
	
	// For each light
	for (l = 0; u = T[l++]; ) {
		// Get intensity and check for end of array
	
		// Compute vector from intersection to light (I = T[l++] - X) and 
		// j = <N,L> (reused below)
		j = d(o, I = f(T[l++], X, 1) );

		// V = <N,L> / (|L|*|N|) = cos(alpha)
		// Also, |N|*|L| = sqrt(<N,N>)*sqrt(<L,L>) = sqrt(<N,N>*<L,L>)
		v = j / Q(d(I, I)*d(o, o));
		
		// Add diffuse contribution only if it's facing the point 
		// (cos(alpha) > 0) and no other sphere casts a shadow
		// [L, U]  =  [epsilon,  1] - epsilon avoids self-shadow, 1 doesn't look 
		// farther than the light itself
		if (v > 0 && !N(X, I, .01, 1)) {
			i += v*u;

			// Specular highlights
			//
			// specular = (<R,V>   / (|R|*|V|))   ^ exponent
			//			= (<-R,-V> / (|-R|*|-V|)) ^ exponent
			//			= (<-R,D>  / (|-R|*|D|))  ^ exponent
			//
			// R = 2*N*<N,I> - I
			// M = -R = -2*o*<o,I> + I = I + o*(-2*<o,I>)
			//
			v = G.pow( d( M = f(I, o, 2*j), D)/ Q(d(M, M)*d(D, D)), S[g+4]);

			// Add specular contribution only if visible
			v > 0 && 
				(i += v*u);
		}
	}
	

	// Compute the color channel multiplied by the light intensity. 2.8 "fixes"
	// the color range in [0, 9] instead of [0, 255] and the intensity in [0, 10]
	// instead of [0, 1],  because 2.8 ~ (255/9)/10
	// 
	// S[g] = sphere center, so S[g+c] = color channel c (c = [1..3] because a=c++ below)
	q = S[g+c]*i*2.8;
	
	// If the recursion limit hasn't been hit yet, trace reflection rays
	// o = normal
	// D = -view
	// M = 2*N*<N,V> - V = 2*o*<o,-D> + D = D - o*(2*<o,D>)
	k = S[g+5]/9;

	return p-- ? R(X, f(D, o, 2*d(o, D)), .01, H, p)*k + q*(1-k) : q;
}

// For each y; also compute h=w/2 without paying an extra ";"
/*
for (y = h = w/2; y-- > -h;) {

	// For each x
	for (x = -h; x++ < h;) {
		// For each color channel
		for (c = 0; ++c < 4;) {

			// Camera is at (0, 1, 0)
			//
			// Ray direction is (x*vw/cw, y*vh/ch, 1) where vw = viewport width, 
			// cw = canvas width (vh and ch are the same for height). vw is fixed
			// at 1 so (x/w, y/w, 1)
			//
			// [L, U] = [1, H], 1 starts at the projection plane, H is +infinity
			//
			// 2 is a good recursion depth to appreciate the reflections without
			// slowing things down too much
			//
			P[a++] = R([0, 1, -2], [x/w, y/w, 1], 1, H, 2);
		}
			
		P[a++] = 255; // Opaque alpha
	}
}
*/


var y = h = w/2;

canvas.fontSize = 76;

canvas.requestAnimationFrame(function(){
	canvas.fillStyle = "#111111";
	canvas.fillRect(0, 0, w, w);

	if (y-- > -h) {

		// For each x
		for (x = -h; x++ < h;) {
			// For each color channel
			for (c = 0; ++c < 4;) {

				// Camera is at (0, 1, 0)
				//
				// Ray direction is (x*vw/cw, y*vh/ch, 1) where vw = viewport width, 
				// cw = canvas width (vh and ch are the same for height). vw is fixed
				// at 1 so (x/w, y/w, 1)
				//
				// [L, U] = [1, H], 1 starts at the projection plane, H is +infinity
				//
				// 2 is a good recursion depth to appreciate the reflections without
				// slowing things down too much
				//
				P[a++] = R([0, 1.5, -4.5], [x/w, y/w, 1.0], 1, H, 3);
			}
				
			P[a++] = 255; // Opaque alpha
		}
	}

	O.putImageData(V, 0, 0);

	canvas.fillStyle = "rgba(255, 255, 255, 0.15)";
	canvas.fillText("Old School Raytracer", 10, 80);

});


