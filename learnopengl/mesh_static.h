#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/shader.h>

#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE_STATIC 4

struct VertexStatic {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE_STATIC];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE_STATIC];
};

struct TextureStatic {
    unsigned int id;
    string type;
    string path;
};

class MeshStatic {
public:
    // mesh Data
    vector<VertexStatic>       vertices;
    vector<unsigned int> indices;
    vector<TextureStatic>      textures;
    unsigned int VAO;

    glm::vec3 diffuseColor;
    bool hasTexture;

    // constructor
    MeshStatic(vector<VertexStatic> vertices, vector<unsigned int> indices, vector<TextureStatic> textures, glm::vec3 color, bool hasTex)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->diffuseColor = color;
        this->hasTexture = hasTex;

        setupMeshStatic();
    }

    // render the mesh
    void Draw(Shader& shader)
    {
        // 1. Tell the shader if we are using an image or a .mtl color
        glUniform1i(glGetUniformLocation(shader.ID, "hasTexture"), hasTexture);

        if (hasTexture && textures.size() > 0) {
            // Bind the JPG/PNG
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(shader.ID, "texture_diffuse1"), 0);
            glBindTexture(GL_TEXTURE_2D, textures[0].id);
        }
        else {
            // Send the original .mtl color (like the orange, grey, etc.) to the shader
            glUniform3f(glGetUniformLocation(shader.ID, "meshColor"), diffuseColor.r, diffuseColor.g, diffuseColor.b);
        }

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMeshStatic()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexStatic), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStatic), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStatic), (void*)offsetof(VertexStatic, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexStatic), (void*)offsetof(VertexStatic, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStatic), (void*)offsetof(VertexStatic, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStatic), (void*)offsetof(VertexStatic, Bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(VertexStatic), (void*)offsetof(VertexStatic, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexStatic), (void*)offsetof(VertexStatic, m_Weights));
        glBindVertexArray(0);
    }
};