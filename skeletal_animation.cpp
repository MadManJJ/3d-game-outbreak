#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model_animation.h>
#include <learnopengl/animator.h>
#include <learnopengl/animation.h>
#include <C:\dev\github\LearnOpenGL\src\8.guest\2020\skeletal_animation\learnopengl\model_static.h>  

#include <iostream>
#include <vector>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Animator& animator, Animation& idle, Animation& sprint_forward, Animation& sprint_backward, Animation& strafe_left, Animation& strafe_right, Animation& sprint_forward_left, Animation& sprint_forward_right, Animation& sprint_backward_left, Animation& sprint_backward_right, Animation& firing, Animation& walking_firing, Animation& stepping_back, Animation& crouch_idle);

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
float characterHealth = 100.0f;

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

enum class ZombieState { Idle, Walking, Attacking, Dead };
enum class EnemyType { Zombie, Mutant };

struct Enemy {
    EnemyType Type;
    float Health;
    float MaxHealth;
    float Damage;
    float Speed;
    glm::vec3 Position;
    float Rotation;
    bool Dead;
    float DeathTimer;
    ZombieState State;
    float AttackCooldown;
    Animator animator;

    Enemy(EnemyType type, glm::vec3 pos, float health, float damage, float speed, Animation* idleAnim)
        : Type(type), Position(pos), Health(health), MaxHealth(health), Damage(damage), Speed(speed), 
          Rotation(1.57f), Dead(false), DeathTimer(0.0f), State(ZombieState::Idle), AttackCooldown(0.0f),
          animator(idleAnim) {}
};

std::vector<Enemy> enemies;
int currentWave = 0;
int currentScore = 0;
bool gameWon = false;

const int digitPatterns[10][5][3] = {
    {{1,1,1}, {1,0,1}, {1,0,1}, {1,0,1}, {1,1,1}}, // 0
    {{0,1,0}, {1,1,0}, {0,1,0}, {0,1,0}, {1,1,1}}, // 1
    {{1,1,1}, {0,0,1}, {1,1,1}, {1,0,0}, {1,1,1}}, // 2
    {{1,1,1}, {0,0,1}, {1,1,1}, {0,0,1}, {1,1,1}}, // 3
    {{1,0,1}, {1,0,1}, {1,1,1}, {0,0,1}, {0,0,1}}, // 4
    {{1,1,1}, {1,0,0}, {1,1,1}, {0,0,1}, {1,1,1}}, // 5
    {{1,1,1}, {1,0,0}, {1,1,1}, {1,0,1}, {1,1,1}}, // 6
    {{1,1,1}, {0,0,1}, {0,0,1}, {0,0,1}, {0,0,1}}, // 7
    {{1,1,1}, {1,0,1}, {1,1,1}, {1,0,1}, {1,1,1}}, // 8
    {{1,1,1}, {1,0,1}, {1,1,1}, {0,0,1}, {1,1,1}}, // 9
};

const int letterPatterns[26][5][3] = {
    {{1,1,1},{1,0,1},{1,1,1},{1,0,1},{1,0,1}}, // A
    {{1,1,0},{1,0,1},{1,1,0},{1,0,1},{1,1,0}}, // B
    {{1,1,1},{1,0,0},{1,0,0},{1,0,0},{1,1,1}}, // C
    {{1,1,0},{1,0,1},{1,0,1},{1,0,1},{1,1,0}}, // D
    {{1,1,1},{1,0,0},{1,1,1},{1,0,0},{1,1,1}}, // E
    {{1,1,1},{1,0,0},{1,1,1},{1,0,0},{1,0,0}}, // F
    {{1,1,1},{1,0,0},{1,0,1},{1,0,1},{1,1,1}}, // G
    {{1,0,1},{1,0,1},{1,1,1},{1,0,1},{1,0,1}}, // H
    {{1,1,1},{0,1,0},{0,1,0},{0,1,0},{1,1,1}}, // I
    {{0,0,1},{0,0,1},{0,0,1},{1,0,1},{1,1,1}}, // J
    {{1,0,1},{1,1,0},{1,0,0},{1,1,0},{1,0,1}}, // K
    {{1,0,0},{1,0,0},{1,0,0},{1,0,0},{1,1,1}}, // L
    {{1,1,1},{1,1,1},{1,0,1},{1,0,1},{1,0,1}}, // M
    {{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,0,1}}, // N
    {{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1}}, // O
    {{1,1,1},{1,0,1},{1,1,1},{1,0,0},{1,0,0}}, // P
    {{1,1,1},{1,0,1},{1,0,1},{1,1,1},{0,0,1}}, // Q
    {{1,1,1},{1,0,1},{1,1,0},{1,0,1},{1,0,1}}, // R
    {{1,1,1},{1,0,0},{1,1,1},{0,0,1},{1,1,1}}, // S
    {{1,1,1},{0,1,0},{0,1,0},{0,1,0},{0,1,0}}, // T
    {{1,0,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1}}, // U
    {{1,0,1},{1,0,1},{1,0,1},{1,0,1},{0,1,0}}, // V
    {{1,0,1},{1,0,1},{1,0,1},{1,1,1},{1,1,1}}, // W
    {{1,0,1},{1,0,1},{1,0,1},{0,1,0},{0,1,0}}, // X
    {{1,0,1},{1,0,1},{0,1,0},{0,1,0},{0,1,0}}, // Y
    {{1,1,1},{0,0,1},{0,1,0},{1,0,0},{1,1,1}}  // Z
};

