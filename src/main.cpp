#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string.h>
#include <omp.h>
#include <stack>
#include <algorithm>
#include <vector>
#include <map>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <ft2build.h>
#include FT_FREETYPE_H  

#include "../include/shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
float function_x(float x);
void RebindBuffer(GLuint* VAO, GLuint* VBO);
bool IsOperator(char c);
int Precedence(char c);
float PerformOperation(char ope, float number_1, float number_2);
void RenderText(Shader &s, std::string text, float x, float y, float scale, glm::vec3 color);

GLuint VAO[3];
GLuint VBO[3];

float x_range = 2;
float step = 0.01f;
int total_points = x_range / step;
float* points = new float[total_points * 2];

float f = 0.0f;

GLuint gridShader, functionShader;

//std::string expression_string = "x^3-15*x+4";
std::string expression_string = "x";
std::vector<char> expression_chars;
char input[100];

bool isLastCharNumber = false;

std::stack<float> operands;
std::stack<float> operators;

struct Character
{
	GLuint Texture2D;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	long int Advance;
};

std::map<char, Character> Characters;

constexpr float screen_width = 900.0f;
constexpr float screen_height = 900.0f;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Graph Display", NULL, NULL);
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
	glViewport(0, 0, screen_width, screen_height);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}

	FT_Face face;
	if (FT_New_Face(ft, "fonts/BitterPro-Medium.ttf", 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  
		return -1;
	}
	FT_Set_Pixel_Sizes(face, 0, 48);  

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
  
	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// now store character for later use
		Character character = {
			texture, 
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

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

	#pragma omp target
	#pragma omp loop
	for(int i = 0; i < total_points * 2; i += 2)
	{
		new_x_coord = x_coord + step * (i/2);
		y_coord = function_x(new_x_coord);

		points[i] = static_cast<float>(new_x_coord) / (static_cast<float>(x_range) / 2);
		points[i + 1] = y_coord / (static_cast<float>(x_range) / 2);
	}

	glGenVertexArrays(1, &VAO[0]);
	glGenVertexArrays(1, &VAO[1]);

	glGenBuffers(1, &VBO[0]);
	glGenBuffers(1, &VBO[1]);

	glBindVertexArray(VAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, (number_of_vertices * 2 * 2) * sizeof(float), vertices, GL_DYNAMIC_DRAW);

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

	// -------------------------------

	glGenVertexArrays(1, &VAO[2]);
	glGenBuffers(1, &VBO[2]);
	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);   

	Shader ourShader("./shaders/vertexShader.glsl", "./shaders/geometryShader.glsl", "./shaders/fragmentShader.glsl");
	Shader functionShader("./shaders/vertexShader_graph.glsl", "./shaders/fragmentShader_graph.glsl");
	Shader fontShader("./shaders/vertexShader_Fonts.glsl", "./shaders/fragmentShader_Fonts.glsl");


	float initial_x_position = 0.0f;
	float initial_y_position = 0.0f;
	float initial_x_value = 0.0f;
	float initial_y_value = 0.0f;
	//glm::vec3 font_color(0.5, 0.8f, 0.2f);
	glm::vec3 font_color(0.8f, 0.8f, 0.8f);

	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::SetWindowSize(ImVec2(600, 250));
		ImGui::SetWindowFontScale(2.0f);
		
		ImGui::InputText("Input", input, IM_ARRAYSIZE(input));
		if(ImGui::Button("Save", ImVec2(100, 100)))
		{
			expression_string = input;
			x_range = 2;
			RebindBuffer(VAO, VBO);
		}

		if(ImGui::SliderFloat("xy-range", &x_range, 2.0f, 200.0f))
		{
			RebindBuffer(VAO, VBO);
		}

		// rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ourShader.use();
		ourShader.setFloat("horizontal_offset", 1.0f);
		glBindVertexArray(VAO[0]);
		ourShader.setInt("switcher", 0);
		glDrawArrays(GL_POINTS, 0, number_of_vertices);
		ourShader.setInt("switcher", 1);
		glUniform1i(glGetUniformLocation(gridShader, "switcher"), 1); 
		glDrawArrays(GL_POINTS, number_of_vertices, number_of_vertices);
		glBindVertexArray(0);

		glPointSize(10);
		functionShader.use();
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_LINE_STRIP, 0, total_points);
		glBindVertexArray(0);

		// Font Rendering
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
		glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 800.0f);
		fontShader.use();
    	glUniformMatrix4fv(glGetUniformLocation(fontShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		std::string font_to_draw = std::to_string(-1*x_range/2);
		std::string::size_type dot_location = font_to_draw.find(".");
		std::string final_string = font_to_draw.substr(0, dot_location) + "." + font_to_draw.at(dot_location+1) + font_to_draw.at(dot_location+2);

		initial_x_position = 20.0f;
		initial_y_position = 75.0f;
		initial_x_value = -1*x_range/2 + x_range/(number_of_vertices);
		for(int i = 0; i < number_of_vertices-1; i++)
		{
			font_to_draw = std::to_string(initial_x_value);
			initial_x_value += x_range/(number_of_vertices);
			dot_location = font_to_draw.find(".");
			final_string = font_to_draw.substr(0, dot_location) + "." + font_to_draw.at(dot_location+1) + font_to_draw.at(dot_location+2);
			RenderText(fontShader, final_string, initial_x_position + 40.0f*i, screen_height/2.0f - initial_y_position, 0.3f, font_color);
		}

		initial_x_position = 40.0f;
		initial_y_position = 30.0f;
		initial_y_value = -1*x_range/2 + x_range/(number_of_vertices);
		for(int i = 0; i < number_of_vertices-1; i++)
		{
			if(i == ((number_of_vertices-1)/2))
			{
				initial_y_value += x_range/(number_of_vertices);
				continue;
			}
			font_to_draw = std::to_string(initial_y_value);
			initial_y_value += x_range/(number_of_vertices);
			dot_location = font_to_draw.find(".");
			final_string = font_to_draw.substr(0, dot_location) + "." + font_to_draw.at(dot_location+1) + font_to_draw.at(dot_location+2);
			RenderText(fontShader, final_string, screen_width/2.0f - initial_x_position, initial_y_position + 40.0f*i, 0.3f, font_color);
		}

		// Rendering
		// (Your code clears your framebuffer, renders your other stuff etc.)
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// (Your code calls glfwSwapBuffers() etc.)

		// check and call event and swap buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	delete[] vertices;
	delete[] points;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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
		points[i + 1] = y_coord / (static_cast<float>(x_range) / 2);
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
	while(!operands.empty())
	{
		operands.pop();
	}
	while(!operators.empty())
	{
		operators.pop();
	}
	for(char c : expression_string)
	{
		if(c == 'x')
		{
			operands.push(x);
			isLastCharNumber = true;

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
					while(!operators.empty() && Precedence(c) <= Precedence(operators.top()))
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
			isLastCharNumber = false;
		}else
		{
			if(isLastCharNumber)
			{
				float lastNumber = operands.top();
				operands.pop();
				operands.push(lastNumber*10 + (c - '0'));
			}else
			{
				operands.push(c - '0');
			}
			isLastCharNumber = true;
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

	return operands.top();
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

void RenderText(Shader &s, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    s.use();
    glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO[2]);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.Texture2D);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}