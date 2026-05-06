#include "Mesh.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace gps {

    
    static GLuint getWhiteTexture() {
        static GLuint whiteTexture = 0;
        if (whiteTexture == 0) {
            unsigned char white[] = { 255, 255, 255, 255 };
            glGenTextures(1, &whiteTexture);
            glBindTexture(GL_TEXTURE_2D, whiteTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        return whiteTexture;
    }

    static std::string normalizeTextureType(const std::string& t) {
        
        if (t == "texture_diffuse")  return "diffuseTexture";
        if (t == "texture_specular") return "specularTexture";
        if (t == "texture_ambient")  return "ambientTexture";
        return t; 
    }

    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures) {
        this->vertices = std::move(vertices);
        this->indices = std::move(indices);
        this->textures = std::move(textures);

        
        this->material.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
        this->material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
        this->material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
        this->material.shininess = 32.0f;

        setupMesh();
    }

    Buffers Mesh::getBuffers() {
        return this->buffers;
    }

    void Mesh::Draw(gps::Shader shader) {
        shader.useShaderProgram();

        
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "material.ambient"), 1, glm::value_ptr(material.ambient));
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 1, glm::value_ptr(material.diffuse));
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "material.specular"), 1, glm::value_ptr(material.specular));
        glUniform1f(glGetUniformLocation(shader.shaderProgram, "material.shininess"), material.shininess);

      
        GLuint diffuseId = 0, specularId = 0, ambientId = 0;

        for (const auto& tx : textures) {
            std::string type = normalizeTextureType(tx.type);

            if (type == "diffuseTexture" && diffuseId == 0) diffuseId = tx.id;
            else if (type == "specularTexture" && specularId == 0) specularId = tx.id;
            else if (type == "ambientTexture" && ambientId == 0) ambientId = tx.id;
        }

        GLuint white = getWhiteTexture();
        if (diffuseId == 0) diffuseId = white; 
        if (specularId == 0) specularId = white;             
        if (ambientId == 0) ambientId = white;               

        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseId);
        glUniform1i(glGetUniformLocation(shader.shaderProgram, "diffuseTexture"), 0);

       
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularId);
        glUniform1i(glGetUniformLocation(shader.shaderProgram, "specularTexture"), 1);

        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ambientId);
        glUniform1i(glGetUniformLocation(shader.shaderProgram, "ambientTexture"), 2);

        
        glBindVertexArray(buffers.VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Mesh::setupMesh() {
        glGenVertexArrays(1, &buffers.VAO);
        glGenBuffers(1, &buffers.VBO);
        glGenBuffers(1, &buffers.EBO);

        glBindVertexArray(buffers.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // texcoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }

}
