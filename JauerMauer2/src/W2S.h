#pragma once
#include "../pch.h"

glm::vec4 Multiply(glm::vec4 vec, std::vector<float> mat)
{
	return glm::vec4(
		vec.x * mat[0] + vec.y * mat[4] + vec.z * mat[8] + vec.w * mat[12],
		vec.x * mat[1] + vec.y * mat[5] + vec.z * mat[9] + vec.w * mat[13],
		vec.x * mat[2] + vec.y * mat[6] + vec.z * mat[10] + vec.w * mat[14],
		vec.x * mat[3] + vec.y * mat[7] + vec.z * mat[11] + vec.w * mat[15]
	);
}

bool WorldToScreen(glm::vec3 pointInWorld, glm::vec2& screen, std::vector<float> modelView, std::vector<float> projection, std::vector<int> viewPort)
{
	glm::vec4 worldPos(pointInWorld, 1.0f);
	glm::vec4 clip = Multiply(Multiply(worldPos, modelView), projection);

	if (clip.w == 0.0f)
		return false;

	glm::vec3 ndc = glm::vec3(clip.x, clip.y, clip.z) / clip.w;

	if (ndc.z < -1.0f || ndc.z > 1.0f)
		return false;

	screen.x = (ndc.x + 1.0f) * 0.5f * viewPort[2] + viewPort[0];
	screen.y = (1.0f - ndc.y) * 0.5f * viewPort[3] + viewPort[1];

	return true;
}
