var RT_P_MASK = 255;
var RT_P_SIZE = 256;
var RT_P = [151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  ];

var RT_G_MASK = 15;
var RT_G_SIZE = 16;
var RT_G_VECSIZE = 4;
var RT_G = [
	 +1.0, +1.0, +0.0, 0.0 ,
	 -1.0, +1.0, +0.0, 0.0 ,
	 +1.0, -1.0, +0.0, 0.0 ,
	 -1.0, -1.0, +0.0, 0.0 ,
	 +1.0, +0.0, +1.0, 0.0 ,
	 -1.0, +0.0, +1.0, 0.0 ,
	 +1.0, +0.0, -1.0, 0.0 ,
	 -1.0, +0.0, -1.0, 0.0 ,
	 +0.0, +1.0, +1.0, 0.0 ,
	 +0.0, -1.0, +1.0, 0.0 ,
	 +0.0, +1.0, -1.0, 0.0 ,
	 +0.0, -1.0, -1.0, 0.0 ,
	 +1.0, +1.0, +0.0, 0.0 ,
	 -1.0, +1.0, +0.0, 0.0 ,
	 +0.0, -1.0, +1.0, 0.0 ,
	 +0.0, -1.0, -1.0, 0.0
];

function RT_add4(a, b)
{
	return [ a[0]+b[0], a[1]+b[1], a[2]+b[2], a[3]+b[3] ];
}

function RT_sub4(a, b)
{
	return [ a[0]-b[0], a[1]-b[1], a[2]-b[2], a[3]-b[3] ];
}

function RT_mul4(v, s)
{
	return [ s*v[0], s*v[1], s*v[2], s*v[3] ];
}

function RT_div4(v, s)
{
	return [ v[0]/s, v[1]/s, v[2]/s, v[3]/s ];
}

function RT_dot4(a, b)
{
	return (a[0]*b[0]) + (a[1]*b[1]) + (a[2]*b[2]) + (a[3]*b[3]);
}

// Note: Three vertices are xyz
function RT_vload4(index, ar, NUM_VERTEX_COMPONENTS)
{
	var i = index * NUM_VERTEX_COMPONENTS;
	// FIXME: '+ 0' ensures the elements of the result array have the same precision.
	// It's a workaround due to a bug in jslib.
	return [ ar[i] + 0, ar[i+1] + 0, ar[i+2] + 0, 1.0 ];
}

function RT_clamp(x, minval, maxval)
{
	var mx = (x > minval) ? x : minval;
	return (mx < maxval) ? mx : maxval;
}

function RT_mix1d(a, b, t)
{
	var ba = b - a;
	var tba = t * ba;
	var atba = a + tba;
	return atba;
}

function RT_mix2d(a, b, t)
{
	var ba   = [0, 0];
	var tba  = [0, 0];
	var atba = [0, 0];
	ba[0] = b[0] - a[0];
	ba[1] = b[1] - a[1];
	tba[0] = t * ba[0];
	tba[1] = t * ba[1];
	atba[0] = a[0] + tba[0];
	atba[1] = a[1] + tba[1];
	return atba;
}

function RT_mix3d(a, b, t)
{
	var ba   = [0, 0, 0, 0];
	var tba  = [0, 0, 0, 0];
	var atba = [0, 0, 0, 0];
	ba[0] = b[0] - a[0];
	ba[1] = b[1] - a[1];
	ba[2] = b[2] - a[2];
	ba[3] = b[3] - a[3];
	tba[0] = t * ba[0];
	tba[1] = t * ba[1];
	tba[2] = t * ba[2];
	tba[3] = t * ba[3];
	atba[0] = a[0] + tba[0];
	atba[1] = a[1] + tba[1];
	atba[2] = a[2] + tba[2];
	atba[3] = a[3] + tba[3];
	return atba;
}

function RT_smooth(t)
{
	return t*t*t*(t*(t*6.0-15.0)+10.0);
}

