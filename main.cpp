#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// جزء خاص بتجاهل تحذيرات معينة من المترجم تخص مكتبة تحميل الصور
#pragma warning(push)
#pragma warning(disable: 6262) 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // مكتبة بسيطة لتحميل ملفات الصور (JPG, PNG)
#pragma warning(pop)
// --- إعدادات النافذة ---
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// --- متغيرات الكاميرا والشخصية ---
glm::vec3 cameraPos = glm::vec3(0.0f, 1.6f, 5.0f);//عدلت
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 playerPos = glm::vec3(0.0f, 0.0f, 5.0f);// متغير لموقع اللاعب
glm::vec3 playerSize(0.5f, 1.0f, 0.5f);//حجم اللاعب
glm::vec3 keyPos = glm::vec3(0.0f, 0.5f, -55.0f);//متغير لموقع المفتاح
glm::vec3 keySize(0.5f, 0.5f, 0.5f);//حجم المفتاح
glm::vec3 housePos = glm::vec3(0.0f, 0.5f, -90.0f); // متغير لموقع الكوخ
glm::vec3 houseSize = glm::vec3(2.0f, 2.0f, 2.0f); //متغير لموقع الكوخ

glm::vec3 wallPos[] = {//اماكن الجدران
	glm::vec3(3.0f, 1.0f, -10.0f),
	glm::vec3(-3.0f, 1.0f, -20.0f),
	glm::vec3(3.0f, 1.0f, -30.0f),
	glm::vec3(-3.0f, 1.0f, -40.0f),
	glm::vec3(3.0f, 1.0f, -50.0f),
	glm::vec3(-3.0f, 1.0f, -60.0f)
};
//اماكن الشجر
glm::vec3 treePos[] = {
	//صف اول قريب
	glm::vec3(-6.0f, 0.0f, -10.0f),
	glm::vec3(6.0f, 0.0f, -10.0f),
	//صف ثاني بالوسط
	glm::vec3(-7.0f, 0.0f, -20.0f),
	glm::vec3(7.0f, 0.0f, -20.0f),
	//صف ثالث ابعد
	glm::vec3(-8.0f, 0.0f, -30.0f),
	glm::vec3(8.0f, 0.0f, -30.0f),
	//صف رابع بعيد
	glm::vec3(-9.0f, 0.0f, -40.0f),
	glm::vec3(9.0f, 0.0f, -40.0f),

	glm::vec3(-10.0f, 0.0f, -50.0f),
	glm::vec3(10.0f, 0.0f, -50.0f),

	glm::vec3(-11.0f, 0.0f, -60.0f),
	glm::vec3(11.0f, 0.0f, -60.0f)
};
// --- متغيرات الفأرة والزوايا ---
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool hasKey = false;
//متغيرات الوقت 
float deltaTime = 0.0f;
float lastFrame = 0.0f;
//متغيرات للفوز والخسارة
bool gameOver = false;
bool win = false;
float gameTime = 0.0f;
// --- نصوص المظلات (Shaders) المعدلة ---
// Vertex Shader: الآن يستقبل اللون كمدخل ثاني (location = 1) ويمرره
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n" // احداثيات النقطة (x,y,z)
"layout(location = 1) in vec2 aTexCoord;\n"//احداثيات الصورة (u,v)
"out vec2 TexCoord;\n"
"out vec3 FragPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"TexCoord = aTexCoord;"
" FragPos = vec3(model * vec4(aPos, 1.0));\n"
" gl_Position = projection * view * model * vec4(aPos, 1.0);\n" //يحسب مكان النقطة على الشاشة
"}\0";
// Fragment Shader: الآن يستقبل اللون من Vertex Shader ويعرضه
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n" //لون النهائي
"in vec3 FragPos;\n"//موقع النقطة
"in vec2 TexCoord;\n"
"uniform sampler2D texture1; \n"
"uniform vec3 objectColor;\n"
"uniform vec3 cameraPos;\n"
"uniform float fogDensity;\n"
////الاضاءة 
"uniform vec3 lightPos;\n"
"uniform bool win;\n"
///
"uniform vec3 fogColor;\n"
"uniform bool useTexture;\n"
"void main()\n"
"{\n"
//المسافة بين الكاميرا والنقطة
"float distance = length(cameraPos - FragPos);\n"
//كل ما بعد الضباب بزيد
"float fogFactor = exp(-fogDensity * distance * 0.01);\n"
"fogFactor = clamp(fogFactor, 0.0, 1.0);\n"
"\n"
"vec3 color;\n"
"if(useTexture)\n"
"    color = texture(texture1, TexCoord).rgb;\n"
"else\n"
"    color = objectColor;\n"
"\n"
"vec3 finalColor = mix(fogColor, color, fogFactor);\n"
"\n"
/////////عدلتتت
"// ===== الإضاءة عند الفوز =====\n"
"if(win){\n"//كل ماقربت الضوء بصير اقوى
"float lightDist = length(lightPos - FragPos);\n"
"float intensity = 1.0 / (lightDist * lightDist * 0.2 + 0.05);\n"
"vec3 lightColor = vec3(1.0, 0.9, 0.6);\n"
//يضيف اضاءة
"finalColor += lightColor * intensity;\n"
"}\n"
//////////
"FragColor = vec4(finalColor, 1.0f);\n"
"}\n";
//لرسم الشريط hud
const char* hudVertexShader = "#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}\0";
const char* hudFragmentShader = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 color;\n"
"void main()\n"
"{\n"
"FragColor = vec4(color, 1.0);\n"
"}\n";
// --- التصاريح المسبقة للدوال ---
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void processInput(GLFWwindow * window);
//دالة للتصادم
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
	// تهيئة مكتبة GLFW وإعداد النافذة
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "مشروع الكاميرا والمكعب - ملون", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "فشل في إنشاء نافذة GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// ربط دوال الاستدعاء (Callbacks)
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	// إخفاء مؤشر الفأرة والتقاطه داخل النافذة 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// تهيئة مكتبة GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "فشل في تهيئة مكتبة GLEW" << std::endl;
		return -1;
	}
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	// تفعيل اختبار العمق (Depth Testing)
	glEnable(GL_DEPTH_TEST);
	// تهيئة textures
	unsigned int groundTexture;
	glGenTextures(1, &groundTexture);//نحجز مكانها ب gpu
	glBindTexture(GL_TEXTURE_2D, groundTexture);

	// wrap تكرار
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// filter الجودة 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load image
	stbi_set_flip_vertically_on_load(true);//مننقلب الصورة
	int width, height, nrChannels;
	unsigned char* data = stbi_load("ground.png", &width, &height, &nrChannels, 0);//منحمل الصورة على ram

	if (data)
	{
		if (nrChannels == 3)//بينقل الصورة من من ram للgpu
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if (nrChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//تحسين الاداء  يعني لما بعد الصورة بتضل واضحة 
		glGenerateMipmap(GL_TEXTURE_2D);

	}
	else
	{
		std::cout << "Failed to load texture\n";
	}
	stbi_image_free(data);//بتحذف من ram
	// --- بناء المظلات (Compile Shaders) ---
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	unsigned int shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// --- بناء المظلات ( hud Compile Shaders) ---
	unsigned int hudVS = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(hudVS, 1, &hudVertexShader, NULL);
	glCompileShader(hudVS);

	unsigned int hudFS = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(hudFS, 1, &hudFragmentShader, NULL);
	glCompileShader(hudFS);

	unsigned int hudProgram = glCreateProgram();
	glAttachShader(hudProgram, hudVS);
	glAttachShader(hudProgram, hudFS);
	glLinkProgram(hudProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	// --- إعداد نقاط المكعب والألوان الجديدة ---
	// كل سطر يمثل نقطة: 3 إحداثيات (x, y, z) + 3 ألوان (r, g, b)
	float vertices[] = {
		// الوجه الخلفي (أحمر)
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,

		// الوجه الأمامي (أخضر)
		-0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,

		// الوجه الأيسر (أزرق)
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

		// الوجه الأيمن (أصفر)
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

		 // الوجه السفلي (سماوي)
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
		  0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
		  0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
		  0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
		 -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,

		 // الوجه العلوي (بنفسجي)
		 -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
		  0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
		 -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
		 -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f
	};
	//احداثيات لرسم الارض ب EBO
	float groundVertices[] = {
		// positions        // tex coords u,v
		 -1.0f, 0.0f, -1.0f,  0.0f, 0.0f,
		  1.0f, 0.0f, -1.0f,  5.0f, 0.0f,
		  1.0f, 0.0f,  1.0f,  5.0f, 5.0f,
		 -1.0f, 0.0f,  1.0f,  0.0f, 5.0f
	};
	//vao بخزن البيانات 
	//ينظم كيف نقرا البيانات vbo
	unsigned int VBO, VAO;
	unsigned int groundIndices[] =
	{ 0, 1, 2,
	  2, 3, 0 };
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// 1. مؤشر الإحداثيات (الموقع)
	// الخطوة (Stride) أصبحت 6 الآن بدلاً من 3 (3 إحداثيات + 3 ألوان)
		//vao الارض  // position 
// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glDisableVertexAttribArray(1);

	// ما في texture للمكعب → نعطل attribute 1
	// texture coords (غير مستخدم للمكعب)
	glDisableVertexAttribArray(1);
	glEnableVertexAttribArray(0);


	// VBO,VAO,EBO للارض
	unsigned int groundVAO, groundVBO, groundEBO;

	glGenVertexArrays(1, &groundVAO);
	glGenBuffers(1, &groundVBO);
	glGenBuffers(1, &groundEBO);

	glBindVertexArray(groundVAO);

	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

	// مؤشر الإحداثيات
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// texture coords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	//احداثيات الشريط
	float hudVertices[12];
	//VAO HUD
	unsigned int hudVAO, hudVBO;
	glGenVertexArrays(1, &hudVAO);
	glGenBuffers(1, &hudVBO);

	glBindVertexArray(hudVAO);
	glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	// --- حلقة الرسم الأساسية (Render Loop) ---
	while (!glfwWindowShouldClose(window))
	{
		// الوقت
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		// خلفية الغابة
		glClearColor(0.08f, 0.08f, 0.10f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glUniform1i(glGetUniformLocation(shaderProgram, "win"), win);

		// خلي الضوء بموقع الكوخ
		glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"),
			housePos.x, housePos.y + 2.0f, housePos.z);
		glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
		//ارسالuniform للضباب 
		glUniform3f(glGetUniformLocation(shaderProgram, "cameraPos"),
			cameraPos.x, cameraPos.y, cameraPos.z);
		glUniform3f(glGetUniformLocation(shaderProgram, "fogColor"),
			0.8f, 0.8f, 0.8f); // لون الضباب رمادي
		//
		float fog = 0.01f;
		glUniform1f(glGetUniformLocation(shaderProgram, "fogDensity"), fog);
		// الكاميرا
		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f),
			(float)SCR_WIDTH / (float)SCR_HEIGHT,
			0.1f,
			100.0f
		);

		// أوفست الكاميرا (ورا وفوق اللاعب)
		glm::vec3 offset = glm::vec3(0.0f, 2.0f, -5.0f);

		// لف الأوفست حسب دوران الماوس (yaw)
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0, 1, 0));
		glm::vec3 rotatedOffset = glm::vec3(rot * glm::vec4(offset, 1.0f));

		// موقع الكاميرا النهائي (ورا اللاعب)
		cameraPos = playerPos + rotatedOffset;

		// خلي الكاميرا تطلع على اللاعب
		glm::vec3 target = playerPos + glm::vec3(0.0f, 1.0f, 0.0f);

		// view
		glm::mat4 view = glm::lookAt(
			cameraPos,
			target,
			cameraUp
		);

		//ارسال view لل shader
		glUniformMatrix4fv(
			glGetUniformLocation(shaderProgram, "view"),
			1,
			GL_FALSE,
			glm::value_ptr(view)
		);

		glUniformMatrix4fv(
			glGetUniformLocation(shaderProgram, "projection"),
			1,
			GL_FALSE,
			glm::value_ptr(projection)
		);

		// نجيب مكان model مرة وحدة
		unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");

		glBindVertexArray(VAO);

		// =========================
		// 1.رسم الأرض عن طريق plane ebo (Ground)
		// =========================
		glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, groundTexture);
		glBindVertexArray(groundVAO);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, -50.0f));
		model = glm::scale(model, glm::vec3(50.0f, 0.1f, 120.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//ربط texture
		glBindTexture(GL_TEXTURE_2D, groundTexture);

		//بدل glDrawArrays
		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"),
			1.0f, 1.0f, 1.0f);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		//ترجع تربط VAOتبع المكعب مهم جداً
		glBindVertexArray(VAO);
		glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
		// =========================
		// 2. الطريق (Road)
		// =========================
		glBindTexture(GL_TEXTURE_2D, 0);
		glm::mat4 road = glm::mat4(1.0f);
		road = glm::translate(road, glm::vec3(0.0f, 0.01f, -50.0f));
		road = glm::scale(road, glm::vec3(4.0f, 0.1f, 100.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(road));

		// لون الطريق
		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"),
			0.4f, 0.4f, 0.4f);

		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(VAO);
		// =========================
// اللاعب
// =========================
		glm::mat4 player = glm::mat4(1.0f);
		player = glm::translate(player, playerPos + glm::vec3(0.0f, 0.5f, 0.0f));
		player = glm::scale(player, glm::vec3(0.5f, 1.0f, 0.5f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(player));
		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"),
			0.8f, 0.2f, 0.2f);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		//  شجرة

		// اذا بدي الاشجار ابعد عن الطريق بكبر x
		//الاشجار متقاربة بصغر z

		for (int i = 0; i < 12; i++)
		{
			float scale = 1.0f + (i * 0.15f);

			// --- الجذع ---
			glm::mat4 treeModel = glm::mat4(1.0f);
			treeModel = glm::translate(treeModel, treePos[i] + glm::vec3(0.0f, 1.0f, 0.0f));
			treeModel = glm::scale(treeModel, glm::vec3(0.5f, 2.0f, 0.5f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(treeModel));
			glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"),
				0.3f, 0.2f, 0.1f); // بني الجذع

			glDrawArrays(GL_TRIANGLES, 0, 36);

			// --- الأوراق ---

			treeModel = glm::mat4(1.0f);
			treeModel = glm::translate(treeModel, treePos[i] + glm::vec3(0.0f, 2.5f, 0.0f));
			treeModel = glm::scale(treeModel, glm::vec3(1.5f * scale, 1.5f * scale, 1.5f * scale));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(treeModel));
			glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"),
				0.0f, 0.3f, 0.0f); // أخضر غامق


			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// الاحداثيات:, x :+2 يمين الطريق,: y:-2 يسار الطريق
		//z كل ما تكبر بالسالب بتكون ابعد لقدام 
