#define NOMINMAX

#include "Mini/YutaTeapot.h"

#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/MeshPrimitive.h"

#include <array>
#include <cmath>
#include <unordered_map>

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGaffer;

namespace
{
	// Martin Newell's teapot, as 32 bicubic Bezier patches of 16 control points each.
	// Indices are 1 based into g_vertices, as in the original data set. The data is Z up.

	enum Part
	{
		Rim, Body, Handle, Spout, Lid, Bottom
	};

	struct Patch
	{
		Part part;
		int indices[16];
	};

	const Patch g_patches[] = {
		// Rim
		{ Rim, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } },
		{ Rim, { 4, 17, 18, 19, 8, 20, 21, 22, 12, 23, 24, 25, 16, 26, 27, 28 } },
		{ Rim, { 19, 29, 30, 31, 22, 32, 33, 34, 25, 35, 36, 37, 28, 38, 39, 40 } },
		{ Rim, { 31, 41, 42, 1, 34, 43, 44, 5, 37, 45, 46, 9, 40, 47, 48, 13 } },
		// Body
		{ Body, { 13, 14, 15, 16, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60 } },
		{ Body, { 16, 26, 27, 28, 52, 61, 62, 63, 56, 64, 65, 66, 60, 67, 68, 69 } },
		{ Body, { 28, 38, 39, 40, 63, 70, 71, 72, 66, 73, 74, 75, 69, 76, 77, 78 } },
		{ Body, { 40, 47, 48, 13, 72, 79, 80, 49, 75, 81, 82, 53, 78, 83, 84, 57 } },
		{ Body, { 57, 58, 59, 60, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96 } },
		{ Body, { 60, 67, 68, 69, 88, 97, 98, 99, 92, 100, 101, 102, 96, 103, 104, 105 } },
		{ Body, { 69, 76, 77, 78, 99, 106, 107, 108, 102, 109, 110, 111, 105, 112, 113, 114 } },
		{ Body, { 78, 83, 84, 57, 108, 115, 116, 85, 111, 117, 118, 89, 114, 119, 120, 93 } },
		// Handle
		{ Handle, { 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136 } },
		{ Handle, { 124, 137, 138, 121, 128, 139, 140, 125, 132, 141, 142, 129, 136, 143, 144, 133 } },
		{ Handle, { 133, 134, 135, 136, 145, 146, 147, 148, 149, 150, 151, 152, 69, 153, 154, 155 } },
		{ Handle, { 136, 143, 144, 133, 148, 156, 157, 145, 152, 158, 159, 149, 155, 160, 161, 69 } },
		// Spout
		{ Spout, { 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177 } },
		{ Spout, { 165, 178, 179, 162, 169, 180, 181, 166, 173, 182, 183, 170, 177, 184, 185, 174 } },
		{ Spout, { 174, 175, 176, 177, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197 } },
		{ Spout, { 177, 184, 185, 174, 189, 198, 199, 186, 193, 200, 201, 190, 197, 202, 203, 194 } },
		// Lid
		{ Lid, { 204, 204, 204, 204, 207, 208, 209, 210, 211, 211, 211, 211, 212, 213, 214, 215 } },
		{ Lid, { 204, 204, 204, 204, 210, 217, 218, 219, 211, 211, 211, 211, 215, 220, 221, 222 } },
		{ Lid, { 204, 204, 204, 204, 219, 224, 225, 226, 211, 211, 211, 211, 222, 227, 228, 229 } },
		{ Lid, { 204, 204, 204, 204, 226, 230, 231, 207, 211, 211, 211, 211, 229, 232, 233, 212 } },
		{ Lid, { 212, 213, 214, 215, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245 } },
		{ Lid, { 215, 220, 221, 222, 237, 246, 247, 248, 241, 249, 250, 251, 245, 252, 253, 254 } },
		{ Lid, { 222, 227, 228, 229, 248, 255, 256, 257, 251, 258, 259, 260, 254, 261, 262, 263 } },
		{ Lid, { 229, 232, 233, 212, 257, 264, 265, 234, 260, 266, 267, 238, 263, 268, 269, 242 } },
		// Bottom
		{ Bottom, { 270, 270, 270, 270, 279, 280, 281, 282, 275, 276, 277, 278, 271, 272, 273, 274 } },
		{ Bottom, { 270, 270, 270, 270, 282, 289, 290, 291, 278, 286, 287, 288, 274, 283, 284, 285 } },
		{ Bottom, { 270, 270, 270, 270, 291, 298, 299, 300, 288, 295, 296, 297, 285, 292, 293, 294 } },
		{ Bottom, { 270, 270, 270, 270, 300, 305, 306, 279, 297, 303, 304, 275, 294, 301, 302, 271 } },
	};

	const float g_vertices[][3] = {
		{ 1.4f, 0.0f, 2.4f }, { 1.4f, -0.784f, 2.4f }, { 0.784f, -1.4f, 2.4f }, { 0.0f, -1.4f, 2.4f },
		{ 1.3375f, 0.0f, 2.53125f }, { 1.3375f, -0.749f, 2.53125f }, { 0.749f, -1.3375f, 2.53125f }, { 0.0f, -1.3375f, 2.53125f },
		{ 1.4375f, 0.0f, 2.53125f }, { 1.4375f, -0.805f, 2.53125f }, { 0.805f, -1.4375f, 2.53125f }, { 0.0f, -1.4375f, 2.53125f },
		{ 1.5f, 0.0f, 2.4f }, { 1.5f, -0.84f, 2.4f }, { 0.84f, -1.5f, 2.4f }, { 0.0f, -1.5f, 2.4f },
		{ -0.784f, -1.4f, 2.4f }, { -1.4f, -0.784f, 2.4f }, { -1.4f, 0.0f, 2.4f },
		{ -0.749f, -1.3375f, 2.53125f }, { -1.3375f, -0.749f, 2.53125f }, { -1.3375f, 0.0f, 2.53125f },
		{ -0.805f, -1.4375f, 2.53125f }, { -1.4375f, -0.805f, 2.53125f }, { -1.4375f, 0.0f, 2.53125f },
		{ -0.84f, -1.5f, 2.4f }, { -1.5f, -0.84f, 2.4f }, { -1.5f, 0.0f, 2.4f },
		{ -1.4f, 0.784f, 2.4f }, { -0.784f, 1.4f, 2.4f }, { 0.0f, 1.4f, 2.4f },
		{ -1.3375f, 0.749f, 2.53125f }, { -0.749f, 1.3375f, 2.53125f }, { 0.0f, 1.3375f, 2.53125f },
		{ -1.4375f, 0.805f, 2.53125f }, { -0.805f, 1.4375f, 2.53125f }, { 0.0f, 1.4375f, 2.53125f },
		{ -1.5f, 0.84f, 2.4f }, { -0.84f, 1.5f, 2.4f }, { 0.0f, 1.5f, 2.4f },
		{ 0.784f, 1.4f, 2.4f }, { 1.4f, 0.784f, 2.4f },
		{ 0.749f, 1.3375f, 2.53125f }, { 1.3375f, 0.749f, 2.53125f },
		{ 0.805f, 1.4375f, 2.53125f }, { 1.4375f, 0.805f, 2.53125f },
		{ 0.84f, 1.5f, 2.4f }, { 1.5f, 0.84f, 2.4f },
		{ 1.75f, 0.0f, 1.875f }, { 1.75f, -0.98f, 1.875f }, { 0.98f, -1.75f, 1.875f }, { 0.0f, -1.75f, 1.875f },
		{ 2.0f, 0.0f, 1.35f }, { 2.0f, -1.12f, 1.35f }, { 1.12f, -2.0f, 1.35f }, { 0.0f, -2.0f, 1.35f },
		{ 2.0f, 0.0f, 0.9f }, { 2.0f, -1.12f, 0.9f }, { 1.12f, -2.0f, 0.9f }, { 0.0f, -2.0f, 0.9f },
		{ -0.98f, -1.75f, 1.875f }, { -1.75f, -0.98f, 1.875f }, { -1.75f, 0.0f, 1.875f },
		{ -1.12f, -2.0f, 1.35f }, { -2.0f, -1.12f, 1.35f }, { -2.0f, 0.0f, 1.35f },
		{ -1.12f, -2.0f, 0.9f }, { -2.0f, -1.12f, 0.9f }, { -2.0f, 0.0f, 0.9f },
		{ -1.75f, 0.98f, 1.875f }, { -0.98f, 1.75f, 1.875f }, { 0.0f, 1.75f, 1.875f },
		{ -2.0f, 1.12f, 1.35f }, { -1.12f, 2.0f, 1.35f }, { 0.0f, 2.0f, 1.35f },
		{ -2.0f, 1.12f, 0.9f }, { -1.12f, 2.0f, 0.9f }, { 0.0f, 2.0f, 0.9f },
		{ 0.98f, 1.75f, 1.875f }, { 1.75f, 0.98f, 1.875f },
		{ 1.12f, 2.0f, 1.35f }, { 2.0f, 1.12f, 1.35f },
		{ 1.12f, 2.0f, 0.9f }, { 2.0f, 1.12f, 0.9f },
		{ 2.0f, 0.0f, 0.45f }, { 2.0f, -1.12f, 0.45f }, { 1.12f, -2.0f, 0.45f }, { 0.0f, -2.0f, 0.45f },
		{ 1.5f, 0.0f, 0.225f }, { 1.5f, -0.84f, 0.225f }, { 0.84f, -1.5f, 0.225f }, { 0.0f, -1.5f, 0.225f },
		{ 1.5f, 0.0f, 0.15f }, { 1.5f, -0.84f, 0.15f }, { 0.84f, -1.5f, 0.15f }, { 0.0f, -1.5f, 0.15f },
		{ -1.12f, -2.0f, 0.45f }, { -2.0f, -1.12f, 0.45f }, { -2.0f, 0.0f, 0.45f },
		{ -0.84f, -1.5f, 0.225f }, { -1.5f, -0.84f, 0.225f }, { -1.5f, 0.0f, 0.225f },
		{ -0.84f, -1.5f, 0.15f }, { -1.5f, -0.84f, 0.15f }, { -1.5f, 0.0f, 0.15f },
		{ -2.0f, 1.12f, 0.45f }, { -1.12f, 2.0f, 0.45f }, { 0.0f, 2.0f, 0.45f },
		{ -1.5f, 0.84f, 0.225f }, { -0.84f, 1.5f, 0.225f }, { 0.0f, 1.5f, 0.225f },
		{ -1.5f, 0.84f, 0.15f }, { -0.84f, 1.5f, 0.15f }, { 0.0f, 1.5f, 0.15f },
		{ 1.12f, 2.0f, 0.45f }, { 2.0f, 1.12f, 0.45f },
		{ 0.84f, 1.5f, 0.225f }, { 1.5f, 0.84f, 0.225f },
		{ 0.84f, 1.5f, 0.15f }, { 1.5f, 0.84f, 0.15f },
		{ -1.6f, 0.0f, 2.025f }, { -1.6f, -0.3f, 2.025f }, { -1.5f, -0.3f, 2.25f }, { -1.5f, 0.0f, 2.25f },
		{ -2.3f, 0.0f, 2.025f }, { -2.3f, -0.3f, 2.025f }, { -2.5f, -0.3f, 2.25f }, { -2.5f, 0.0f, 2.25f },
		{ -2.7f, 0.0f, 2.025f }, { -2.7f, -0.3f, 2.025f }, { -3.0f, -0.3f, 2.25f }, { -3.0f, 0.0f, 2.25f },
		{ -2.7f, 0.0f, 1.8f }, { -2.7f, -0.3f, 1.8f }, { -3.0f, -0.3f, 1.8f }, { -3.0f, 0.0f, 1.8f },
		{ -1.5f, 0.3f, 2.25f }, { -1.6f, 0.3f, 2.025f },
		{ -2.5f, 0.3f, 2.25f }, { -2.3f, 0.3f, 2.025f },
		{ -3.0f, 0.3f, 2.25f }, { -2.7f, 0.3f, 2.025f },
		{ -3.0f, 0.3f, 1.8f }, { -2.7f, 0.3f, 1.8f },
		{ -2.7f, 0.0f, 1.575f }, { -2.7f, -0.3f, 1.575f }, { -3.0f, -0.3f, 1.35f }, { -3.0f, 0.0f, 1.35f },
		{ -2.5f, 0.0f, 1.125f }, { -2.5f, -0.3f, 1.125f }, { -2.65f, -0.3f, 0.9375f }, { -2.65f, 0.0f, 0.9375f },
		{ -2.0f, -0.3f, 0.9f }, { -1.9f, -0.3f, 0.6f }, { -1.9f, 0.0f, 0.6f },
		{ -3.0f, 0.3f, 1.35f }, { -2.7f, 0.3f, 1.575f },
		{ -2.65f, 0.3f, 0.9375f }, { -2.5f, 0.3f, 1.125f },
		{ -1.9f, 0.3f, 0.6f }, { -2.0f, 0.3f, 0.9f },
		{ 1.7f, 0.0f, 1.425f }, { 1.7f, -0.66f, 1.425f }, { 1.7f, -0.66f, 0.6f }, { 1.7f, 0.0f, 0.6f },
		{ 2.6f, 0.0f, 1.425f }, { 2.6f, -0.66f, 1.425f }, { 3.1f, -0.66f, 0.825f }, { 3.1f, 0.0f, 0.825f },
		{ 2.3f, 0.0f, 2.1f }, { 2.3f, -0.25f, 2.1f }, { 2.4f, -0.25f, 2.025f }, { 2.4f, 0.0f, 2.025f },
		{ 2.7f, 0.0f, 2.4f }, { 2.7f, -0.25f, 2.4f }, { 3.3f, -0.25f, 2.4f }, { 3.3f, 0.0f, 2.4f },
		{ 1.7f, 0.66f, 0.6f }, { 1.7f, 0.66f, 1.425f },
		{ 3.1f, 0.66f, 0.825f }, { 2.6f, 0.66f, 1.425f },
		{ 2.4f, 0.25f, 2.025f }, { 2.3f, 0.25f, 2.1f },
		{ 3.3f, 0.25f, 2.4f }, { 2.7f, 0.25f, 2.4f },
		{ 2.8f, 0.0f, 2.475f }, { 2.8f, -0.25f, 2.475f }, { 3.525f, -0.25f, 2.49375f }, { 3.525f, 0.0f, 2.49375f },
		{ 2.9f, 0.0f, 2.475f }, { 2.9f, -0.15f, 2.475f }, { 3.45f, -0.15f, 2.5125f }, { 3.45f, 0.0f, 2.5125f },
		{ 2.8f, 0.0f, 2.4f }, { 2.8f, -0.15f, 2.4f }, { 3.2f, -0.15f, 2.4f }, { 3.2f, 0.0f, 2.4f },
		{ 3.525f, 0.25f, 2.49375f }, { 2.8f, 0.25f, 2.475f },
		{ 3.45f, 0.15f, 2.5125f }, { 2.9f, 0.15f, 2.475f },
		{ 3.2f, 0.15f, 2.4f }, { 2.8f, 0.15f, 2.4f },
		{ 0.0f, 0.0f, 3.15f }, { 0.0f, -0.002f, 3.15f }, { 0.002f, 0.0f, 3.15f },
		{ 0.8f, 0.0f, 3.15f }, { 0.8f, -0.45f, 3.15f }, { 0.45f, -0.8f, 3.15f }, { 0.0f, -0.8f, 3.15f },
		{ 0.0f, 0.0f, 2.85f },
		{ 0.2f, 0.0f, 2.7f }, { 0.2f, -0.112f, 2.7f }, { 0.112f, -0.2f, 2.7f }, { 0.0f, -0.2f, 2.7f },
		{ -0.002f, 0.0f, 3.15f }, { -0.45f, -0.8f, 3.15f }, { -0.8f, -0.45f, 3.15f }, { -0.8f, 0.0f, 3.15f },
		{ -0.112f, -0.2f, 2.7f }, { -0.2f, -0.112f, 2.7f }, { -0.2f, 0.0f, 2.7f },
		{ 0.0f, 0.002f, 3.15f }, { -0.8f, 0.45f, 3.15f }, { -0.45f, 0.8f, 3.15f }, { 0.0f, 0.8f, 3.15f },
		{ -0.2f, 0.112f, 2.7f }, { -0.112f, 0.2f, 2.7f }, { 0.0f, 0.2f, 2.7f },
		{ 0.45f, 0.8f, 3.15f }, { 0.8f, 0.45f, 3.15f },
		{ 0.112f, 0.2f, 2.7f }, { 0.2f, 0.112f, 2.7f },
		{ 0.4f, 0.0f, 2.55f }, { 0.4f, -0.224f, 2.55f }, { 0.224f, -0.4f, 2.55f }, { 0.0f, -0.4f, 2.55f },
		{ 1.3f, 0.0f, 2.55f }, { 1.3f, -0.728f, 2.55f }, { 0.728f, -1.3f, 2.55f }, { 0.0f, -1.3f, 2.55f },
		{ 1.3f, 0.0f, 2.4f }, { 1.3f, -0.728f, 2.4f }, { 0.728f, -1.3f, 2.4f }, { 0.0f, -1.3f, 2.4f },
		{ -0.224f, -0.4f, 2.55f }, { -0.4f, -0.224f, 2.55f }, { -0.4f, 0.0f, 2.55f },
		{ -0.728f, -1.3f, 2.55f }, { -1.3f, -0.728f, 2.55f }, { -1.3f, 0.0f, 2.55f },
		{ -0.728f, -1.3f, 2.4f }, { -1.3f, -0.728f, 2.4f }, { -1.3f, 0.0f, 2.4f },
		{ -0.4f, 0.224f, 2.55f }, { -0.224f, 0.4f, 2.55f }, { 0.0f, 0.4f, 2.55f },
		{ -1.3f, 0.728f, 2.55f }, { -0.728f, 1.3f, 2.55f }, { 0.0f, 1.3f, 2.55f },
		{ -1.3f, 0.728f, 2.4f }, { -0.728f, 1.3f, 2.4f }, { 0.0f, 1.3f, 2.4f },
		{ 0.224f, 0.4f, 2.55f }, { 0.4f, 0.224f, 2.55f },
		{ 0.728f, 1.3f, 2.55f }, { 1.3f, 0.728f, 2.55f },
		{ 0.728f, 1.3f, 2.4f }, { 1.3f, 0.728f, 2.4f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.5f, 0.0f, 0.15f }, { 1.5f, 0.84f, 0.15f }, { 0.84f, 1.5f, 0.15f }, { 0.0f, 1.5f, 0.15f },
		{ 1.5f, 0.0f, 0.075f }, { 1.5f, 0.84f, 0.075f }, { 0.84f, 1.5f, 0.075f }, { 0.0f, 1.5f, 0.075f },
		{ 1.425f, 0.0f, 0.0f }, { 1.425f, 0.798f, 0.0f }, { 0.798f, 1.425f, 0.0f }, { 0.0f, 1.425f, 0.0f },
		{ -0.84f, 1.5f, 0.15f }, { -1.5f, 0.84f, 0.15f }, { -1.5f, 0.0f, 0.15f },
		{ -0.84f, 1.5f, 0.075f }, { -1.5f, 0.84f, 0.075f }, { -1.5f, 0.0f, 0.075f },
		{ -0.798f, 1.425f, 0.0f }, { -1.425f, 0.798f, 0.0f }, { -1.425f, 0.0f, 0.0f },
		{ -1.5f, -0.84f, 0.15f }, { -0.84f, -1.5f, 0.15f }, { 0.0f, -1.5f, 0.15f },
		{ -1.5f, -0.84f, 0.075f }, { -0.84f, -1.5f, 0.075f }, { 0.0f, -1.5f, 0.075f },
		{ -0.798f, -1.425f, 0.0f }, { -1.425f, -0.798f, 0.0f }, { 0.0f, -1.425f, 0.0f },
		{ 0.84f, -1.5f, 0.15f }, { 1.5f, -0.84f, 0.15f },
		{ 0.84f, -1.5f, 0.075f }, { 1.5f, -0.84f, 0.075f },
		{ 1.425f, -0.798f, 0.0f }, { 0.798f, -1.425f, 0.0f },
	};

	static_assert( sizeof( g_vertices ) / sizeof( g_vertices[0] ) == 306, "The teapot has 306 control points" );
	static_assert( sizeof( g_patches ) / sizeof( g_patches[0] ) == 32, "The teapot has 32 patches" );

	// Converts from the Z up teapot data to Gaffer's Y up.
	V3f controlPoint( int oneBasedIndex )
	{
		const float *v = g_vertices[oneBasedIndex - 1];
		return V3f( v[0], v[2], -v[1] );
	}

	array<float, 4> bernstein( float t )
	{
		const float s = 1.0f - t;
		return { s * s * s, 3.0f * t * s * s, 3.0f * t * t * s, t * t * t };
	}

	// Merges points closer than a tolerance, so the patches join up into one mesh.
	class Welder
	{
		public :

			Welder( float tolerance, V3fVectorData *points )
				: m_tolerance( tolerance ), m_points( points->writable() )
			{
			}

			int add( const V3f &p )
			{
				const V3i cell = cellOf( p );
				for( int x = -1; x <= 1; ++x )
				{
					for( int y = -1; y <= 1; ++y )
					{
						for( int z = -1; z <= 1; ++z )
						{
							auto it = m_cells.find( key( cell + V3i( x, y, z ) ) );
							if( it == m_cells.end() )
							{
								continue;
							}
							for( int index : it->second )
							{
								if( ( m_points[index] - p ).length2() <= m_tolerance * m_tolerance )
								{
									return index;
								}
							}
						}
					}
				}

				const int index = (int)m_points.size();
				m_points.push_back( p );
				m_cells[key( cell )].push_back( index );
				return index;
			}

		private :

			V3i cellOf( const V3f &p ) const
			{
				return V3i(
					(int)std::floor( p.x / m_tolerance ),
					(int)std::floor( p.y / m_tolerance ),
					(int)std::floor( p.z / m_tolerance )
				);
			}

			static uint64_t key( const V3i &c )
			{
				return ( (uint64_t)( c.x & 0x1fffff ) << 42 ) | ( (uint64_t)( c.y & 0x1fffff ) << 21 ) | (uint64_t)( c.z & 0x1fffff );
			}

			const float m_tolerance;
			vector<V3f> &m_points;
			unordered_map<uint64_t, vector<int>> m_cells;
	};
}