function RT_lattice3d(i, P)
{
	return P[i[0] + P[i[1] + P[i[2]]]];
}

function RT_gradient3d(i, v, P, G, G_MASK, G_VECSIZE)
{
	var index = (RT_lattice3d(i, P) & G_MASK) * G_VECSIZE;
	var g = [ G[index + 0], G[index + 1], G[index + 2], 1.0 ];
	return RT_dot4(v, g);
}

function RT_normalized(v)
{
	var d = Math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	d = d > 0.0 ? d : 1.0;
	var result = [ v[0]/d, v[1]/d, v[2]/d, 1.0 ];
	return result;
}

function RT_gradient_noise3d(position, P, G, P_MASK, G_MASK, G_VECSIZE)
{

	var p = position;
	var pf = [ Math.floor(p[0]), Math.floor(p[1]), Math.floor(p[2]), Math.floor(p[3]) ];
	var ip = [ pf[0], pf[1], pf[2], 0 ];
	var fp = RT_sub4(p, pf);
	
	ip[0] = ip[0] & P_MASK;
	ip[1] = ip[1] & P_MASK;
	ip[2] = ip[2] & P_MASK;
	ip[3] = ip[3] & P_MASK;

	var I000 = [0, 0, 0, 0];
	var I001 = [0, 0, 1, 0];
	var I010 = [0, 1, 0, 0];
	var I011 = [0, 1, 1, 0];
	var I100 = [1, 0, 0, 0];
	var I101 = [1, 0, 1, 0];
	var I110 = [1, 1, 0, 0];
	var I111 = [1, 1, 1, 0];

	var F000 = [0.0, 0.0, 0.0, 0.0];
	var F001 = [0.0, 0.0, 1.0, 0.0];
	var F010 = [0.0, 1.0, 0.0, 0.0];
	var F011 = [0.0, 1.0, 1.0, 0.0];
	var F100 = [1.0, 0.0, 0.0, 0.0];
	var F101 = [1.0, 0.0, 1.0, 0.0];
	var F110 = [1.0, 1.0, 0.0, 0.0];
	var F111 = [1.0, 1.0, 1.0, 0.0];

	var n000 = RT_gradient3d(RT_add4(ip, I000), RT_sub4(fp, F000), P, G, G_MASK, G_VECSIZE);
	var n001 = RT_gradient3d(RT_add4(ip, I001), RT_sub4(fp, F001), P, G, G_MASK, G_VECSIZE);

	var n010 = RT_gradient3d(RT_add4(ip, I010), RT_sub4(fp, F010), P, G, G_MASK, G_VECSIZE);
	var n011 = RT_gradient3d(RT_add4(ip, I011), RT_sub4(fp, F011), P, G, G_MASK, G_VECSIZE);

	var n100 = RT_gradient3d(RT_add4(ip, I100), RT_sub4(fp, F100), P, G, G_MASK, G_VECSIZE);
	var n101 = RT_gradient3d(RT_add4(ip, I101), RT_sub4(fp, F101), P, G, G_MASK, G_VECSIZE);

	var n110 = RT_gradient3d(RT_add4(ip, I110), RT_sub4(fp, F110), P, G, G_MASK, G_VECSIZE);
	var n111 = RT_gradient3d(RT_add4(ip, I111), RT_sub4(fp, F111), P, G, G_MASK, G_VECSIZE);

	var n40 = [n000, n001, n010, n011];
	var n41 = [n100, n101, n110, n111];

	var n4 = RT_mix3d(n40, n41, RT_smooth(fp[0]));
	var n2 = RT_mix2d([n4[0], n4[1]], [n4[2], n4[3]], RT_smooth(fp[1]));
	var n = 0.5 - 0.5 * RT_mix1d(n2[0], n2[1], RT_smooth(fp[2]));
	return n;
}