//  الجدران
		float time = glfwGetTime();
		float moveRange = 2.5f;   // قديش يتحرك يمين/يسار
		float speed = 1.5f;
		for (int i = 0; i < 6; i++) {

			glm::mat4 wall = glm::mat4(1.0f);
			glm::vec3 movingPos = wallPos[i];
			// حركة يمين/يسار
			movingPos.x += sin(time * speed + i) * moveRange;
			wall = glm::translate(wall, movingPos);
			wall = glm::scale(wall, glm::vec3(1.0f, 2.0f, 4.0f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(wall));
			glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"),
				0.4f, 0.25f, 0.1f); // بني

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		//رسم المفتاح
		if (!hasKey)
		{
			glm::mat4 key = glm::mat4(1.0f);
			key = glm::translate(key, keyPos);
			key = glm::scale(key, glm::vec3(0.3f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(key));
			glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"),
				1.0f, 0.84f, 0.0f); // ذهبي

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		//  رسم الكوخ
		glm::mat4 house = glm::mat4(1.0f);
		house = glm::translate(house, housePos);
		house = glm::scale(house, glm::vec3(2.0f, 2.0f, 2.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(house));

		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"),
			0.6f, 0.4f, 0.2f);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//HUD رسم
		glDisable(GL_DEPTH_TEST);
		glUseProgram(hudProgram);
		glBindVertexArray(hudVAO);
		// حساب التقدم
		float currentDistance = abs(playerPos.z);
		float progress = currentDistance / abs(housePos.z);
		if (progress > 1.0f) progress = 1.0f;

		// عرض الشريط حسب التقدم
		float barWidth = -1.0f + (2.0f * progress);

		float dynamicHUD[] = {
			-1.0f,  0.98f,
			 barWidth, 0.98f,
			 barWidth, 0.93f,

			 barWidth, 0.93f,
			-1.0f,  0.93f,
			-1.0f,  0.98f
		};

		// تحديث البيانات داخل GPU
		glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(dynamicHUD), dynamicHUD);
		glm::vec3 hudColor;

		// خسارة
		if (gameOver)
			hudColor = glm::vec3(1.0f, 0.0f, 0.0f);

		// فوز
		else if (win)
			hudColor = glm::vec3(0.0f, 1.0f, 0.0f);

		// معك مفتاح
		else if (hasKey)
			hudColor = glm::vec3(1.0f, 1.0f, 0.0f);

		// لعب عادي
		else
			hudColor = glm::vec3(progress, 0.5f, 1.0f - progress);
		//طباعة فوز وخسارة
		static bool printed = false;

		if (!printed && (win && gameOver))
		{
			if (win)
				std::cout << "YOU WIN!\n";
			else
				std::cout << "GAME OVER!\n";

			printed = true;
		}

		glUniform3f(glGetUniformLocation(hudProgram, "color"),
			hudColor.x, hudColor.y, hudColor.z);

		if (!gameOver && !win)
		{
			gameTime += deltaTime;
		}

		float distance = abs(playerPos.z);
		// عرضهم (مبدئياً Console)
		std::cout << "Time: " << gameTime
			<< " | Distance: " << distance << "\r";
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);

		//عرض الإطار
		// =========================
		glfwSwapBuffers(window);
		glfwPollEvents();
	}




	// تنظيف الذاكرة قبل الإغلاق
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();
	return 0;
}

