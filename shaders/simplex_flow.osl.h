/// Oiginal notice:
 
/*
 * srdnoise23, Simplex noise with rotating gradients
 * and a true analytic derivative in 2D and 3D.
 *
 * This is version 2 of srdnoise23 written in early 2008.
 * A stupid bug was corrected. Do not use earlier versions.
 *
 * Author: Stefan Gustavson, 2003-2008
 *
 * Contact: stefan.gustavson@gmail.com
 *
 * This code was GPL licensed until February 2011.
 * As the original author of this code, I hereby
 * release it into the public domain.
 * Please feel free to use it for whatever you want.
 * Credit is appreciated where appropriate, and I also
 * appreciate being told where this code finds any use,
 * but you may do as you like.
 */
  
/// Translated and modified by Ivan Mavrov, Chaos Group Ltd. 2016
/// Contact: ivan.mavrov@chaosgroup.com
 
/// 3D Simplex flow noise implementation.
/// Returned values are in the [0, 1] range.
float simplexFlowNoise3D(point position, float flow, output vector dNoise) {
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
     
    // Skewing factors for the 3D simplex grid.
    // float F3 = 1.0 / 3.0;
    // float G3 = 1.0 / 6.0;
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
     
    // Sine and cosine for the gradient rotation angle.
    float sinTheta = 0.0;
    float cosTheta = 0.0;
    sincos(M_2PI * flow, sinTheta, cosTheta);
     
    // Calculate the contribution from the four points.
    float t0 = 0.6 - dot(offset0, offset0);
    float t02 = 0.0;
    float t04 = 0.0;
    vector gradient0 = vector(0.0);
    float n0 = 0.0;
    if (t0 < 0.0) {
        t0 = 0.0;
    } else {
        t02 = t0 * t0;
        t04 = t02 * t02;
        gradient0 = getGradient(perm[i + perm[j + perm[k]]], sinTheta, cosTheta);
        n0 = t04 * dot(gradient0, offset0);
    }
 
    float t1 = 0.6 - dot(offset1, offset1);
    float t12 = 0.0;
    float t14 = 0.0;
    vector gradient1 = vector(0.0);
    float n1 = 0.0;
    if (t1 < 0.0) {
        t1 = 0.0;
    } else {
        t12 = t1 * t1;
        t14 = t12 * t12;
        gradient1 = getGradient(perm[i + int(skewedOffset1[0]) + perm[j + int(skewedOffset1[1]) + perm[k + int(skewedOffset1[2])]]], sinTheta, cosTheta);
        n1 = t14 * dot(gradient1, offset1);
    }
 
    float t2 = 0.6 - dot(offset2, offset2);
    float t22 = 0.0;
    float t24 = 0.0;
    vector gradient2 = vector(0.0);
    float n2 = 0.0;
    if(t2 < 0.0) {
        t2 = 0.0;
    } else {
        t22 = t2 * t2;
        t24 = t22 * t22;
        gradient2 = getGradient(perm[i + int(skewedOffset2[0]) + perm[j + int(skewedOffset2[1]) + perm[k + int(skewedOffset2[2])]]], sinTheta, cosTheta);
        n2 = t24 * dot(gradient2, offset2);
    }
     
    float t3 = 0.6 - dot(offset3, offset3);
    float t32 = 0.0;
    float t34 = 0.0;
    vector gradient3 = vector(0.0);
    float n3 = 0.0;
    if(t3 < 0.0) {
        t32 = 0.0;
    } else {
        t32 = t3 * t3;
        t34 = t32 * t32;
        gradient3 = getGradient(perm[i + 1 + perm[j + 1 + perm[k + 1]]], sinTheta, cosTheta);
        n3 = t34 * dot(gradient3, offset3);
    }
 
    // Accumulate the contributions from each point to get the final noise value.
    // The result is scaled to return values in the range [-1,1].
    float noiseValue = 32.0 * (n0 + n1 + n2 + n3);
 
    // Compute noise derivative.
    //  *dnoise_dx = -8.0f * t02 * t0 * x0 * dot(gx0, gy0, gz0, x0, y0, z0) + t04 * gx0;
    //  *dnoise_dy = -8.0f * t02 * t0 * y0 * dot(gx0, gy0, gz0, x0, y0, z0) + t04 * gy0;
    //  *dnoise_dz = -8.0f * t02 * t0 * z0 * dot(gx0, gy0, gz0, x0, y0, z0) + t04 * gz0;
    //  *dnoise_dx += -8.0f * t12 * t1 * x1 * dot(gx1, gy1, gz1, x1, y1, z1) + t14 * gx1;
    //  *dnoise_dy += -8.0f * t12 * t1 * y1 * dot(gx1, gy1, gz1, x1, y1, z1) + t14 * gy1;
    //  *dnoise_dz += -8.0f * t12 * t1 * z1 * dot(gx1, gy1, gz1, x1, y1, z1) + t14 * gz1;
    //  *dnoise_dx += -8.0f * t22 * t2 * x2 * dot(gx2, gy2, gz2, x2, y2, z2) + t24 * gx2;
    //  *dnoise_dy += -8.0f * t22 * t2 * y2 * dot(gx2, gy2, gz2, x2, y2, z2) + t24 * gy2;
    //  *dnoise_dz += -8.0f * t22 * t2 * z2 * dot(gx2, gy2, gz2, x2, y2, z2) + t24 * gz2;
    //  *dnoise_dx += -8.0f * t32 * t3 * x3 * dot(gx3, gy3, gz3, x3, y3, z3) + t34 * gx3;
    //  *dnoise_dy += -8.0f * t32 * t3 * y3 * dot(gx3, gy3, gz3, x3, y3, z3) + t34 * gy3;
    //  *dnoise_dz += -8.0f * t32 * t3 * z3 * dot(gx3, gy3, gz3, x3, y3, z3) + t34 * gz3;
     
    dNoise  = t02 * t0 * offset0 * dot(gradient0, offset0) + t04 * gradient0;
    dNoise += t12 * t1 * offset1 * dot(gradient1, offset1) + t14 * gradient1;
    dNoise += t22 * t2 * offset2 * dot(gradient2, offset2) + t24 * gradient2;
    dNoise += t32 * t3 * offset3 * dot(gradient3, offset3) + t34 * gradient3;  
    dNoise *= -8.0;
     
    // Scale noise derivative to match the noise scaling.
    //dNoise *= (32.0 / 2.0);
     
    // Scale to the [0, 1] range.
    return 0.5 * noiseValue + 0.5;
}
 
/// 3D Fractal Simplex flow noise implementation.
/// Returned values are in the [-1, 1] range.
float fractalSimplexFlowNoise3D(
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
        vector flownoise3DGradient = vector(0.0);
        float noiseOctave = amplitude * (simplexFlowNoise3D(position, flowValue, flownoise3DGradient) - 0.5);
        noiseValue += noiseOctave;
         
        position -= advectionAmount * noiseOctave * flownoise3DGradient;
 
        position *= lacunarity;
        flowValue *= flowRate;
        amplitude *= gain;
        advectionAmount *= advect;
    }
     
    return noiseValue;
}
 
