#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "../include/shader.h"
#include <string.h>
#include <omp.h>
#include <stack>
#include <algorithm>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
float function_x(float x);
void RebindBuffer(GLuint* VAO, GLuint* VBO);
bool IsOperator(char c);
int Precedence(char c);
float PerformOperation(char ope, float number_1, float number_2);

GLuint VAO[2];
GLuint VBO[2];

float x_range = 2;
float step = 0.01f;
int total_points = x_range / step;
float* points = new float[total_points * 2];

GLuint gridShader, functionShader;

std::string expression_string = "x^3-5*x";
std::vector<char> expression_chars;

std::stack<float> operands;
std::stack<float> operators;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 800, "Graph Display", NULL, NULL);
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
	float new_y_coord = 0.0f;

	float divider_y = function_x(x_coord);
	divider_y = divider_y < 0 ? -1*divider_y : divider_y;

	#pragma omp target
	#pragma omp loop
	for(int i = 0; i < total_points * 2; i += 2)
	{
		new_x_coord = x_coord + step * (i/2);
		y_coord = function_x(new_x_coord);
		//new_y_coord = y_coord + step * (i/2);
		
		if(y_coord != 0)
		{
			std::cout << y_coord / divider_y << "\n";
		}

		points[i] = static_cast<float>(new_x_coord) / (static_cast<float>(x_range) / 2);
		points[i + 1] = y_coord;

		//points[i + 1] = static_cast<float>(y_coord) / (static_cast<float>(x_range) / 2);
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

	Shader ourShader("./shaders/vertexShader.glsl", "./shaders/geometryShader.glsl", "./shaders/fragmentShader.glsl");
	Shader functionShader("./shaders/vertexShader_graph.glsl", "./shaders/fragmentShader_graph.glsl");

	//gridShader = Utils::createShaderProgram("./shaders/vertexShader.glsl", "./shaders/geometryShader.glsl", "./shaders/fragmentShader.glsl");
	//functionShader = Utils::createShaderProgram("./shaders/vertexShader_graph.glsl", "./shaders/fragmentShader_graph.glsl");

	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);

		// rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ourShader.use();
		ourShader.setFloat("horizontal_offset", 1.0f);
		//glUseProgram(gridShader);
		glBindVertexArray(VAO[0]);
		ourShader.setInt("switcher", 0);
		//glUniform1i(glGetUniformLocation(gridShader, "switcher"), 0); 
		glDrawArrays(GL_POINTS, 0, number_of_vertices);
		ourShader.setInt("switcher", 1);
		glUniform1i(glGetUniformLocation(gridShader, "switcher"), 1); 
		glDrawArrays(GL_POINTS, number_of_vertices, number_of_vertices);
		glBindVertexArray(0);

		glPointSize(10);
		functionShader.use();
		//glUseProgram(functionShader);
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

	float divider_y = function_x(x_coord);
	divider_y = divider_y < 0 ? -1*divider_y : divider_y;

	#pragma omp target
	#pragma omp loop
	for(int i = 0; i < total_points * 2; i += 2)
	{
		new_x_coord = x_coord + step * (i/2);
		y_coord = function_x(new_x_coord);
		
		std::cout << "new_x_coord: " << new_x_coord << "\n";
		std::cout << "y_coord: " << y_coord << "\n";

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
	//return glm::sin(x);

	for(char c : expression_string)
	{
		if(c == 'x')
		{
			operands.push(x);

		}else if(IsOperator(c))
		{
			if(operators.empty())
			{
				operators.push(c);
			}else
			{
				if(Precedence(c) >= Precedence(operators.top()))
				{
					operators.push(c);
				}else
				{
					while(!operators.empty() && Precedence(c) < Precedence(operators.top()))
					{
						char o = operators.top();
						operators.pop();
						
						float number_2 = operands.top();
						operands.pop();
						float number_1 = operands.top();
						operands.pop();

						float result = PerformOperation(o, number_1, number_2);
						operands.push(result);
					}

					operators.push(c);
				}
			}	
		}else
		{
			operands.push(c - '0');
		}
	}

	while(!operators.empty())
	{
		char o = operators.top();
		operators.pop();

		float number_2 = operands.top();
		operands.pop();
		float number_1 = operands.top();
		operands.pop();

		float result = PerformOperation(o, number_1, number_2);
		operands.push(result);	
	}

	//std::cout << "Input: " << x << "\n";
	//std::cout << "Result: " << operands.top() << "\n";

	return operands.top();
	//return x * x;
}

bool IsOperator(char c)
{
	if(c == '^' || c == '+' || c == '-' || c == '*' || c == '\\')
	{
		return true;
	}

	return false;
}

int Precedence(char c)
{
	switch(c)
	{
		case '+':
		case '-':
			return 1;
		case '*':
		case '/':
			return 2;
		case '^':
			return 3;
	}

	return -1;
}

float PerformOperation(char ope, float number_1, float number_2)
{
	float result = 0.0f;

	switch (ope)
	{
	case '+':
		result = number_1 + number_2;
		break;
	case '-':
		result = number_1 - number_2;
		break;
	case '/':
		result = number_1 / number_2;
		break;
	case '*':
		result = number_1 * number_2;
		break;
	case '^':
		result = pow(number_1, number_2);
		break;
	}

	return result;
}