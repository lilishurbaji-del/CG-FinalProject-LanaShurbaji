
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// --- إعدادات النافذة ---
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// --- الكاميرا ---
glm::vec3 cameraPos(0.0f, 1.6f, 5.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

// --- اللاعب ---
glm::vec3 playerPos(0.0f, 0.0f, 5.0f);
glm::vec3 playerSize(0.8f, 1.0f, 0.8f);

// --- المفتاح ---
glm::vec3 keyPos(0.0f, 0.5f, -55.0f);
glm::vec3 keySize(0.5f, 0.5f,0.5f);

// --- الكوخ ---
glm::vec3 housePos(0.0f, 0.5f, -90.0f);
glm::vec3 houseSize(2.0f);

// --- الجدران ---
glm::vec3 wallPos[] = {
	{3,1,-10},{-3,1,-20},{3,1,-30},
	{-3,1,-40},{3,1,-50},{-3,1,-60}
};

// --- الأشجار ---
glm::vec3 treePos[] = {
	{-6,0,-10},{6,0,-10},{-7,0,-20},{7,0,-20},
	{-8,0,-30},{8,0,-30},{-9,0,-40},{9,0,-40},
	{-10,0,-50},{10,0,-50},{-11,0,-60},{11,0,-60}
};

// --- ماوس ---
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400;
float lastY = 300;

// --- حالة اللعبة ---
bool hasKey = false;
bool gameOver = false;
bool win = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float gameTime = 0.0f;

// ================= SHADERS =================

const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"out vec3 FragPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"FragPos = vec3(model * vec4(aPos,1.0));\n"
"gl_Position = projection * view * model * vec4(aPos,1.0);\n"
"}";
const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 FragPos;\n"
"uniform vec3 objectColor;\n"
"uniform vec3 cameraPos;\n"
"uniform float fogDensity;\n"
"uniform vec3 lightPos;\n"
"uniform bool win;\n"
"void main(){\n"
"float dist = length(cameraPos - FragPos);\n"
"float fog = exp(-fogDensity * dist * 0.03);\n"
"fog = clamp(fog,0.0,1.0);\n"
"vec3 fogColor = vec3(0.05,0.05,0.08);\n"
"vec3 color = mix(fogColor, objectColor, fog);\n"

"// ===== الإضاءة عند الفوز =====\n"
"if(win){\n"
"float lightDist = length(lightPos - FragPos);\n"
"float intensity = 1.0 / (lightDist * lightDist * 0.5 + 0.1);\n"
"vec3 lightColor = vec3(1.0, 0.9, 0.6);\n"
"color += lightColor * intensity;\n"
"}\n"

"FragColor = vec4(color,1.0);\n"
"}";
// HUD
const char* hudVS =
"#version 330 core\n"
"layout (location=0) in vec2 aPos;\n"
"void main(){gl_Position = vec4(aPos,0,1);}";

const char* hudFS =
"#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 color;\n"
"void main(){FragColor = vec4(color,1);}";

// --- Functions ---
void processInput(GLFWwindow*);
void mouse_callback(GLFWwindow*, double, double);
void framebuffer_size_callback(GLFWwindow*, int, int);

bool checkCollision(glm::vec3 aPos, glm::vec3 aSize,
	glm::vec3 bPos, glm::vec3 bSize)
{
	return (aPos.x < bPos.x + bSize.x &&
		aPos.x + aSize.x > bPos.x &&
		aPos.z < bPos.z + bSize.z &&
		aPos.z + aSize.z > bPos.z);
}
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Game", NULL, NULL);

	if (!window) {
		std::cout << "GLFW error\n";
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();
	glEnable(GL_DEPTH_TEST);

	// ===== SHADERS =====
	unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertexShaderSource, NULL);
	glCompileShader(vs);

	unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragmentShaderSource, NULL);
	glCompileShader(fs);

	unsigned int shader = glCreateProgram();
	glAttachShader(shader, vs);
	glAttachShader(shader, fs);
	glLinkProgram(shader);

	// HUD
	unsigned int hudVSid = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(hudVSid, 1, &hudVS, NULL);
	glCompileShader(hudVSid);

	unsigned int hudFSid = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(hudFSid, 1, &hudFS, NULL);
	glCompileShader(hudFSid);

	unsigned int hudShader = glCreateProgram();
	glAttachShader(hudShader, hudVSid);
	glAttachShader(hudShader, hudFSid);
	glLinkProgram(hudShader);

	// ===== CUBE (36 vertices FIXED) =====
	float vertices[] = {
		// back
		-0.5,-0.5,-0.5, 1,0,0,  0.5,-0.5,-0.5, 1,0,0,  0.5,0.5,-0.5, 1,0,0,
		 0.5,0.5,-0.5, 1,0,0, -0.5,0.5,-0.5, 1,0,0, -0.5,-0.5,-0.5, 1,0,0,
		 // front
		 -0.5,-0.5,0.5, 0,1,0, 0.5,-0.5,0.5, 0,1,0, 0.5,0.5,0.5, 0,1,0,
		  0.5,0.5,0.5, 0,1,0, -0.5,0.5,0.5, 0,1,0, -0.5,-0.5,0.5, 0,1,0,
		  // left
		  -0.5,0.5,0.5, 0,0,1, -0.5,0.5,-0.5, 0,0,1, -0.5,-0.5,-0.5, 0,0,1,
		  -0.5,-0.5,-0.5, 0,0,1, -0.5,-0.5,0.5, 0,0,1, -0.5,0.5,0.5, 0,0,1,
		  // right
		   0.5,0.5,0.5, 1,1,0, 0.5,0.5,-0.5, 1,1,0, 0.5,-0.5,-0.5, 1,1,0,
		   0.5,-0.5,-0.5, 1,1,0, 0.5,-0.5,0.5, 1,1,0, 0.5,0.5,0.5, 1,1,0,
		   // bottom
		   -0.5,-0.5,-0.5, 0,1,1, 0.5,-0.5,-0.5, 0,1,1, 0.5,-0.5,0.5, 0,1,1,
			0.5,-0.5,0.5, 0,1,1, -0.5,-0.5,0.5, 0,1,1, -0.5,-0.5,-0.5, 0,1,1,
			// top
			-0.5,0.5,-0.5, 1,0,1, 0.5,0.5,-0.5, 1,0,1, 0.5,0.5,0.5, 1,0,1,
			 0.5,0.5,0.5, 1,0,1, -0.5,0.5,0.5, 1,0,1, -0.5,0.5,-0.5, 1,0,1
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// ===== الأرض (GROUND) =====
	float groundVertices[] = {
		-1,0,-1,
		 1,0,-1,
		 1,0, 1,
		-1,0, 1
	};

	unsigned int groundIndices[] = { 0,1,2, 2,3,0 };

	unsigned int groundVAO, groundVBO, groundEBO;
	glGenVertexArrays(1, &groundVAO);
	glGenBuffers(1, &groundVBO);
	glGenBuffers(1, &groundEBO);

	glBindVertexArray(groundVAO);

	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// ===== HUD =====
	float hudVertices[] = { 12 };

	unsigned int hudVAO, hudVBO;
	glGenVertexArrays(1, &hudVAO);
	glGenBuffers(1, &hudVBO);

	glBindVertexArray(hudVAO);
	glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, NULL, GL_DYNAMIC_DRAW);


	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//لحساب التقدم
	float maxDistance = abs(housePos.z); // المسافة الكاملة للنهاية
