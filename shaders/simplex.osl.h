/// 3D Simplex noise implementation.
/// Returned values are in the [0, 1] range.
float simplexNoise3D(point position) {
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
     
    vector gradient[16] = {
        vector( 1.0,  1.0,  0.0),
        vector(-1.0,  1.0,  0.0),
        vector( 1.0, -1.0,  0.0),
        vector(-1.0, -1.0,  0.0),
        vector( 1.0,  0.0,  1.0),
        vector(-1.0,  0.0,  1.0),
        vector( 1.0,  0.0, -1.0),
        vector(-1.0,  0.0, -1.0),
        vector( 0.0,  1.0,  1.0),
        vector( 0.0, -1.0,  1.0),
        vector( 0.0,  1.0, -1.0),
        vector( 0.0, -1.0, -1.0),
        vector( 1.0,  1.0,  0.0),
        vector( 0.0, -1.0,  1.0),
        vector(-1.0,  1.0,  0.0),
        vector( 0.0, -1.0, -1.0)
    };
     
    float evaluatePointContribution(int i, int j, int k, vector offset) {
        float t = 0.6 - dot(offset, offset);
        if (t < 0.0) {
            return 0.0;
        }
         
        float t2 = t * t;
        float t4 = t2 * t2;
         
        int gradientIndex = perm[i + perm[j + perm[k]]];
        int safeGradientIndex = gradientIndex % 16;
         
        return t4 * dot(gradient[safeGradientIndex], offset);
    }
     
    // Skewing factors for the 3D simplex grid.
    float F3 = 0.333333333;
    float G3 = 0.166666667;
    float G3a = 2.0 * G3;
    float G3b = 3.0 * G3 - 1.0;
 
    // Skew the input space to determine the simplex cell we are in.
    float skew = (position[0] + position[1] + position[2]) * F3;
    point skewedPosition = position + skew;
    point skewedCellOrigin = floor(skewedPosition);
 
    // Unskew the cell origin
    float unskew = (skewedCellOrigin[0] + skewedCellOrigin[1] + skewedCellOrigin[2]) * G3;
    point cellOrigin = skewedCellOrigin - unskew;
     
    // The offset from the cell's origin a.k.a point 0
    vector offset0 = position - cellOrigin;
 
    // The second point offset from the cell origin in skewed space.
    vector skewedOffset1;
    // The third point offset from the cell origin in skewed space.
    vector skewedOffset2;
 
    if (offset0[0] >= offset0[1]) {
        if (offset0[1] >= offset0[2]) {
            // X Y Z order
            skewedOffset1 = vector(1, 0, 0);
            skewedOffset2 = vector(1, 1, 0);
        } else if (offset0[0] >= offset0[2]) {
            // X Z Y order
            skewedOffset1 = vector(1, 0, 0);
            skewedOffset2 = vector(1, 0, 1);
        } else {
            // Z X Y order
            skewedOffset1 = vector(0, 0, 1);
            skewedOffset2 = vector(1, 0, 1);
        }
    } else {
        if (offset0[1] < offset0[2]) {
            // Z Y X order
            skewedOffset1 = vector(0, 0, 1);
            skewedOffset2 = vector(0, 1, 1);
        } else if (offset0[0] < offset0[2]) {
            // Y Z X order
            skewedOffset1 = vector(0, 1, 0);
            skewedOffset2 = vector(0, 1, 1);
        } else {
            // Y X Z order
            skewedOffset1 = vector(0, 1, 0);
            skewedOffset2 = vector(1, 1, 0);
        }
    }
 
    // A step of (1, 0, 0) in skewed space means a step of (1 - G3, -G3, -G3) in regular space.
    // A step of (0, 1, 0) in skewed space means a step of (-G3, 1 - G3, -G3) in regular space.
    // A step of (0, 0, 1) in skewed space means a step of (-G3, -G3, 1 - G3) in regular space.
 
    // The offset from point 1 in regular space.
    vector offset1 = offset0 - skewedOffset1 + G3;
     
    // The offset from point 2 in regular space.
    vector offset2 = offset0 - skewedOffset2 + G3a;
 
    // The offset from point 3 in regular space.
    vector offset3 = offset0 + G3b;
 
    // Wrap the integer indices at 256, to avoid indexing perm[] out of bounds.
    int i = int(skewedCellOrigin[0]) & 255;
    int j = int(skewedCellOrigin[1]) & 255;
    int k = int(skewedCellOrigin[2]) & 255;
 
    // Accumulate the contributions from the four points.
    float noiseValue = 0.0;
    noiseValue += evaluatePointContribution(i, j, k, offset0);
    noiseValue += evaluatePointContribution(i + int(skewedOffset1[0]), j + int(skewedOffset1[1]), k + int(skewedOffset1[2]), offset1);
    noiseValue += evaluatePointContribution(i + int(skewedOffset2[0]), j + int(skewedOffset2[1]), k + int(skewedOffset2[2]), offset2);
    noiseValue += evaluatePointContribution(i + 1, j + 1, k + 1, offset3);
 
    // Scale to the [-1,1] range.
    noiseValue *= 32.0;
     
    // Scale to the [0, 1] range.
    return 0.5 * noiseValue + 0.5;
}
 
/// 3D Fractal Simplex noise implementation.
/// Returned values are in the [-1, 1] range.
float fractalSimplexNoise3D(
    point position,
    float lacunarity,
    float gain,
    int octaveCount
) {
    float noiseValue = 0.0;
     
    float amplitude = 1.0;
     
    for (int octave = 0; octave < octaveCount; ++octave) {
        noiseValue += amplitude * (simplexNoise3D(position) - 0.5);
        position *= lacunarity;
        amplitude *= gain;
    }
     
    return noiseValue;
}