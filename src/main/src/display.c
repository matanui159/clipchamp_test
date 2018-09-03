#include "display.h"
#include <SDL/SDL.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <assert.h>

#define GLSL(code) #code
#define BUFFER_SIZE 256

static GLuint g_textures[3];

static void compile_shader(GLuint program, GLenum type, const char* code) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, NULL);
	glCompileShader(shader);

	char infolog[BUFFER_SIZE];
	glGetShaderInfoLog(shader, BUFFER_SIZE, NULL, infolog);
	puts(infolog);
	glAttachShader(program, shader);
}

static void texture_data(int texture, GLsizei width, GLsizei height, const byte_t* data) {
	glActiveTexture(GL_TEXTURE0 + texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
}

void display_init() {
	int result = SDL_Init(SDL_INIT_VIDEO);
	assert(result == 0);

	SDL_Surface* surface = SDL_SetVideoMode(640, 360, 0, SDL_OPENGL);
	assert(surface != NULL);

	glClear(GL_COLOR_BUFFER_BIT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GLuint program = glCreateProgram();

	compile_shader(program, GL_VERTEX_SHADER, GLSL(
		attribute vec2 vpos;
		varying vec2 fpos;

		void main() {
			fpos = vpos;
			gl_Position = vec4(
				vpos.x * 2.0 - 1.0,
				1.0 - vpos.y * 2.0,
				0.0, 1.0
			);
		}
	));

	compile_shader(program, GL_FRAGMENT_SHADER, GLSL(
		precision mediump float;
		varying vec2 fpos;

		uniform sampler2D Y;
		uniform sampler2D U;
		uniform sampler2D V;

		void main() {
			float y = texture2D(Y, fpos).a;
			float u = texture2D(U, fpos).a - 0.5;
			float v = texture2D(V, fpos).a - 0.5;
			gl_FragColor = vec4(
				y + 1.402*v,
				y - 0.344*u - 0.714*v,
				y + 1.772*u,
				1.0
			);
		}
	));

	glLinkProgram(program);
	char infolog[BUFFER_SIZE];
	glGetProgramInfoLog(program, BUFFER_SIZE, NULL, infolog);
	puts(infolog);
	glUseProgram(program);

	const byte_t buffer_data[] = {
		0, 0,
		0, 1,
		1, 0,
		1, 1
	};

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(buffer_data), buffer_data, GL_STATIC_DRAW);
	GLint attrib = glGetAttribLocation(program, "vpos");
	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, 2, GL_BYTE, GL_FALSE, 0, NULL);

	glGenTextures(3, g_textures);
	glUniform1i(glGetUniformLocation(program, "Y"), 0);
	glUniform1i(glGetUniformLocation(program, "U"), 1);
	glUniform1i(glGetUniformLocation(program, "V"), 2);
	for (int i = 0; i < 3; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, g_textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	assert(glGetError() == GL_NO_ERROR);
}

void display_frame(int width, int height, const byte_t* data) {
	texture_data(0, width, height, data);
	texture_data(1, width / 2, height / 2, data += width * height);
	texture_data(2, width / 2, height / 2, data += width * height / 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}