void drawDigit(int digit, float x, float y, float scale, Shader& shader, unsigned int vao) {
    for (int row=0; row<5; ++row) {
        for (int col=0; col<3; ++col) {
            if (digitPatterns[digit][row][col]) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x + col*scale, y - row*scale, 0.0f));
                model = glm::scale(model, glm::vec3(scale, scale, 0.1f));
                shader.setMat4("model", model);
                glBindVertexArray(vao);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }
}

void drawString(const std::string& text, float x, float y, float scale, Shader& shader, unsigned int vao) {
    for (char c : text) {
        if (c == ' ') {
            x += 4.0f * scale; // Spacing for space character
            continue;
        }
        if (c >= '0' && c <= '9') {
            drawDigit(c - '0', x, y, scale, shader, vao);
        } else if (c >= 'A' && c <= 'Z') {
            int letterIdx = c - 'A';
            for (int row = 0; row < 5; ++row) {
                for (int col = 0; col < 3; ++col) {
                    if (letterPatterns[letterIdx][row][col]) {
                        glm::mat4 model = glm::mat4(1.0f);
                        model = glm::translate(model, glm::vec3(x + col * scale, y - row * scale, 0.0f));
                        model = glm::scale(model, glm::vec3(scale, scale, 0.1f));
                        shader.setMat4("model", model);
                        glBindVertexArray(vao);
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                }
            }
        }
        x += 4.0f * scale; // advance cursor
    }
}

void drawScore(int score, float startX, float startY, float scale, Shader& shader, unsigned int vao) {
    shader.setVec3("solidColor", glm::vec3(1.0f, 0.8f, 0.0f)); // Gold color for score
    std::string s = std::to_string(score);
    float x = startX - s.length() * (4.0f * scale); // Right align based on string length
    drawString(s, x, startY, scale, shader, vao);
}

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
    Shader staticShader("1.model_loading.vs", "1.model_loading.fs");

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
    ModelStatic apartmentModel(FileSystem::getPath("resources/objects/apartment/model.obj"));

    const glm::vec3 apartmentPosition(0.0f, 0.0f, 0.0f);
    const glm::vec3 apartmentScale(1.0f, 1.0f, 1.0f);

	ModelStatic gunModel(FileSystem::getPath("resources/objects/player/gun/gun.obj"));

	Model playerModel(FileSystem::getPath("resources/objects/player/player.dae"));
    Animation idle_animation_player(FileSystem::getPath("resources/objects/player/Idle.dae"), &playerModel);
    Animation firing_animation_player(FileSystem::getPath("resources/objects/player/Firing Rifle.dae"), &playerModel);
    Animation walking_firing_animation_player(FileSystem::getPath("resources/objects/player/Walking Firing Rifle.dae"), &playerModel);
    Animation sprint_forward_animation_player(FileSystem::getPath("resources/objects/player/Sprint Forward.dae"), &playerModel);
    Animation sprint_backward_animation_player(FileSystem::getPath("resources/objects/player/Sprint Backward.dae"), &playerModel);
    Animation strafe_left_animation_player(FileSystem::getPath("resources/objects/player/Strafe Left.dae"), &playerModel);
    Animation strafe_right_animation_player(FileSystem::getPath("resources/objects/player/Strafe Right.dae"), &playerModel);
    Animation sprint_forward_left_animation_player(FileSystem::getPath("resources/objects/player/Sprint Forward Left.dae"), &playerModel);
    Animation sprint_forward_right_animation_player(FileSystem::getPath("resources/objects/player/Sprint Forward Right.dae"), &playerModel);
    Animation sprint_backward_left_animation_player(FileSystem::getPath("resources/objects/player/Sprint Backward Left.dae"), &playerModel);
    Animation sprint_backward_right_animation_player(FileSystem::getPath("resources/objects/player/Sprint Backward Right.dae"), &playerModel);
    Animation stepping_back_animation_player(FileSystem::getPath("resources/objects/player/Stepping Backward.dae"), &playerModel);
    Animation crouch_idle_animation_player(FileSystem::getPath("resources/objects/player/Crouch Idle.dae"), &playerModel);	
    Animator playerAnimator(&idle_animation_player);

	Model zombieGirlModel(FileSystem::getPath("resources/objects/zombie/zombie girl.dae"));
	Animation zombie_idle_animation(FileSystem::getPath("resources/objects/zombie/zombie idle.dae"), &zombieGirlModel);
	Animation zombie_walk_animation(FileSystem::getPath("resources/objects/zombie/zombie walk.dae"), &zombieGirlModel);
	Animation zombie_death_animation(FileSystem::getPath("resources/objects/zombie/zombie death.dae"), &zombieGirlModel);
	Animation zombie_attack_animation(FileSystem::getPath("resources/objects/zombie/zombie attack.dae"), &zombieGirlModel);

    Model mutantModel(FileSystem::getPath("resources/objects/mutant/mutant.dae"));
    Animation mutant_idle_animation(FileSystem::getPath("resources/objects/mutant/mutant idle.dae"), &mutantModel);
	Animation mutant_walk_animation(FileSystem::getPath("resources/objects/mutant/mutant run.dae"), &mutantModel);
	Animation mutant_death_animation(FileSystem::getPath("resources/objects/mutant/mutant dying.dae"), &mutantModel);
	Animation mutant_attack_animation(FileSystem::getPath("resources/objects/mutant/mutant swiping.dae"), &mutantModel);

    auto spawnWave1 = [&](){
        enemies.clear();
        enemies.emplace_back(EnemyType::Zombie, glm::vec3(-3.0f, -0.4f, 0.0f), 100.0f, 10.0f, 0.5f, &zombie_idle_animation);
        enemies.emplace_back(EnemyType::Zombie, glm::vec3(3.0f, -0.4f, -2.0f), 100.0f, 10.0f, 0.5f, &zombie_idle_animation);
        enemies.emplace_back(EnemyType::Zombie, glm::vec3(1.0f, -0.4f, 3.0f), 100.0f, 10.0f, 0.5f, &zombie_idle_animation);
        currentWave = 1;
    };

    auto spawnWave2 = [&](){
        enemies.clear();
        enemies.emplace_back(EnemyType::Mutant, glm::vec3(0.0f, -0.4f, -6.0f), 300.0f, 25.0f, 1.35f, &mutant_idle_animation);
        currentWave = 2;
    };

    auto spawnWave3 = [&]() {
        enemies.clear();
        enemies.emplace_back(EnemyType::Mutant, glm::vec3(0.0f, -0.4f, -6.0f), 300.0f, 25.0f, 1.35f, &mutant_idle_animation);
        enemies.emplace_back(EnemyType::Zombie, glm::vec3(-3.0f, -0.4f, 0.0f), 100.0f, 10.0f, 0.5f, &zombie_idle_animation);
        enemies.emplace_back(EnemyType::Zombie, glm::vec3(3.0f, -0.4f, -2.0f), 100.0f, 10.0f, 0.5f, &zombie_idle_animation);
        enemies.emplace_back(EnemyType::Zombie, glm::vec3(1.0f, -0.4f, 3.0f), 100.0f, 10.0f, 0.5f, &zombie_idle_animation);
        currentWave = 3;
    };

    spawnWave1();

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
		processInput(window, playerAnimator, idle_animation_player, sprint_forward_animation_player, sprint_backward_animation_player, strafe_left_animation_player, strafe_right_animation_player, sprint_forward_left_animation_player, sprint_forward_right_animation_player, sprint_backward_left_animation_player, sprint_backward_right_animation_player, firing_animation_player, walking_firing_animation_player, stepping_back_animation_player, crouch_idle_animation_player);
		playerAnimator.UpdateAnimation(deltaTime);
        
        bool allDead = true;
        for (auto& enemy : enemies) {
		    enemy.animator.UpdateAnimation(deltaTime);
            if (!enemy.Dead || enemy.DeathTimer > 0.0f) {
                allDead = false;
            }
        }

        if (allDead && enemies.size() > 0) {
            if (currentWave == 1) {
                spawnWave2();
            } else if (currentWave == 2) {
                spawnWave3();
            }
            else if (currentWave == 3) {
                gameWon = true;
            }
        }

		// Update cooldown
		if (fireRateCooldown > 0.0f) {
			fireRateCooldown -= deltaTime;
		}

		// Update Enemy AI
		if (characterHealth > 0.0f && !gameWon) {
            for (auto& enemy : enemies) {
                if (enemy.Dead) {
                    if (enemy.DeathTimer > 0.0f) {
                        enemy.DeathTimer -= deltaTime;
                    }
                    continue;
                }

			    glm::vec3 diff = characterPosition - enemy.Position;
			    diff.y = 0.0f; // ignore vertical difference for distance
			    float dist = glm::length(diff);
                
                if (enemy.AttackCooldown > 0.0f) {
                    enemy.AttackCooldown -= deltaTime;
                    if (enemy.AttackCooldown <= 0.0f && enemy.State == ZombieState::Attacking) {
                        if (dist < 2.0f) { // If still in range after finishing the attack swing
                            characterHealth -= enemy.Damage;
                            if (characterHealth <= 0.0f) {
                                characterHealth = 0.0f;
                                std::cout << "\n============================================\n";
                                std::cout << "GAME OVER! Do you want to retry or try again?\n";
                                std::cout << "Press SPACE to try again.\n";
                                std::cout << "============================================\n\n";
                            } else {
                                std::cout << "Character hit! Health: " << characterHealth << "\n";
                            }
                        }
                    }
                }

                bool lockedInAttack = (enemy.State == ZombieState::Attacking && enemy.AttackCooldown > 0.0f);

                if (!lockedInAttack) {
			        if (dist < 1.5f) {
			        	if (enemy.State != ZombieState::Attacking) {
			        		enemy.State = ZombieState::Attacking;
			        		enemy.animator.PlayAnimation(enemy.Type == EnemyType::Zombie ? &zombie_attack_animation : &mutant_attack_animation);
			        	}
                        // Face character
                        enemy.Rotation = atan2(diff.x, diff.z);
			        	enemy.AttackCooldown = 1.6f; // Lock animation length and attack timing
			        } else if (dist < 10.0f) {
			        	if (enemy.State != ZombieState::Walking) {
			        		enemy.State = ZombieState::Walking;
			        		enemy.animator.PlayAnimation(enemy.Type == EnemyType::Zombie ? &zombie_walk_animation : &mutant_walk_animation);
			        	}
			        	// Move towards character
			        	glm::vec3 dir = diff / dist;
			        	enemy.Position += dir * enemy.Speed * deltaTime;
                        enemy.Rotation = atan2(dir.x, dir.z);
			        } else {
			        	if (enemy.State != ZombieState::Idle) {
			        		enemy.State = ZombieState::Idle;
			        		enemy.animator.PlayAnimation(enemy.Type == EnemyType::Zombie ? &zombie_idle_animation : &mutant_idle_animation);
			        	}
			        }
                }
            }
		} else {
            // Game Over or Win Logic
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                characterHealth = 100.0f;
                characterPosition = glm::vec3(0.0f, -0.4f, 0.0f);
                characterYaw = -90.0f;
                currentScore = 0;
                gameWon = false;
                bullets.clear();
                spawnWave1();
                std::cout << "Game Restarted!\n";
            }
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

			    // ENEMY COLLISION DETECTION
            for (auto& enemy : enemies) {
                if (enemy.Dead) continue;
			    float hitRadius = (enemy.Type == EnemyType::Mutant) ? 1.5f : 1.0f; // Approximate width
			    if (glm::distance(bullet.Position, enemy.Position + glm::vec3(0.0f, 1.0f, 0.0f)) < hitRadius) {
			    	bullet.Active = false;
			    	enemy.Health -= 10.0f;
			    	if (enemy.Health <= 0.0f) {
			    		std::cout << "Enemy Died!\n";
                        currentScore += (enemy.Type == EnemyType::Mutant) ? 50 : 20; // Update score
                        enemy.Dead = true;
                        enemy.State = ZombieState::Dead;
                        enemy.DeathTimer = 2.85f; // Wait for death animation to finish (~3.5 seconds)
                        enemy.animator.PlayAnimation(enemy.Type == EnemyType::Zombie ? &zombie_death_animation : &mutant_death_animation);
			    	}
                    break;
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

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(camera.Position, lookTarget, glm::vec3(0.0f, 1.0f, 0.0f));

        // render the apartment model
        staticShader.use();
        staticShader.setMat4("projection", projection);
        staticShader.setMat4("view", view);

        glm::mat4 apartmentWorld = glm::mat4(1.0f);
        apartmentWorld = glm::translate(apartmentWorld, apartmentPosition);
        apartmentWorld = glm::scale(apartmentWorld, apartmentScale);
        staticShader.setMat4("model", apartmentWorld);
        apartmentModel.Draw(staticShader);

        // don't forget to enable shader before setting uniforms
		ourShader.use();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

        auto transforms = playerAnimator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, characterPosition);
        model = glm::rotate(model, glm::radians(-characterYaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate based on character Yaw
		model = glm::scale(model, glm::vec3(50.0f, 50.0f, 50.0f));	// Scale Player appropriately
		ourShader.setMat4("model", model);
        ourShader.setBool("useSolidColor", false);
		playerModel.Draw(ourShader);

        // draw the gun attached to the right hand
        glm::mat4 rightHandTransform = playerAnimator.GetGlobalTransform("mixamorig_RightHand");
        glm::mat4 gunModelMatrix = model * rightHandTransform;
        
        // Adjust the gun relative to the bone
        // You might need to tweak these scale/rotate/translate values to make it fit perfectly
        // Apply translations and rotations BEFORE scaling to accurately offset it from the hand
        gunModelMatrix = glm::translate(gunModelMatrix, glm::vec3(0.0f, 0.1f, 0.0f)); // Move it into the palm
        gunModelMatrix = glm::rotate(gunModelMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, -0.001f)); // Pitch to aim forward
        gunModelMatrix = glm::rotate(gunModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw to align grip
        gunModelMatrix = glm::rotate(gunModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Roll to align grip
        gunModelMatrix = glm::scale(gunModelMatrix, glm::vec3(0.002f, 0.002f, 0.002f)); // Scale it way down!

        staticShader.use();
        staticShader.setMat4("model", gunModelMatrix);
        gunModel.Draw(staticShader);

        // bind zombie shader and set up its matrices
        zombieShader.use();
        zombieShader.setMat4("projection", projection);
        zombieShader.setMat4("view", view);

        for (auto& enemy : enemies) {
            if (enemy.Dead && enemy.DeathTimer <= 0.0f) continue;

            auto transforms = enemy.animator.GetFinalBoneMatrices();
            for (int i = 0; i < transforms.size(); ++i)
                zombieShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

            model = glm::mat4(1.0f);
		    model = glm::translate(model, enemy.Position);
            model = glm::rotate(model, enemy.Rotation, glm::vec3(0.0f, 1.0f, 0.0f));
            float scale = (enemy.Type == EnemyType::Mutant) ? 0.7f : 0.5f;
            model = glm::scale(model, glm::vec3(scale, scale, scale));
		    zombieShader.setMat4("model", model);
            zombieShader.setBool("useSolidColor", false);
            
            if (enemy.Type == EnemyType::Zombie) {
		        zombieGirlModel.Draw(zombieShader);
            } else {
                mutantModel.Draw(zombieShader);
            }
        }
        
        ourShader.use(); // Switch back to ourShader
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind inherited model textures so bullets look normal
        ourShader.setBool("useSolidColor", true);
        
        // Render Enemy Health Bars
        for (auto& enemy : enemies) {
            if (enemy.Health > 0.0f) {
                float healthRatio = enemy.Health / enemy.MaxHealth;
                
                glm::mat4 hpModel = glm::mat4(1.0f);
                float heightOffset = (enemy.Type == EnemyType::Mutant) ? 2.5f : 1.8f;
                hpModel = glm::translate(hpModel, enemy.Position + glm::vec3(0.0f, heightOffset, 0.0f));
                
                // Face camera precisely
                glm::vec3 dirToCam = glm::normalize(camera.Position - (enemy.Position + glm::vec3(0.0f, heightOffset, 0.0f)));
                float angle = atan2(dirToCam.x, dirToCam.z);
                hpModel = glm::rotate(hpModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                
                // Background (grayish white)
                glm::mat4 bgModel = glm::scale(hpModel, glm::vec3(1.0f, 0.1f, 0.02f));
                ourShader.setMat4("model", bgModel);
                ourShader.setVec3("solidColor", glm::vec3(0.8f, 0.8f, 0.8f));
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                // Foreground (red)
                if (healthRatio > 0.0f) {
                    glm::mat4 fgModel = glm::translate(hpModel, glm::vec3(-0.5f * (1.0f - healthRatio), 0.0f, 0.025f)); 
                    fgModel = glm::scale(fgModel, glm::vec3(healthRatio * 0.98f, 0.08f, 0.025f));
                    ourShader.setMat4("model", fgModel);
                    ourShader.setVec3("solidColor", glm::vec3(1.0f, 0.0f, 0.0f));
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
        }
        
        // Render Character Health Bar (2D Screen Space UI)
        if (characterHealth > 0.0f && !gameWon) {
            float healthRatio = characterHealth / 100.0f;
            
            glDisable(GL_DEPTH_TEST); // Ensure UI draws over everything
            
            // Switch to orthographic projection for UI
            glm::mat4 orthoProj = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
            ourShader.setMat4("projection", orthoProj);
            ourShader.setMat4("view", glm::mat4(1.0f));
            
            float barWidth = 400.0f;
            float barHeight = 20.0f;
            float xPos = SCR_WIDTH / 2.0f;
            float yPos = SCR_HEIGHT - 30.0f;

            glm::mat4 charHpModel = glm::mat4(1.0f);
            charHpModel = glm::translate(charHpModel, glm::vec3(xPos, yPos, 0.0f));
            
            // Background (grayish white)
            glm::mat4 bgModel = glm::scale(charHpModel, glm::vec3(barWidth, barHeight, 0.1f));
            ourShader.setMat4("model", bgModel);
            ourShader.setVec3("solidColor", glm::vec3(0.8f, 0.8f, 0.8f));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // Foreground (green)
            if (healthRatio > 0.0f) {
                float fgWidth = barWidth * healthRatio;
                // Shift to align left edge
                glm::mat4 fgModel = glm::translate(charHpModel, glm::vec3(-0.5f * (barWidth - fgWidth), 0.0f, 0.0f)); 
                fgModel = glm::scale(fgModel, glm::vec3(fgWidth, barHeight, 0.1f));
                ourShader.setMat4("model", fgModel);
                ourShader.setVec3("solidColor", glm::vec3(0.0f, 0.8f, 0.2f)); // Greenish health
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            
            glEnable(GL_DEPTH_TEST);
            
            // Restore perspective projection for further 3D rendering
            ourShader.setMat4("projection", projection);
            ourShader.setMat4("view", view);
        } else if (characterHealth <= 0.0f) {
            // Draw big red overlay to symbolise Game Over
            glDisable(GL_DEPTH_TEST);
            glm::mat4 orthoProj = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
            ourShader.setMat4("projection", orthoProj);
            ourShader.setMat4("view", glm::mat4(1.0f));
            
            glm::mat4 bgModel = glm::mat4(1.0f);
            bgModel = glm::translate(bgModel, glm::vec3(SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f, 0.0f));
            bgModel = glm::scale(bgModel, glm::vec3(SCR_WIDTH, SCR_HEIGHT, 0.1f));
            ourShader.setMat4("model", bgModel);
            ourShader.setVec3("solidColor", glm::vec3(0.4f, 0.0f, 0.0f)); // Dark red
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            // Draw text overlay
            ourShader.setVec3("solidColor", glm::vec3(1.0f, 1.0f, 1.0f)); // White text
            float goScale = 8.0f;
            float goWidth = 9.0f * (4.0f * goScale); // "GAME OVER" length
            drawString("GAME OVER", (SCR_WIDTH - goWidth) / 2.0f, SCR_HEIGHT / 2.0f + 50.0f, goScale, ourShader, cubeVAO);
            
            ourShader.setVec3("solidColor", glm::vec3(0.8f, 0.8f, 0.8f)); // Grey sub-text
            float psScale = 4.0f;
            float psWidth = 11.0f * (4.0f * psScale); // "PRESS SPACE" length
            drawString("PRESS SPACE", (SCR_WIDTH - psWidth) / 2.0f, SCR_HEIGHT / 2.0f - 50.0f, psScale, ourShader, cubeVAO);

            glEnable(GL_DEPTH_TEST);
            ourShader.setMat4("projection", projection);
            ourShader.setMat4("view", view);
        } else if (gameWon) {
            // Draw big green overlay to symbolise You Win
            glDisable(GL_DEPTH_TEST);
            glm::mat4 orthoProj = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
            ourShader.setMat4("projection", orthoProj);
            ourShader.setMat4("view", glm::mat4(1.0f));
            
            glm::mat4 bgModel = glm::mat4(1.0f);
            bgModel = glm::translate(bgModel, glm::vec3(SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f, 0.0f));
            bgModel = glm::scale(bgModel, glm::vec3(SCR_WIDTH, SCR_HEIGHT, 0.1f));
            ourShader.setMat4("model", bgModel);
            ourShader.setVec3("solidColor", glm::vec3(0.0f, 0.4f, 0.0f)); // Dark green
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            // Draw text overlay
            ourShader.setVec3("solidColor", glm::vec3(1.0f, 1.0f, 1.0f)); // White text
            float ywScale = 8.0f;
            float ywWidth = 7.0f * (4.0f * ywScale); // "YOU WIN" length
            drawString("YOU WIN", (SCR_WIDTH - ywWidth) / 2.0f, SCR_HEIGHT / 2.0f + 50.0f, ywScale, ourShader, cubeVAO);
            
            ourShader.setVec3("solidColor", glm::vec3(0.8f, 0.8f, 0.8f)); // Grey sub-text
            float psScale = 4.0f;
            float psWidth = 11.0f * (4.0f * psScale); // "PRESS SPACE" length
            drawString("PRESS SPACE", (SCR_WIDTH - psWidth) / 2.0f, SCR_HEIGHT / 2.0f - 50.0f, psScale, ourShader, cubeVAO);

            glEnable(GL_DEPTH_TEST);
            ourShader.setMat4("projection", projection);
            ourShader.setMat4("view", view);
        }

        // Render Score Geometry (Top Right)
        glDisable(GL_DEPTH_TEST);
        glm::mat4 orthoProjTitle = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
        ourShader.setMat4("projection", orthoProjTitle);
        ourShader.setMat4("view", glm::mat4(1.0f));
        drawScore(currentScore, SCR_WIDTH - 20.0f, SCR_HEIGHT - 20.0f, 6.0f, ourShader, cubeVAO);
        glEnable(GL_DEPTH_TEST);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // set bright glowing yellow/orange color for bullets
        ourShader.setVec3("solidColor", glm::vec3(1.0f, 0.8f, 0.2f)); 
        glBindVertexArray(cubeVAO);

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
            bulletModel = glm::scale(bulletModel, glm::vec3(0.01f, 0.015f, 0.5f)); 
            
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
void processInput(GLFWwindow* window, Animator& animator, Animation& idle, Animation& sprint_forward, Animation& sprint_backward, Animation& strafe_left, Animation& strafe_right, Animation& sprint_forward_left, Animation& sprint_forward_right, Animation& sprint_backward_left, Animation& sprint_backward_right, Animation& firing, Animation& walking_firing, Animation& stepping_back, Animation& crouch_idle)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

    if (characterHealth <= 0.0f || gameWon)
        return;

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
    bool ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

    // Evaluate 8-way movement
    if (ctrl) {
        targetAnimation = &crouch_idle;
    } else if (w && a) {
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
        if (ctrl) targetAnimation = &crouch_idle;
        else if (w) targetAnimation = &walking_firing;
        else if (s) targetAnimation = &stepping_back;
        else if (!w && !a && !s && !d) targetAnimation = &firing;
        
        // Firing mechanism
        if (fireRateCooldown <= 0.0f) {
            Bullet newBullet;
            // Spawn bullet roughly at gun height
            newBullet.Position = characterPosition + glm::vec3(0.0f, 0.2f, 0.0f) + forward * 0.5f; 
            newBullet.Velocity = forward * 25.0f; // Bullet speed
            newBullet.Lifetime = 0.8f;
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

    // Make the character move slower when walking/stepping backward while firing
    if ((w || s) && mouseLeft) {
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
