#pragma once
#include "../pch.h"


namespace GLH {
	//colors
	//GLH::

	GLubyte red[3] = { 255, 0, 0 };
	GLubyte green[3] = { 0,255,0 };
	GLubyte white[3] = { 255,255,255 };

	void SetupOrtho() {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glViewport(0, 0, viewport[2], viewport[3]);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
	}
	void RestoreGL() {
		glPopMatrix();
		glPopAttrib();
	}

	void DrawOutLine(float x, float y, float width, float height, float lineWidth, const GLubyte color[3], float alpha) {
		glLineWidth(lineWidth);
		glColor4ub(color[0], color[1], color[2], alpha);
		glBegin(GL_LINE_STRIP);
		glVertex2f(x - 0.5f, y - 0.5f);
		glVertex2f(x + width + 0.5f, y - 0.5f);
		glVertex2f(x + width + 0.5f, y + height + 0.5f);
		glVertex2f(x - 0.5f, y + height + 0.5f);
		glVertex2f(x - 0.5f, y - 0.5f);
		glEnd();
	}

	void DrawLine(float x1, float y1, float x2, float y2, float lineWidth, const GLubyte color[3], float alpha) {
		glLineWidth(lineWidth);
		glColor4ub(color[0], color[1], color[2], alpha);
		glBegin(GL_LINES);
		glVertex2f(x1, y1);
		glVertex2f(x2, y2);
		glEnd();
	}

	// Additional useful functions
	void DrawFilledRect(float x, float y, float width, float height, const GLubyte color[3], float alpha) {
		glColor4ub(color[0], color[1], color[2], alpha);
		glBegin(GL_QUADS);
		glVertex2f(x, y);
		glVertex2f(x + width, y);
		glVertex2f(x + width, y + height);
		glVertex2f(x, y + height);
		glEnd();
	}

	void DrawCircle(float x, float y, float radius, int segments, const GLubyte color[3], float alpha){
		glColor4ub(color[0], color[1], color[2],alpha);
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < segments; i++) {
			float theta = 2.0f * 3.1415 * float(i) / float(segments);
			float dx = radius * cosf(theta);
			float dy = radius * sinf(theta);
			glVertex2f(x + dx, y + dy);
		}
		glEnd();
	}

	void DrawCrosshair(float x, float y, float size, float lineWidth, const GLubyte color[3], float alpha) {
		DrawLine(x - size, y, x + size, y, lineWidth, color,alpha);
		DrawLine(x, y - size, x, y + size, lineWidth, color,alpha);
	}
}