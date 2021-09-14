#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "global.h"
#include "sphere.h"
#include "detector.h"
#include <iostream>

#define MOVABLE_CAM true


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);


// ������С����
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


// �����
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// ��ײ�����
Detector detector;

// ��ʱ��
float deltaTime = 0.0f;	// ��֮֡���ʱ���
float lastFrame = 0.0f; // ��һ֡


int main()
{
	// glfw ��ʼ��
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw ���ڴ���
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Collision Detection", NULL, NULL);
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	if (MOVABLE_CAM) {
		// ����ص�����
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);
	}

	// ��׽���
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	// glad����OpenGL����
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// ������Ȳ���
	glEnable(GL_DEPTH_TEST);

	// ������ɫ������
	Shader shader("resources/shader/vertex.vs", "resources/shader/frag.fs");

	// ����������
	detector.generateBalls(NUM_BALLS);

	// ��������
	Sphere sphere;
	sphere.buildSphere();
	sphere.generateIndices();

	unsigned int ballVBO;
	unsigned int ballVAO;

	glGenVertexArrays(1, &ballVAO);
	glGenBuffers(1, &ballVBO);
	glBindVertexArray(ballVAO);
	glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
	glBufferData(GL_ARRAY_BUFFER, sphere.vertices.size() * sizeof(float), &sphere.vertices[0], GL_STATIC_DRAW);

	unsigned int ballEBO;
	glGenBuffers(1, &ballEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices.size() * sizeof(int), &sphere.indices[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	// ���÷��䣨Լ����ƽ�棩
	unsigned int planeVBO;
	unsigned int planeVAO;

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PLANE_VERTICES), PLANE_VERTICES, GL_STATIC_DRAW);

	unsigned int planeEBO;
	glGenBuffers(1, &planeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(PLANE_INDICES), PLANE_INDICES, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	float dt = 0.0f;
	// ��Ⱦѭ��
	while (!glfwWindowShouldClose(window))
	{
		// ����ʱ���
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// ����С��
		detector.update(deltaTime, dt);

		// �����ӽ�
		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();

		// �������ͼ�������
		processInput(window);

		// ������ɫ
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// ������ɫ��
		shader.use();
		shader.setVec3("viewPos", camera._pos);
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		glm::vec3 lightDir = glm::vec3(1.0f, -1.0f, 0.0f);
		glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
		shader.setVec3("light.ambient", ambientColor);
		shader.setVec3("light.diffuse", diffuseColor);
		shader.setVec3("light.direction", lightDir);
		shader.setVec3("light.specular", lightColor);

		// ��������
		for (auto b : detector.getBalls()) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, b->pos);
			model = glm::scale(model, glm::vec3(b->radius));
			shader.setMat4("model", model);
			shader.setVec3("material.ambient", b->color);
			shader.setVec3("material.diffuse", b->color);
			shader.setFloat("material.shininess", 32.0f);
			shader.setVec3("material.specular", b->color * 0.6f); 
			glBindVertexArray(ballVAO);
			glPointSize(2);
			glDrawElements(GL_TRIANGLES, sizeof(int) * sphere.indices.size(), GL_UNSIGNED_INT, 0);
		}
		

		// ���Ʒ���
		const glm::vec3 PLANE_COLOR = vec3(0.2f, 0.2f, 0.3f);
		shader.setVec3("material.ambient", PLANE_COLOR);
		shader.setVec3("material.diffuse", PLANE_COLOR);
		shader.setFloat("material.shininess", 32.0f);
		shader.setVec3("material.specular", PLANE_COLOR * 0.6f);
		const glm::mat4 floorTrans = glm::scale(
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, MIN_POS.y, 0.0f)),
			glm::vec3(SIZE)
		);
		const glm::mat4 leftTrans = glm::scale(
			glm::rotate(
				glm::translate(glm::mat4(1.0f), glm::vec3(MAX_POS.x, 0.0f, 0.0f)), 
				glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::vec3(SIZE)
		);

		const glm::mat4 rightTrans = glm::scale(
			glm::rotate(
				glm::rotate(
					glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, MIN_POS.z)),
					glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)
				),
				glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)
			),
			glm::vec3(SIZE)
		);

		shader.setMat4("model", floorTrans);
		glBindVertexArray(planeVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		shader.setMat4("model", leftTrans);
		glBindVertexArray(planeVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		shader.setMat4("model", rightTrans);
		glBindVertexArray(planeVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		// ��������
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	// �����ڴ�
	glDeleteVertexArrays(1, &ballVAO);
	glDeleteBuffers(1, &ballVBO);
	glDeleteBuffers(1, &ballEBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &planeEBO);
	glfwTerminate();
	return 0;
}

// �����¼�
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (MOVABLE_CAM) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.processKey(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.processKey(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.processKey(LEFTWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.processKey(RIGHTWARD, deltaTime);
	}
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// ����ƶ��¼�
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

	camera.processMouseMovement(xoffset, yoffset);
}

// �������¼�
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.processMouseScroll(yoffset);
}