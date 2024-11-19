#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "shader.h"
#include <string.h>
#include <omp.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
float function_x(float x);
void RebindBuffer(GLuint* VAO, GLuint* VBO);

GLuint VAO[2];
GLuint VBO[2];

float x_range = 2;
float step = 0.01f;
int total_points = x_range / step;
float* points = new float[total_points * 2];

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 800, "Learn OpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to Create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, 800, 800);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	constexpr float start_x_position  = -1.0f;
	constexpr float start_y_position  = 1.0f;
	constexpr float offset = 0.1;
	float x_coord;
	float y_coord;
	constexpr size_t number_of_vertices = 20;

	float* vertices = new float[number_of_vertices * 2 * 2];

	int index = 0;

	#pragma omp target
	#pragma omp loop
	for(index = 0; index < number_of_vertices * 2; index += 2)
	{
		x_coord = start_x_position + (offset * index/2);

		vertices[index] = x_coord;
		vertices[index + 1] = start_y_position;
	}

	#pragma omp target
	#pragma omp loop
	for(index = number_of_vertices * 2; index < number_of_vertices * 2 * 2; index += 2)
	{
		y_coord = start_y_position - offset * (index/2 - number_of_vertices);

		vertices[index] = start_x_position;
		vertices[index + 1] = y_coord;
	}

	// Refactored Loop
	x_coord = -1 * (x_range / 2);
	y_coord = 0.0f;
	float new_x_coord = 0.0f;

	#pragma omp target
	#pragma omp loop
	for(int i = 0; i < total_points * 2; i += 2)
	{
		new_x_coord = x_coord + step * (i/2);
		y_coord = function_x(new_x_coord);

		points[i] = static_cast<float>(new_x_coord) / (static_cast<float>(x_range) / 2);
		points[i + 1] = y_coord;
	}

	glGenVertexArrays(1, &VAO[0]);
	glGenVertexArrays(1, &VAO[1]);

	glGenBuffers(1, &VBO[0]);
	glGenBuffers(1, &VBO[1]);

	glBindVertexArray(VAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, (number_of_vertices * 2 * 2) * sizeof(float), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindVertexArray(0);

	// -----------------------------------

	glBindVertexArray(VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, (total_points * 2 ) * sizeof(float), points, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	Shader ourShader("vertexShader.glsl", "geometryShader.glsl", "fragmentShader.glsl");
	Shader functionShader("vertexShader_graph.glsl", "fragmentShader_graph.glsl");

	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);

		// rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ourShader.use();
		//ourShader.setFloat("horizontal_offset", 1.0f);
		glBindVertexArray(VAO[0]);
		ourShader.setInt("switcher", 0);
		glDrawArrays(GL_POINTS, 0, number_of_vertices);
		ourShader.setInt("switcher", 1);
		glDrawArrays(GL_POINTS, number_of_vertices, number_of_vertices);
		glBindVertexArray(0);

		glPointSize(10);
		functionShader.use();
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_LINE_STRIP, 0, total_points);
		glBindVertexArray(0);

		// check and call event and swap buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	delete[] vertices;
	delete[] points;

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void RebindBuffer(GLuint* VAO, GLuint* VBO)
{
	total_points = static_cast<int>(x_range / step);

	delete[] points;
	points = new float[total_points * 2];
	
	// Refactored Loop
	float x_coord = -1 * (x_range / 2);
	float y_coord = 0.0f;
	float new_x_coord = 0.0f;

	#pragma omp target
	#pragma omp loop
	for(int i = 0; i < total_points * 2; i += 2)
	{
		new_x_coord = x_coord + step * (i/2);
		y_coord = function_x(new_x_coord);

		points[i] = static_cast<float>(new_x_coord) / (static_cast<float>(x_range) / 2);
		points[i + 1] = y_coord;
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO[1]);

	glGenBuffers(1, &VBO[1]);

	glBindVertexArray(VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, (total_points * 2 ) * sizeof(float), points, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	/*
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	float* mapped_data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	memcpy(mapped_data, points, (total_points * 2 ) * sizeof(float));
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	*/
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	glfwWaitEvents();
	if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		x_range += 0.1f;
		if(x_range > 220)
		{
			x_range = 220;
		}
		RebindBuffer(VAO, VBO);
	}
	glfwWaitEvents();
	if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		x_range -= 0.1f;
		if(x_range < 2)
		{
			x_range = 2;
		}
		RebindBuffer(VAO, VBO);
	}
}
float function_x(float x)
{
	return glm::sin(x);
	//return x * x;
}