function RT_ridgedmultifractal3d(
	position,
	frequency,
	lacunarity,
	increment,
	octaves,
	P,
	G,
	P_MASK,
	G_MASK,
	G_VECSIZE)
{
	var i = 0;
	var fi = 0.0;
	var remainder = 0.0;
	var sample = 0.0;
	var value = 0.0;
	var iterations = Math.floor(octaves);

	var threshold = 0.5;
	var offset = 1.0;
	var weight = 1.0;

	var signal = Math.abs( (1.0 - 2.0 * RT_gradient_noise3d(RT_mul4(position, frequency), P, G, P_MASK, G_MASK, G_VECSIZE)) );
	signal = offset - signal;
	signal *= signal;
	value = signal;

	for ( i = 0; i < iterations; i++ )
	{
		frequency *= lacunarity;
		weight = RT_clamp( signal * threshold, 0.0, 1.0 );
		signal = Math.abs( (1.0 - 2.0 * RT_gradient_noise3d(RT_mul4(position, frequency), P, G, P_MASK, G_MASK, G_VECSIZE)) );
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		value += signal * Math.pow( lacunarity, -fi * increment );
	}
	return value;
}

function RT_displace(
	index,
	vertices,
	frequency,
	amplitude,
	phase,
	lacunarity,
	increment,
	octaves,
	roughness,
	P,
	G,
	P_MASK,
	G_MASK,
	G_VECSIZE,
	NUM_VERTEX_COMPONENTS)
{
	var position = RT_vload4(index, vertices, NUM_VERTEX_COMPONENTS);
	var normal = position;

	roughness /= amplitude;
	var sample = RT_add4(position, [phase + 100.0, phase + 100.0, phase + 100.0, 0.0]);

	var dx = [roughness, 0.0, 0.0, 1.0];
	var dy = [0.0, roughness, 0.0, 1.0];
	var dz = [0.0, 0.0, roughness, 1.0];

	var f0 = RT_ridgedmultifractal3d(sample, frequency, lacunarity, increment, octaves, P, G, P_MASK, G_MASK, G_VECSIZE);
	var f1 = RT_ridgedmultifractal3d(RT_add4(sample, dx), frequency, lacunarity, increment, octaves, P, G, P_MASK, G_MASK, G_VECSIZE);
	var f2 = RT_ridgedmultifractal3d(RT_add4(sample, dy), frequency, lacunarity, increment, octaves, P, G, P_MASK, G_MASK, G_VECSIZE);
	var f3 = RT_ridgedmultifractal3d(RT_add4(sample, dz), frequency, lacunarity, increment, octaves, P, G, P_MASK, G_MASK, G_VECSIZE);
	
	var displacement = (f0 + f1 + f2 + f3) / 4.0;

	var vertex = RT_add4(position, RT_mul4(normal, amplitude * displacement));
	vertex[3] = 1.0;

	normal[0] -= (f1 - f0);
	normal[1] -= (f2 - f0);
	normal[2] -= (f3 - f0);
	normal = RT_normalized(RT_div4(normal, roughness));

	return {pos : [vertex[0], vertex[1], vertex[2]], nor : [normal[0], normal[1], normal[2]]};
}

function SimulateRT() {
	var nVertices	= userData.nVertices;
	var initPos		= userData.initPos;
    
	var curPosAndNor = new ParallelArray(nVertices, RT_displace, initPos, userData.frequency, userData.amplitude, userData.phase, userData.lacunarity, userData.increment, userData.octaves, userData.roughness, RT_P, RT_G, RT_P_MASK, RT_G_MASK, RT_G_VECSIZE, NUM_VERTEX_COMPONENTS).unzip(); // unzip() is undocumented
	if(curPosAndNor.pos.data instanceof Float64Array) { // ParallelArray is hacked here
		userData.curPos = new Float32Array(curPosAndNor.pos.data);
		userData.curNor = new Float32Array(curPosAndNor.nor.data);
	}
	else {
		userData.curPos = curPosAndNor.pos.data;
		userData.curNor = curPosAndNor.nor.data;
	}
		
	userData.phase += PHASE_DELTA;
}