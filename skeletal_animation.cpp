#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>



#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Animator& animator, Animation& anim1, Animation& anim2, Animation& anim3, Animation& anim4, Animation& anim5);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// character properties
glm::vec3 characterPosition(0.0f, -0.4f, 0.0f);
float characterYaw = -90.0f; // facing direction (in degrees)
float characterSpeed = 2.5f;

// 3rd person camera settings
float cameraDistance = 2.0f;   // distance behind character
float cameraHeight = 1.5f;     // height above character

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("anim_model.vs", "anim_model.fs");

	
    // set up simple cube
    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    // Note: providing generic attribute values for attributes 5,6 to circumvent bone calculations
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glVertexAttribI4i(5, 100, 0, 0, 0); // boneIds default
    glVertexAttrib4f(6, 1.0f, 0.0f, 0.0f, 0.0f); // weights default

	// load models
	// -----------
	Model ourModel(FileSystem::getPath("resources/objects/swat/Ch15_nonPBR.dae"));
	Animation idle_animation(FileSystem::getPath("resources/objects/swat/Rifle Aiming Idle.dae"), &ourModel);
	Animation firing_animation(FileSystem::getPath("resources/objects/swat/Firing Rifle.dae"), &ourModel);
	Animation walking_firing_animation(FileSystem::getPath("resources/objects/swat/Firing Rifle While Walking Forward.dae"), &ourModel);
	Animation sprint_forward_animation(FileSystem::getPath("resources/objects/swat/Sprint Forward.dae"), &ourModel);
	Animation sprint_backward_animation(FileSystem::getPath("resources/objects/swat/Sprint Backward.dae"), &ourModel);
	Animator animator(&idle_animation);

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window, animator, idle_animation, firing_animation, walking_firing_animation, sprint_forward_animation, sprint_backward_animation);
		animator.UpdateAnimation(deltaTime);
		
		// Update camera position to follow character (3rd person)
        float yawRadians = glm::radians(characterYaw);
        glm::vec3 cameraOffset(
            -cos(yawRadians) * cameraDistance, // X offset
            cameraHeight, // Y offset
            -sin(yawRadians) * cameraDistance // Z offset
        );
        camera.Position = characterPosition + cameraOffset;
        
        // Make camera look at the character
        glm::vec3 lookTarget = characterPosition + glm::vec3(0.0f, 1.0f, 0.0f); // look slightly above character center
		
		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = glm::lookAt(camera.Position, lookTarget, glm::vec3(0.0f, 1.0f, 0.0f));
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

        auto transforms = animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);


		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, characterPosition);
        model = glm::rotate(model, glm::radians(-characterYaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate based on character Yaw
		model = glm::scale(model, glm::vec3(.5f, .5f, .5f));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);
        
        // render cube reference
        glm::mat4 cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(3.0f, 0.0f, 0.0f)); // Fixed position to move around
        cubeModel = glm::scale(cubeModel, glm::vec3(1.0f));
        ourShader.setMat4("model", cubeModel);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Animator& animator, Animation& anim1, Animation& anim2, Animation& anim3, Animation& anim4, Animation& anim5)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

    float velocity = characterSpeed * deltaTime;
    
    // Calculate forward and right vectors based on character's facing direction
    float yawRadians = glm::radians(characterYaw);
    glm::vec3 forward(cos(yawRadians), 0.0f, sin(yawRadians));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 moveInput(0.0f);

    // Move character with WASD
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveInput += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveInput -= forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveInput -= right; // A is left, so negative right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveInput += right;

    if (glm::dot(moveInput, moveInput) > 0.0f)
        moveInput = glm::normalize(moveInput);

    characterPosition += moveInput * velocity;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		animator.PlayAnimation(&anim1);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		animator.PlayAnimation(&anim2);
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		animator.PlayAnimation(&anim3);
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		animator.PlayAnimation(&anim4);
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		animator.PlayAnimation(&anim5);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
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
	lastX = xpos;
	lastY = ypos;

    // Rotate character based on mouse X movement
    float sensitivity = 0.1f;
    characterYaw += xoffset * sensitivity;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cameraDistance -= static_cast<float>(yoffset) * 0.5f;
    if (cameraDistance < 1.0f)
        cameraDistance = 1.0f;
    if (cameraDistance > 10.0f)
        cameraDistance = 10.0f;
}
