int fastFloor(float x) {
    return (x < 0) ? int(x) - 1 : int(x);
}
 
float fade(float t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}
 
float lerp(float t, float a, float b) {
    return a + t * (b - a);
}
 
/// 3D Perlin flow noise implementation.
/// Returned values are in the [0, 1] range.
float perlinFlowNoise3D(point position, float flow) {
    int perm[512] = {
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
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };
     
    // Gradient component that leads to a vector of length sqrt(2).
    // float a = sqrt(2)/sqrt(3);
    float a = 0.81649658;
     
    vector gradientUBase[16] = {
        vector( 1.0,  0.0,  1.0), vector( 0.0,  1.0,  1.0),
        vector(-1.0,  0.0,  1.0), vector( 0.0, -1.0,  1.0),
        vector( 1.0,  0.0, -1.0), vector( 0.0,  1.0, -1.0),
        vector(-1.0,  0.0, -1.0), vector( 0.0, -1.0, -1.0),
        vector( a,  a,  a), vector(-a,  a, -a),
        vector(-a, -a,  a), vector( a, -a, -a),
        vector(-a,  a,  a), vector( a, -a,  a),
        vector( a, -a, -a), vector(-a,  a, -a)
    };
 
    vector gradientVBase[16] = {
        vector(-a,  a,  a), vector(-a, -a,  a),
        vector( a, -a,  a), vector( a,  a,  a),
        vector(-a, -a, -a), vector( a, -a, -a),
        vector( a,  a, -a), vector(-a,  a, -a),
        vector( 1.0, -1.0,  0.0), vector( 1.0,  1.0,  0.0),
        vector(-1.0,  1.0,  0.0), vector(-1.0, -1.0,  0.0),
        vector( 1.0,  0.0,  1.0), vector(-1.0,  0.0,  1.0),
        vector( 0.0,  1.0, -1.0), vector( 0.0, -1.0, -1.0)
    };
     
    // Helper function to compute the rotated gradient.
    vector getGradient(int index, float sinTheta, float cosTheta) {
        int safeIndex = index % 16;
        vector gradientU = gradientUBase[safeIndex];
        vector gradientV = gradientVBase[safeIndex];
        return cosTheta * gradientU + sinTheta * gradientV;
    }
     
    float gradientDot(int index, float sinTheta, float cosTheta, float x, float y, float z) {
        vector gradient = getGradient(index, sinTheta, cosTheta);
        vector value = vector(x, y, z);
        return dot(gradient, value);
    }
 
    float x = position[0];
    float y = position[1];
    float z = position[2];
     
    int ix = fastFloor(x);
    int iy = fastFloor(y);
    int iz = fastFloor(z);
     
    float fx = x - float(ix);
    float fy = y - float(iy);
    float fz = z - float(iz);
     
    ix = ix & 255;
    iy = iy & 255;
    iz = iz & 255;
     
    float i = fade(fx);
    float j = fade(fy);
    float k = fade(fz);
     
    int A = perm[ix    ] + iy, AA = perm[A] + iz, AB = perm[A + 1] + iz;
    int B = perm[ix + 1] + iy, BA = perm[B] + iz, BB = perm[B + 1] + iz;
     
    // Sine and cosine for the gradient rotation angle
    float sinTheta = 0.0;
    float cosTheta = 0.0;
    sincos(M_2PI * flow, sinTheta, cosTheta);
 
    float noiseValue =
        lerp(k, lerp(j, lerp(i, gradientDot(perm[AA    ], sinTheta, cosTheta, fx      , fy      , fz      ),
                                gradientDot(perm[BA    ], sinTheta, cosTheta, fx - 1.0, fy      , fz      )),
                        lerp(i, gradientDot(perm[AB    ], sinTheta, cosTheta, fx      , fy - 1.0, fz      ),
                                gradientDot(perm[BB    ], sinTheta, cosTheta, fx - 1.0, fy - 1.0, fz      ))),
                lerp(j, lerp(i, gradientDot(perm[AA + 1], sinTheta, cosTheta, fx      , fy      , fz - 1.0),
                                gradientDot(perm[BA + 1], sinTheta, cosTheta, fx - 1.0, fy      , fz - 1.0)),
                        lerp(i, gradientDot(perm[AB + 1], sinTheta, cosTheta, fx      , fy - 1.0, fz - 1.0),
                                gradientDot(perm[BB + 1], sinTheta, cosTheta, fx - 1.0, fy - 1.0, fz - 1.0))));
     
    // Scale to the [0, 1] range.
    return 0.5 * noiseValue + 0.5;
}
 
vector perlinFlowNoise3DGradient(vector position, float flow, float delta) {
    vector result = vector(
        perlinFlowNoise3D(position + vector(delta, 0.0, 0.0), flow) -
        perlinFlowNoise3D(position - vector(delta, 0.0, 0.0), flow),
        perlinFlowNoise3D(position + vector(0.0, delta, 0.0), flow) -
        perlinFlowNoise3D(position - vector(0.0, delta, 0.0), flow),
        perlinFlowNoise3D(position + vector(0.0, 0.0, delta), flow) -
        perlinFlowNoise3D(position - vector(0.0, 0.0, delta), flow)
    );
 
    result /= (2.0 * delta);
 
    return result;
}
 
/// 3D Fractal Perlin flow noise implementation.
/// Returned values are in the [-1, 1] range.
float fractalPerlinFlowNoise3D(
    point position,
    float flow,
    float lacunarity,
    float flowRate,
    float gain,
    float advect,
    int octaveCount
) {
    float noiseValue = 0.0;
 
    float flowValue = flow;
     
    float amplitude = 1.0;
    float advectionAmount = advect;
 
    for (int octave = 0; octave < octaveCount; ++octave) {
        float noiseOctave = amplitude * (perlinFlowNoise3D(position, flowValue) - 0.5);
        noiseValue += noiseOctave;
 
        if (advectionAmount != 0.0) {
            position -= advectionAmount * noiseOctave * perlinFlowNoise3DGradient(position, flow, 0.01);
        }
 
        position *= lacunarity;
        flowValue *= flowRate;
        amplitude *= gain;
        advectionAmount *= advect;
    }
     
    return noiseValue;
}
 
