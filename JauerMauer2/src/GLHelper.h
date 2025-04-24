#pragma once

#include "../pch.h"

void DrawOutLine(float x, float y, float width, float height, float lineWidth, const GLubyte color[3]) {
	glLineWidth(lineWidth);
	glBegin(GL_LINE_STRIP);
	glVertex2f(x - 0.5f, y - 0.5f);
	glVertex2f(x + width + 0.5f, y - 0.5f);
	glVertex2f(x + width + 0.5f, y + height + 0.5f);
	glVertex2f(x - 0.5f, y + height + 0.5f);
	glVertex2f(x - 0.5f, y - 0.5f);
	glEnd();
}

