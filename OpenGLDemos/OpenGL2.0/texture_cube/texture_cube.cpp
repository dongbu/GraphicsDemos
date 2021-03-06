/*
If you got errors with libSOIL.lib, please use the src to build the lib instead of just renaming libSOIL.a to libSOIL.lib.
SOIL - Simple OpenGL Image Library
main page www.lonesock.net/soil.html 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl/glew.h>
#include <gl/glut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

GLuint program;
GLuint vbo_cube_vertices;
GLuint vbo_cube_texcoords;
GLuint ibo_cube_elements;

// Texture
GLuint texture_id;
GLint  uniform_mytexture;

GLint  attribute_coord3d;		// vertex coordinates
GLint  attribute_texcoord;		// vertex texture coordinates
GLint  uniform_mvp;				// World-View-Projection matrix

int screen_width  = 800;
int screen_height = 800;

// Read shader source from file
char* file_read(const char* filename)
{
	FILE* input = NULL;
	fopen_s(&input, filename, "rb");
	if (input == NULL)
		return NULL;

	if (fseek(input, 0, SEEK_END) == -1)
		return NULL;

	long size = ftell(input);
	if (size == -1)
		return NULL;

	if (fseek(input, 0, SEEK_SET) == -1)
		return NULL;

	char* content = (char*)malloc((size_t)size + 1);
	if (content == NULL)
		return NULL;

	fread(content, 1, (size_t)size, input);
	if (ferror(input))
	{
		free(content);
		return NULL;
	}

	fclose(input);
	content[size] = '\0';

	return content;
}

// Print shader errors
void print_log(GLuint object)
{
	GLint log_length = 0;

	if (glIsShader(object))
	{
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	}
	else if (glIsProgram(object))
	{
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	}
	else
	{
		fprintf(stderr, "printlog: Not a shader or a program\n");
		return;
	}

	char* log = (char*)malloc(log_length);

	if (glIsShader(object))
	{
		glGetShaderInfoLog(object, log_length, NULL, log);
	}
	else if (glIsProgram(object))
	{
		glGetProgramInfoLog(object, log_length, NULL, log);
	}

	fprintf(stderr, "%s", log);
	free(log);
	system("pause"); // Pause the console window to let user see the error.
}

GLuint create_shader(const char* filename, GLenum type)
{
	const GLchar* source = file_read(filename);
	if (source == NULL)
	{
		fprintf(stderr, "Error opening %s: ", filename);
		system("pause");
		return 0;
	}

	// Read in shader source from file
	const GLchar* sources[2] = 
	{
#ifdef GL_ES_VERSION_2_0
		"#version 100\n"
		"#define GLES2\n",
#else
		"#version 120\n",
#endif
		source
	};

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 2, sources, NULL);
	free((void*)source);

	glCompileShader(shader);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE)
	{
		fprintf(stderr, "%s compile failed!\n", filename);
		print_log(shader);
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

void init_buffers()
{
	GLfloat cube_vertices[] = 
	{
		// front
		-1.0, -1.0,  1.0,
		 1.0, -1.0,  1.0,
		 1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// top
		-1.0,  1.0,  1.0,
		 1.0,  1.0,  1.0,
		 1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0,
		// back
		 1.0, -1.0, -1.0,
		-1.0, -1.0, -1.0,
		-1.0,  1.0, -1.0,
		 1.0,  1.0, -1.0,
		// bottom
		-1.0, -1.0, -1.0,
		 1.0, -1.0, -1.0,
		 1.0, -1.0,  1.0,
		-1.0, -1.0,  1.0,
		// left
		-1.0, -1.0, -1.0,
		-1.0, -1.0,  1.0,
		-1.0,  1.0,  1.0,
		-1.0,  1.0, -1.0,
		// right
		 1.0, -1.0,  1.0,
		 1.0, -1.0, -1.0,
		 1.0,  1.0, -1.0,
		 1.0,  1.0,  1.0,
	};

	glGenBuffers(1, &vbo_cube_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	GLushort cube_elements[] = {
		// front
		0,  1,  2,
		2,  3,  0,
		// top
		4,  5,  6,
		6,  7,  4,
		// back
		8,  9, 10,
		10, 11,  8,
		// bottom
		12, 13, 14,
		14, 15, 12,
		// left
		16, 17, 18,
		18, 19, 16,
		// right
		20, 21, 22,
		22, 23, 20,
	};

	glGenBuffers(1, &ibo_cube_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
}

void init_textures()
{
	// Way 1
	/*int img_width;
	int img_height;
	unsigned char* img = SOIL_load_image("star.png", &img_width, &img_height, NULL, 0);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);*/
	
	// Way 2, this is simple than way 1
	glActiveTexture(GL_TEXTURE0);
	texture_id = SOIL_load_OGL_texture("box.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (texture_id == 0)
	{
		fprintf(stderr, "%s", "SOIL loading error");
	}

	// each texcoord has 2 fileds u and v
	// each face has 4 texcoord.
	// there are totally 6 faces.
	GLfloat cube_texcoords[2 * 4 * 6] =
	{
		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0,
		0.0, 1.0,
	};
	for (int i = 1; i < 6; i++)
		memcpy(&cube_texcoords[i * 4 * 2], &cube_texcoords[0], 2 * 4 * sizeof(GLfloat));

	glGenBuffers(1, &vbo_cube_texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texcoords), cube_texcoords, GL_STATIC_DRAW);
}

