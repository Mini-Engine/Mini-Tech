/*
 * Copyright (c) 2016
 *  Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 */
  
/// Alligator Noise is provided by Side Effects Software Inc. and is licensed
/// under a Creative Commons Attribution-ShareAlike 4.0 International License.
///
/// Author:  "Side Effects Software Inc"
/// Source:  "http://www.sidefx.com/docs/hdk15.0/alligator_2alligator_8_c-example.html"
/// License: "http://creativecommons.org/licenses/by-sa/4.0/"
///
/// Translated and modified by Ivan Mavrov, Chaos Group Ltd. 2016
/// Contact: ivan.mavrov@chaosgroup.com
 
/// 3D Alligator noise implementation.
/// Returned values are in the [0, 1] range.
float alligatorNoise3D(point position) {
    vector cellOffsets[27] = {
        vector( 0,  0,  0),
        vector( 1,  0,  0),
        vector( 1,  1,  0),
        vector( 0,  1,  0),
        vector(-1,  1,  0),
        vector(-1,  0,  0),
        vector(-1, -1,  0),
        vector( 0, -1,  0),
        vector( 1, -1,  0),
         
        vector( 0,  0, -1),
        vector( 1,  0, -1),
        vector( 1,  1, -1),
        vector( 0,  1, -1),
        vector(-1,  1, -1),
        vector(-1,  0, -1),
        vector(-1, -1, -1),
        vector( 0, -1, -1),
        vector( 1, -1, -1),
 
        vector( 0,  0,  1),
        vector( 1,  0,  1),
        vector( 1,  1,  1),
        vector( 0,  1,  1),
        vector(-1,  1,  1),
        vector(-1,  0,  1),
        vector(-1, -1,  1),
        vector( 0, -1,  1),
        vector( 1, -1,  1)
    };
 
    point iPosition = floor(position);
 
    float firstReverseSmoothPointDistance = 0.0;
    float secondReverseSmoothPointDistance = 0.0;
     
    for (int cellIndex = 0; cellIndex < 27; ++cellIndex) {
        point cellOrigin = iPosition + cellOffsets[cellIndex];
        vector cellPointOffset = cellnoise(cellOrigin, 0.0);
        point cellPointPosition = cellOrigin + cellPointOffset;
 
        float cellPointDistance = distance(position, cellPointPosition);
         
        if (cellPointDistance < 1.0) {
            float reverseSmoothDistance = smoothstep(0.0, 1.0, 1.0 - cellPointDistance);
             
            float distanceMultiplier = float(cellnoise(cellOrigin, 1.0));
            reverseSmoothDistance *= distanceMultiplier;
             
            if (firstReverseSmoothPointDistance < reverseSmoothDistance) {
                secondReverseSmoothPointDistance = firstReverseSmoothPointDistance;
                firstReverseSmoothPointDistance = reverseSmoothDistance;
            } else {
                if (secondReverseSmoothPointDistance < reverseSmoothDistance) {
                    secondReverseSmoothPointDistance = reverseSmoothDistance;
                }
            }
        }
    }
 
    return firstReverseSmoothPointDistance - secondReverseSmoothPointDistance;
}
 
/// 3D Fractal Alligator noise implementation.
/// Returned values are in the [-1, 1] range.
float fractalAlligatorNoise3D(
    point position,
    float lacunarity, // Houdini 2.0
    float gain,       // Houdini rough
    int octaveCount   // Houdini turbulence - 1
) {
    float noiseValue = 0.0;
     
    float amplitude = 1.0;
 
    for (int octave = 0; octave < octaveCount; ++octave) {
        noiseValue += amplitude * (alligatorNoise3D(position) - 0.5);      
        position *= lacunarity;
        amplitude *= gain;
    }
     
    return noiseValue;
}
 
