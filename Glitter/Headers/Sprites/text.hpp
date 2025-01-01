#pragma once
#include <GLFW/glfw3.h>
#include "glad/glad.h"
#include "stb_truetype/stb_truetype.h"
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <Helpers/shader.hpp>
#include "glm/gtc/type_ptr.hpp"
#include <filesystem>
namespace fs = std::filesystem;

namespace Sprites{
    class Text
    {
        public:
            Text(std::string txt, glm::vec3 pos)
            {   
                this->text = txt;
                this->position = pos;
                setup();
            }
            void updatePosition(glm::vec3 pos)
            {
                this->position = pos;
            }
            void setup()
            {
                shader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/textShader.vert","E:/OpenGL/Glitter/Glitter/Shaders/textShader.frag");
                shader->use();

                stbtt_fontinfo font;
                auto ttfBuffer = new unsigned char[1 << 20]; // 1 MB buffer
                auto bitmap = new unsigned char[512 * 512];  // 256 KB bitmap

                // Load the font
                FILE* fontFile = fopen("E:/OpenGL/Glitter/EngineAssets/Roboto/Roboto-Regular.ttf", "rb");
                fread(ttfBuffer, 1, 1 << 20, fontFile);
                fclose(fontFile);

                // Initialize font
                stbtt_InitFont(&font, ttfBuffer, stbtt_GetFontOffsetForIndex(ttfBuffer, 0));

                // Create a bitmap of glyphs
                std::cout << "ttfBuffer address: " << ttfBuffer << std::endl;
                std::cout << "bitmap address: " << bitmap << std::endl;
                stbtt_BakeFontBitmap(ttfBuffer, 0, 32.0f, bitmap, 512, 512, 32, 96, cdata);  // ASCII range [32..126]

                // Upload the bitmap as a texture to OpenGL
                glGenTextures(1, &fontTexture);
                glBindTexture(GL_TEXTURE_2D, fontTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }

            void RenderText3D(
            const glm::mat4& viewMatrix,
            const glm::mat4& projectionMatrix
            ) 
            {
                shader->use();

                // Activate the texture
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fontTexture);
                glUniform1i(glGetUniformLocation(shader->ID, "u_Texture"), 0);

                glm::mat4 billboardMatrix = GetBillboardMatrix(position, viewMatrix);

                glm::mat4 mvp = projectionMatrix * viewMatrix * billboardMatrix;
                glUniformMatrix4fv(glGetUniformLocation(shader->ID, "u_MVP"), 1, GL_FALSE, glm::value_ptr(mvp));

                float x = 0.0f;  // Initial x position for rendering text
                float y = 0.0f;  // Initial y position for rendering text

                // Render each character
                for (char c : text) {
                    stbtt_aligned_quad q;
                    stbtt_GetBakedQuad(cdata, 512, 512, c - 32, &x, &y, &q, 1);  // 512x512 texture atlas size

                    float vertices[] = {
                    q.x0, q.y0, 0.0f, q.s0, q.t0,
                    q.x1, q.y0, 0.0f, q.s1, q.t0,
                    q.x1, q.y1, 0.0f, q.s1, q.t1,
                    q.x0, q.y1, 0.0f, q.s0, q.t1
                    };

                    GLuint indices[] = {0, 1, 2, 2, 3, 0};

                    // Upload and draw vertex data
                    GLuint VAO, VBO, EBO;
                    glGenVertexArrays(1, &VAO);
                    glGenBuffers(1, &VBO);
                    glGenBuffers(1, &EBO);

                    glBindVertexArray(VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                    glEnableVertexAttribArray(0);

                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
                    glEnableVertexAttribArray(1);

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                    // Cleanup
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glBindVertexArray(0);
                    glDeleteVertexArrays(1, &VAO);
                    glDeleteBuffers(1, &VBO);
                    glDeleteBuffers(1, &EBO);
                }
            }
        private:

        glm::mat4 GetBillboardMatrix(const glm::vec3& position, const glm::mat4& viewMatrix) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), position);

            glm::mat3 rotation = glm::mat3(viewMatrix);
            rotation = glm::transpose(rotation); // Invert rotation for billboard effect

            glm::mat4 rotationFix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1, 0, 0));

            model *= glm::mat4(rotation) * rotationFix;
            model = glm::scale(model, glm::vec3(0.01f));

            return model;
        }

        std::string text;
        glm::vec3 position;
        Shader* shader;
        GLuint fontTexture;
        stbtt_bakedchar cdata[96];

    };


}