// ===== KEY HUD =====
	unsigned int keyHUDVAO, keyHUDVBO;
	glGenVertexArrays(1, &keyHUDVAO);
	glGenBuffers(1, &keyHUDVBO);

	glBindVertexArray(keyHUDVAO);
	glBindBuffer(GL_ARRAY_BUFFER, keyHUDVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Game Loop: Update -> Render -> Input
	// ===== Render Loop =====
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader);
		glUniform1i(glGetUniformLocation(shader, "win"), win);
		glUniform3f(glGetUniformLocation(shader, "lightPos"),
			housePos.x, housePos.y + 2.0f, housePos.z);

		// ضباب
		glUniform3f(glGetUniformLocation(shader, "cameraPos"),
			cameraPos.x, cameraPos.y, cameraPos.z);

		float fog = 0.03f + sin(glfwGetTime()) * 0.01f;
		glUniform1f(glGetUniformLocation(shader, "fogDensity"), fog);

		// Projection
		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f),
			(float)SCR_WIDTH / SCR_HEIGHT,
			0.1f, 100.0f
		);

		// كاميرا خلف اللاعب
		glm::vec3 offset(0, 2, 5);
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f),
			glm::radians(yaw),
			glm::vec3(0, 1, 0));

		cameraPos = playerPos + glm::vec3(rot * glm::vec4(offset, 1.0f));
		glm::vec3 target = playerPos + glm::vec3(0, 1, 0);

		glm::mat4 view = glm::lookAt(cameraPos, target, cameraUp);
		cameraFront = glm::normalize(target - cameraPos);
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
			1, GL_FALSE, glm::value_ptr(view));

		glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));

		unsigned int modelLoc =
			glGetUniformLocation(shader, "model");
		// ================= الأرض =================
		glBindVertexArray(groundVAO);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, -50.0f));
		model = glm::scale(model, glm::vec3(50.0f, 0.1f, 120.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3f(glGetUniformLocation(shader, "objectColor"),
			0.25f, 0.15f, 0.05f);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(VAO);

		// ================= الطريق =================
		glm::mat4 road = glm::mat4(1.0f);
		road = glm::translate(road, glm::vec3(0.0f, 0.01f, -50.0f));
		road = glm::scale(road, glm::vec3(4.0f, 0.1f, 100.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(road));
		glUniform3f(glGetUniformLocation(shader, "objectColor"),
			0.4f, 0.4f, 0.4f);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ================= اللاعب =================
		glm::mat4 player = glm::mat4(1.0f);
		player = glm::translate(player, playerPos + glm::vec3(0, 0.5f, 0));
		player = glm::scale(player, glm::vec3(0.5f, 1.0f, 0.5f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(player));
		glUniform3f(glGetUniformLocation(shader, "objectColor"),
			0.8f, 0.2f, 0.2f);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ================= الأشجار =================
		for (int i = 0; i < 12; i++)
		{
			float scale = 1.0f + (i * 0.15f);

			glm::mat4 tree = glm::mat4(1.0f);
			tree = glm::translate(tree, treePos[i] + glm::vec3(0, 1, 0));
			tree = glm::scale(tree, glm::vec3(0.5f, 2.0f, 0.5f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tree));
			glUniform3f(glGetUniformLocation(shader, "objectColor"),
				0.3f, 0.2f, 0.1f);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			tree = glm::mat4(1.0f);
			tree = glm::translate(tree, treePos[i] + glm::vec3(0, 2.5f, 0));
			tree = glm::scale(tree, glm::vec3(1.5f * scale));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tree));
			glUniform3f(glGetUniformLocation(shader, "objectColor"),
				0.0f, 0.3f, 0.0f);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// ================= الجدران =================
		float time = glfwGetTime();

		for (int i = 0; i < 6; i++)
		{
			glm::vec3 movingPos = wallPos[i];
			movingPos.x += sin(time * 1.5f + i) * 2.5f;

			glm::mat4 wall = glm::mat4(1.0f);
			wall = glm::translate(wall, movingPos);
			wall = glm::scale(wall, glm::vec3(1.0f, 2.0f, 4.0f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(wall));
			glUniform3f(glGetUniformLocation(shader, "objectColor"),
				0.4f, 0.25f, 0.1f);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		// ================= المفتاح =================
		if (!hasKey)
		{
			glBindVertexArray(VAO); 

			glm::mat4 key = glm::mat4(1.0f);
			key = glm::translate(key, keyPos);
			key = glm::scale(key, glm::vec3(0.3f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(key));

			glUniform3f(glGetUniformLocation(shader, "objectColor"),
				1.0f, 0.84f, 0.0f);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// ================= الكوخ =================
		glm::mat4 house = glm::mat4(1.0f);
		house = glm::translate(house, housePos);
		house = glm::scale(house, glm::vec3(2.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(house));
		glUniform3f(glGetUniformLocation(shader, "objectColor"),
			0.6f, 0.4f, 0.2f);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ================= HUD =================
		glDisable(GL_DEPTH_TEST);
		glUseProgram(hudShader);
		// ================= KEY HUD =================
		glBindVertexArray(VAO);
		glUseProgram(hudShader);
		glBindVertexArray(keyHUDVAO);

		float keyHUD[] = {
		  -0.85f, 0.90f,
		  -0.65f, 0.90f,
		  -0.65f, 0.70f,

		  -0.65f, 0.70f,
		  -0.85f, 0.70f,
		  -0.85f, 0.90f
		};

		glBindBuffer(GL_ARRAY_BUFFER, keyHUDVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(keyHUD), keyHUD);

		if (hasKey)
			glUniform3f(glGetUniformLocation(hudShader, "color"), 1.0f, 0.84f, 0.0f);
		else
			glUniform3f(glGetUniformLocation(hudShader, "color"), 0.3f, 0.3f, 0.3f);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		// ================= PROGRESS BAR =================
		glBindVertexArray(hudVAO);

		float currentDistance = fabs(playerPos.z);
		float progress = currentDistance / fabs(housePos.z);
		if (progress > 1.0f) progress = 1.0f;

		float barWidth = -1.0f + (2.0f * progress);

		float dynamicHUD[] = {
			-1.0f,  0.85f,
			 barWidth, 0.85f,
			 barWidth, 0.80f,

			 barWidth, 0.80f,
			-1.0f,  0.80f,
			-1.0f,  0.85f
		};

		glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(dynamicHUD), dynamicHUD);

		glm::vec3 hudColor;

		if (gameOver)
			hudColor = glm::vec3(1, 0, 0);
		else if (win)
			hudColor = glm::vec3(0, 1, 0);
		else
			hudColor = glm::vec3(0.2f, 0.8f, 1.0f);

		static bool printed = false;

		if (!printed && (win || gameOver))
		{
			if (win)
				std::cout << "YOU WIN!\n";
			else
				std::cout << "GAME OVER!\n";

			printed = true;
		}

		glUniform3f(glGetUniformLocation(hudShader, "color"),
			hudColor.x, hudColor.y, hudColor.z);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
void processInput(GLFWwindow * window)
{
	if (gameOver || win) return;

	float speed = 6.0f * deltaTime;
	glm::vec3 newPos = playerPos;
	glm::vec3 forward = glm::normalize(glm::vec3(
		sin(glm::radians(yaw)),
		0,
		-cos(glm::radians(yaw))
	));

	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		newPos += forward * speed;

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		newPos -= forward * speed;

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		newPos += right * speed;

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		newPos -= right * speed;
	bool hit = false;

	// AABB Collision Detection:
// يقارن حدود الكائنات على XZ axis لمنع الاختراق
	glm::vec3 treeSize(1, 2, 1);
	for (int i = 0; i < 12; i++)
		if (checkCollision(newPos, playerSize, treePos[i], treeSize))
			hit = true;

	glm::vec3 wallSize(1.5f, 2.0f, 4.0f);
	for (int i = 0; i < 6; i++)
	{
		glm::vec3 pos = wallPos[i];
		pos.x += sin(glfwGetTime() * 1.5f + i) * 2.5f;

		if (checkCollision(newPos, playerSize, pos, wallSize))
		{
			gameOver = true;
			hit = true;
		}
	}

	if (!hasKey && checkCollision(newPos, playerSize, keyPos, keySize))
		hasKey = true;

	if (checkCollision(newPos, playerSize, housePos, houseSize))
		if (hasKey) win = true;

	if (!hit) playerPos = newPos;
}

void mouse_callback(GLFWwindow*, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoff = xpos - lastX;
	float yoff = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	float sens = 0.1f;
	yaw += xoff * sens;
	pitch += yoff * sens;

	if (pitch > 89) pitch = 89;
	if (pitch < -89) pitch = -89;
}

void framebuffer_size_callback(GLFWwindow*, int w, int h)
{
	glViewport(0, 0, w, h);
}