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
#include <vector>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Animator& animator, Animation& idle, Animation& sprint_forward, Animation& sprint_backward, Animation& strafe_left, Animation& strafe_right, Animation& sprint_forward_left, Animation& sprint_forward_right, Animation& sprint_backward_left, Animation& sprint_backward_right, Animation& firing, Animation& walking_firing);

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

// bullets and shooting
struct Bullet {
    glm::vec3 Position;
    glm::vec3 Velocity;
    float Lifetime;
    bool Active;
};
std::vector<Bullet> bullets;
float fireRateCooldown = 0.0f;
const float FIRE_COOLDOWN = 0.1f;

// zombie state
float zombieHealth = 100.0f;
glm::vec3 zombiePos(-3.0f, -0.4f, 0.0f);

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
	Shader zombieShader("anim_model.vs", "anim_model.fs");

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
	Model ourModel(FileSystem::getPath("resources/objects/swat/swat.dae"));
	Animation idle_animation(FileSystem::getPath("resources/objects/swat/Rifle Aiming Idle.dae"), &ourModel);
	Animation firing_animation(FileSystem::getPath("resources/objects/swat/Firing Rifle.dae"), &ourModel);
	Animation walking_firing_animation(FileSystem::getPath("resources/objects/swat/Firing Rifle While Walking Forward.dae"), &ourModel);
	Animation sprint_forward_animation(FileSystem::getPath("resources/objects/swat/Sprint Forward.dae"), &ourModel);
	Animation sprint_backward_animation(FileSystem::getPath("resources/objects/swat/Sprint Backward.dae"), &ourModel);
	Animation strafe_left_animation(FileSystem::getPath("resources/objects/swat/Strafe Left.dae"), &ourModel);
	Animation strafe_right_animation(FileSystem::getPath("resources/objects/swat/Strafe Right.dae"), &ourModel);
	Animation sprint_forward_left_animation(FileSystem::getPath("resources/objects/swat/Run Forward Left.dae"), &ourModel);
	Animation sprint_forward_right_animation(FileSystem::getPath("resources/objects/swat/Run Forward Right.dae"), &ourModel);
	Animation sprint_backward_left_animation(FileSystem::getPath("resources/objects/swat/Run Backward Left.dae"), &ourModel);
	Animation sprint_backward_right_animation(FileSystem::getPath("resources/objects/swat/Run Backward Right.dae"), &ourModel);
	Animator animator(&idle_animation);

	Model zombieGirlModel(FileSystem::getPath("resources/objects/zombie/zombie girl.dae"));
	Animation zombie_idle_animation(FileSystem::getPath("resources/objects/zombie/zombie idle.dae"), &zombieGirlModel);
	Animator zombieAnimator(&zombie_idle_animation);

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
		processInput(window, animator, idle_animation, sprint_forward_animation, sprint_backward_animation, strafe_left_animation, strafe_right_animation, sprint_forward_left_animation, sprint_forward_right_animation, sprint_backward_left_animation, sprint_backward_right_animation, firing_animation, walking_firing_animation);
		animator.UpdateAnimation(deltaTime);
		zombieAnimator.UpdateAnimation(deltaTime);

		// Update cooldown
		if (fireRateCooldown > 0.0f) {
			fireRateCooldown -= deltaTime;
		}

		// Update bullets
		for (auto& bullet : bullets) {
			if (!bullet.Active) continue;
			
			bullet.Position += bullet.Velocity * deltaTime;
			bullet.Lifetime -= deltaTime;
			
			if (bullet.Lifetime <= 0.0f) {
				bullet.Active = false;
				continue;
			}

			// ZOMBIE COLLISION DETECTION
			float hitRadius = 1.0f; // Approximate zombie width
			if (zombieHealth > 0.0f && glm::distance(bullet.Position, zombiePos + glm::vec3(0.0f, 1.0f, 0.0f)) < hitRadius) {
				bullet.Active = false;
				zombieHealth -= 10.0f;
				if (zombieHealth <= 0.0f) {
					std::cout << "Zombie Died!\n";
				}
			}
		}
		
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
        ourShader.setBool("useSolidColor", false);
		ourModel.Draw(ourShader);

        // bind zombie shader and set up its matrices
        zombieShader.use();
        zombieShader.setMat4("projection", projection);
        zombieShader.setMat4("view", view);
        transforms = zombieAnimator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            zombieShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        model = glm::mat4(1.0f);
		model = glm::translate(model, zombiePos); // Fixed position to move around
        model = glm::scale(model, glm::vec3(.5f, .5f, .5f));
		zombieShader.setMat4("model", model);
        zombieShader.setBool("useSolidColor", false);
        if (zombieHealth > 0.0f) {
		    zombieGirlModel.Draw(zombieShader);
        }
        
        ourShader.use(); // Switch back to ourShader
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind inherited model textures so bullets look normal
        ourShader.setBool("useSolidColor", true);
        ourShader.setVec3("solidColor", glm::vec3(0.5f, 0.5f, 0.5f)); // Grey for reference block
        
        // render cube reference
        glm::mat4 cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(3.0f, 0.0f, 0.0f)); // Fixed position to move around
        cubeModel = glm::scale(cubeModel, glm::vec3(1.0f));
        ourShader.setMat4("model", cubeModel);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // set bright glowing yellow/orange color for bullets
        ourShader.setVec3("solidColor", glm::vec3(1.0f, 0.8f, 0.2f)); 

        // render bullets
        for (const auto& bullet : bullets) {
            if (!bullet.Active) continue;
            
            glm::mat4 bulletModel = glm::mat4(1.0f);
            bulletModel = glm::translate(bulletModel, bullet.Position);
            
            // Align bullet with its velocity vector (Tracer effect)
            glm::vec3 dir = glm::normalize(bullet.Velocity);
            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), dir));
            glm::vec3 up = glm::cross(dir, right);
            
            glm::mat4 rot(1.0f);
            rot[0] = glm::vec4(right, 0.0f);
            rot[1] = glm::vec4(up, 0.0f);
            rot[2] = glm::vec4(dir, 0.0f);
            
            bulletModel *= rot;
            
            // Scale to look like a long tracer (thin in width/height, long in travel direction)
            bulletModel = glm::scale(bulletModel, glm::vec3(0.02f, 0.02f, 1.0f)); 
            
            ourShader.setMat4("model", bulletModel);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


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
void processInput(GLFWwindow* window, Animator& animator, Animation& idle, Animation& sprint_forward, Animation& sprint_backward, Animation& strafe_left, Animation& strafe_right, Animation& sprint_forward_left, Animation& sprint_forward_right, Animation& sprint_backward_left, Animation& sprint_backward_right, Animation& firing, Animation& walking_firing)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

    float velocity = characterSpeed * deltaTime;
    
    // Calculate forward and right vectors based on character's facing direction
    float yawRadians = glm::radians(characterYaw);
    glm::vec3 forward(cos(yawRadians), 0.0f, sin(yawRadians));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 moveInput(0.0f);

    // State tracking to prevent restarting animation every frame
    static Animation* currentAnimation = &idle;
    Animation* targetAnimation = &idle; // Default to idle if no keys are pressed

    bool w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    bool s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    bool a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    bool d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    bool mouseLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // Evaluate 8-way movement
    if (w && a) {
        moveInput += forward - right;
        targetAnimation = &sprint_forward_left;
    } else if (w && d) {
        moveInput += forward + right;
        targetAnimation = &sprint_forward_right;
    } else if (s && a) {
        moveInput -= forward + right;
        targetAnimation = &sprint_backward_left;
    } else if (s && d) {
        moveInput -= forward - right;
        targetAnimation = &sprint_backward_right;
    } else if (w) {
        moveInput += forward;
        targetAnimation = &sprint_forward;
    } else if (s) {
        moveInput -= forward;
        targetAnimation = &sprint_backward;
    } else if (a) {
        moveInput -= right;
        targetAnimation = &strafe_left;
    } else if (d) {
        moveInput += right;
        targetAnimation = &strafe_right;
    }

    // Override animation if firing
    if (mouseLeft) {
        if (w) targetAnimation = &walking_firing;
        else if (!w && !a && !s && !d) targetAnimation = &firing;
        
        // Firing mechanism
        if (fireRateCooldown <= 0.0f) {
            Bullet newBullet;
            // Spawn bullet roughly at gun height
            newBullet.Position = characterPosition + glm::vec3(0.0f, 0.2f, 0.0f) + forward * 0.5f; 
            newBullet.Velocity = forward * 25.0f; // Bullet speed
            newBullet.Lifetime = 2.0f;
            newBullet.Active = true;
            
            // Reuse an inactive bullet or add a new one
            bool reused = false;
            for (auto& b : bullets) {
                if (!b.Active) {
                    b = newBullet;
                    reused = true;
                    break;
                }
            }
            if (!reused) {
                bullets.push_back(newBullet);
            }
            
            fireRateCooldown = FIRE_COOLDOWN;
        }
    }

    if (glm::dot(moveInput, moveInput) > 0.0f)
        moveInput = glm::normalize(moveInput);

    // Make the character move slower when walking forward while firing
    if (w && mouseLeft) {
        velocity *= 0.5f; // Reduce speed by half
    }

    characterPosition += moveInput * velocity;

    // Only play the new animation if the state actually changed
    if (currentAnimation != targetAnimation) {
        animator.PlayAnimation(targetAnimation);
        currentAnimation = targetAnimation;
    }
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