int init_shaders(void)
{
	GLuint vertex_shader = create_shader("texture_cube.vs", GL_VERTEX_SHADER);
	if (vertex_shader == 0)
		return 0;

	GLuint fragment_shader = create_shader("texture_cube.fs", GL_FRAGMENT_SHADER);
	if (fragment_shader == 0)
		return 0;

	// Link vertex shader and fragment shader
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		fprintf(stderr, "glLinkProgram: ");
		print_log(program);
	}

	// Bind shader variable coord2d
	const char* attribute_name = "coord3d"; // the name must be same as the one in shader file
	attribute_coord3d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord3d == -1)
	{
		fprintf(stderr, "Cound not bind attribute %s\n", attribute_name);
		return 0;
	}

	// Bind model-view-projection matrix
	const char* uniform_name;
	uniform_name = "mvp";
	uniform_mvp = glGetUniformLocation(program, uniform_name);
	if (uniform_mvp == -1) {
		fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
		return 0;
	}

	// Bind vertex texture coordinate
	attribute_name = "texcoord";
	attribute_texcoord = glGetAttribLocation(program, attribute_name);
	if (attribute_texcoord == -1)
	{
		fprintf(stderr, "Cound not bind attribute %s\n", attribute_name);
		return 0;
	}

	return 1;
}

void onDisplay()
{
	// Enable alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable Depth test
	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	// Set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	uniform_mytexture = glGetUniformLocation(program, "mytexture");
	glUniform1i(uniform_mytexture, 0);

	glEnableVertexAttribArray(attribute_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoords);
	glVertexAttribPointer(attribute_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Set vertex coordinates
	glEnableVertexAttribArray(attribute_coord3d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glVertexAttribPointer(attribute_coord3d, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Drawing by index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	int size;  
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(attribute_coord3d);

	glutSwapBuffers();
}

void onIdle()
{
	// Animation
	float angle = glutGet(GLUT_ELAPSED_TIME) / 1000.0 * 5;  // 45� per second
	glm::vec3 axis_y(0.0, 1.0, 0.0);
	glm::mat4 anim = glm::rotate(glm::mat4(1.0f), angle, axis_y);

	// Mode matrix
	glm::mat4 model = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -4.0));

	// View matrix
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 1.0, 0.0));

	// Projection matrix
	glm::mat4 projection = glm::perspective(45.0f, 1.0f * screen_width / screen_height, 0.1f, 10.0f);

	// Multiply all matrix together
	glm::mat4 mvp = projection * view * model * anim;

	glUseProgram(program);
	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

	glutPostRedisplay();
}

void cleanup()
{
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_cube_vertices);
	glDeleteBuffers(1, &ibo_cube_elements);
	glDeleteTextures(1, &texture_id);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutCreateWindow("Cube");

	// glewInit Must be called after glutInit
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return EXIT_FAILURE;
	}

	if (init_shaders())
	{
		init_buffers();
		init_textures();
		glutDisplayFunc(onDisplay);
		glutIdleFunc(onIdle);
		glutMainLoop();
	}

	cleanup();

	return 0;
}