//input,game logic
// --- معالجة مدخلات لوحة المفاتيح (الأسهم) ---
void processInput(GLFWwindow * window)
{
	if (gameOver || win)
		return;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float speed = 5.0f * deltaTime;
	//حركة مؤقتة
	glm::vec3 newPos = playerPos;

	// لقدام
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		newPos.z -= speed;

	// لورا
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		newPos.z += speed;

	// يمين
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		newPos.x += speed;

	// يسار
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		newPos.x -= speed;

	// تحقق التصادم 

	bool hitWall = false;
	glm::vec3 treeSize(1.0f, 2.0f, 1.0f);
	//التصادم مع الشجر
	for (int i = 0; i < 12; i++)
	{
		if (checkCollision(newPos, playerSize, treePos[i], treeSize))
		{
			hitWall = true;
		}
	}//التصادم مع الجدران
	glm::vec3 wallSize(1.5f, 2.0f, 4.0f);
	for (int i = 0; i < 6; i++)
	{
		glm::vec3 movingWallPos = wallPos[i];
		movingWallPos.x += sin(glfwGetTime() * 1.5f + i) * 2.5f;

		if (checkCollision(newPos, playerSize, movingWallPos, wallSize))
		{
			gameOver = true;
			hitWall = true;
			break;
		}
	}

	//دالة للمفتاح 
	if (!hasKey)
	{
		if (checkCollision(newPos, playerSize, keyPos, keySize))
		{
			hasKey = true;
			std::cout << "Got the key !\n";
		}
	}
	//  الكوخ الفوز
	if (!gameOver && !win)
	{
		if (checkCollision(newPos, playerSize, housePos, houseSize))
		{
			if (hasKey)
			{
				win = true;
				std::cout << "YOU WIN!\n";
			}
			else
			{
				std::cout << "You need the key !\n";
			}
		}
	}
	// إذا ما في تصادم  تحرك
	if (!hitWall)
	{
		playerPos = newPos;
	}
}
// --- معالجة حركة الفأرة (الالتفاف الحر) ---
void mouse_callback(GLFWwindow * window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cameraFront = glm::normalize(front);
}

// --- معالجة تغير حجم النافذة ---
void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	glViewport(0, 0, width, height);
}