IE_CORE_DEFINERUNTIMETYPED( YutaTeapot );

size_t YutaTeapot::g_firstPlugIndex = 0;

YutaTeapot::YutaTeapot( const std::string &name )
	: ObjectSource( name, "teapot" )
{
	storeIndexOfNextChild( g_firstPlugIndex );

	addChild( new FloatPlug( "size", Plug::In, 1.0f, 0.0f ) );
	addChild( new IntPlug( "divisions", Plug::In, 8, 1, 64 ) );
	addChild( new BoolPlug( "lid", Plug::In, true ) );
	addChild( new BoolPlug( "bottom", Plug::In, true ) );
}

Gaffer::FloatPlug *YutaTeapot::sizePlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex );
}

const Gaffer::FloatPlug *YutaTeapot::sizePlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex );
}

Gaffer::IntPlug *YutaTeapot::divisionsPlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::IntPlug *YutaTeapot::divisionsPlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 1 );
}

Gaffer::BoolPlug *YutaTeapot::lidPlug()
{
	return getChild<BoolPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::BoolPlug *YutaTeapot::lidPlug() const
{
	return getChild<BoolPlug>( g_firstPlugIndex + 2 );
}

Gaffer::BoolPlug *YutaTeapot::bottomPlug()
{
	return getChild<BoolPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::BoolPlug *YutaTeapot::bottomPlug() const
{
	return getChild<BoolPlug>( g_firstPlugIndex + 3 );
}

void YutaTeapot::affects( const Plug *input, AffectedPlugsContainer &outputs ) const
{
	ObjectSource::affects( input, outputs );

	if(
		input == sizePlug() ||
		input == divisionsPlug() ||
		input == lidPlug() ||
		input == bottomPlug()
	)
	{
		outputs.push_back( sourcePlug() );
	}
}

void YutaTeapot::hashSource( const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	sizePlug()->hash( h );
	divisionsPlug()->hash( h );
	lidPlug()->hash( h );
	bottomPlug()->hash( h );
}

IECore::ConstObjectPtr YutaTeapot::computeSource( const Context *context ) const
{
	const float size = sizePlug()->getValue();
	const int divisions = std::max( 1, divisionsPlug()->getValue() );
	const bool lid = lidPlug()->getValue();
	const bool bottom = bottomPlug()->getValue();

	IntVectorDataPtr verticesPerFaceData = new IntVectorData;
	IntVectorDataPtr vertexIdsData = new IntVectorData;
	V3fVectorDataPtr pointsData = new V3fVectorData;
	V2fVectorDataPtr uvData = new V2fVectorData;
	uvData->setInterpretation( GeometricData::UV );

	vector<int> &verticesPerFace = verticesPerFaceData->writable();
	vector<int> &vertexIds = vertexIdsData->writable();
	vector<V2f> &uvs = uvData->writable();

	Welder welder( 1e-4f, pointsData.get() );

	const int rowSize = divisions + 1;
	vector<int> grid( rowSize * rowSize );
	vector<array<float, 4>> weights( rowSize );
	for( int i = 0; i < rowSize; ++i )
	{
		weights[i] = bernstein( (float)i / (float)divisions );
	}

	for( const Patch &patch : g_patches )
	{
		if( ( patch.part == Lid && !lid ) || ( patch.part == Bottom && !bottom ) )
		{
			continue;
		}

		V3f cps[16];
		for( int i = 0; i < 16; ++i )
		{
			cps[i] = controlPoint( patch.indices[i] );
		}

		// Row r runs along the patch's first parametric direction ( v ), column c along the second ( u ).
		for( int r = 0; r < rowSize; ++r )
		{
			for( int c = 0; c < rowSize; ++c )
			{
				V3f p( 0.0f );
				for( int i = 0; i < 4; ++i )
				{
					for( int j = 0; j < 4; ++j )
					{
						p += cps[i * 4 + j] * ( weights[r][i] * weights[c][j] );
					}
				}
				grid[r * rowSize + c] = welder.add( p );
			}
		}

		for( int r = 0; r < divisions; ++r )
		{
			for( int c = 0; c < divisions; ++c )
			{
				const int corners[4][2] = { { r, c }, { r, c + 1 }, { r + 1, c + 1 }, { r + 1, c } };

				// Skip repeated points, so the quads at the lid knob and the
				// centre of the bottom become triangles.
				int faceIds[4];
				V2f faceUVs[4];
				int numVertices = 0;
				for( const auto &corner : corners )
				{
					const int id = grid[corner[0] * rowSize + corner[1]];
					if( numVertices && ( id == faceIds[numVertices - 1] || id == faceIds[0] ) )
					{
						continue;
					}
					faceIds[numVertices] = id;
					faceUVs[numVertices] = V2f( (float)corner[1] / (float)divisions, 1.0f - (float)corner[0] / (float)divisions );
					++numVertices;
				}

				if( numVertices < 3 )
				{
					continue;
				}

				verticesPerFace.push_back( numVertices );
				vertexIds.insert( vertexIds.end(), faceIds, faceIds + numVertices );
				uvs.insert( uvs.end(), faceUVs, faceUVs + numVertices );
			}
		}
	}

	for( V3f &p : pointsData->writable() )
	{
		p *= size;
	}

	MeshPrimitivePtr result = new MeshPrimitive( verticesPerFaceData, vertexIdsData, "linear", pointsData );
	result->variables["uv"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, uvData );
	result->variables["N"] = MeshAlgo::calculateNormals( result.get() );

	return result;
}
