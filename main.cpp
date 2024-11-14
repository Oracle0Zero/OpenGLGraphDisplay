#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int heighr);
void processInput(GLFWwindow* window);
float function_x_square(float x);

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

	float start_x_position  = -1.0f;
	float start_y_position  = 1.0f;
	float offset = 0.1;
	constexpr size_t number_of_vertices = 20;

	float* vertices = new float[number_of_vertices * 2 * 2];

	int i = 0;
	for(i = 0; i < number_of_vertices * 2 * 2; i += 2)
	{
		if(i < 40)
		{
			vertices[i] = start_x_position;
			vertices[i + 1] = start_y_position;

			//printf("start_x_position: %f ", vertices[i]);
			//printf("start_y_position: %f\n", vertices[i + 1]);

			start_x_position += offset;
		}else
		{
			//start_y_position  = 1.0f;
			start_x_position  = -1.0f;

			
			vertices[i] = start_x_position;
			vertices[i + 1] = start_y_position;

			//printf("start_x_position: %f ", vertices[i]);
			//printf("start_y_position: %f\n", vertices[i + 1]);

		
			start_y_position -= offset;
		}
	}

	int x_range = 6;
	float step = 0.01f;
	int total_points = x_range / step;
	float* points = new float[total_points * 2];
	int hopper = 0;
	for(float i = -x_range/2; i < x_range/2; i+=step)
	{
		//float x_cord = static_cast<float>(i) / (static_cast<float>(total_points) / 2);
		//float y_cord = function_x_square(x_cord);

		float x_cord = i;
		float y_cord = function_x_square(x_cord);  

				
		printf("x_cord: %f ", x_cord);
		printf("y_cord: %f\n", y_cord);

		x_cord = static_cast<float>(x_cord) / (static_cast<float>(x_range) / 2);
		//if(glm::abs(function_x_square((static_cast<float>(x_range) / 2))) < 1.0f && glm::abs(function_x_square((static_cast<float>(x_range) / 2))) > -1.0f)
		//{
			y_cord = static_cast<float>(y_cord);
		//}else
		//{
			//y_cord = static_cast<float>(y_cord)/glm::abs(function_x_square((static_cast<float>(x_range) / 2)));
		//}
		 

		points[hopper] = x_cord;
		points[hopper + 1] = y_cord;



		hopper += 2;
	}

	unsigned int VAO[2];
	glGenVertexArrays(1, &VAO[0]);
	glGenVertexArrays(1, &VAO[1]);

	unsigned int VBO[2];
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
	glBufferData(GL_ARRAY_BUFFER, (total_points * 2 ) * sizeof(float), points, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	delete[] vertices;
	delete[] points;

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
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}
float function_x_square(float x)
{
	return glm::sin(x);
	//return x * x;
}