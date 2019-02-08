#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/projection.hpp>
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#undef main
#define not !
const int WIDTH = 1024;
const int HEIGHT = 768;

enum Uniforms
{
    ProjectionMatrix,
    ViewMatrix,
    TransformMatrix,
    SIZE
};

unsigned int uniformLocations[Uniforms::SIZE];
unsigned int vertexShader = 0;
unsigned int fragmentShader = 0;
unsigned int programId = 0;

void CreateShader();
bool ProcessSdlEvent(SDL_Event& e);
void CreateQuad(GLuint& vao, GLuint& vbo_indices, GLuint& vbo_vertex, GLuint& vbo_text_coord, int& indices_size);
void SimpleRenderVao(GLuint vao, int indices, int attributes, GLenum mode);
void LoadValue(unsigned int loacation, const glm::mat4& value);
unsigned int CreateShaderBuffer(int size);

struct PerResize
{
    int swapDir = true;
    float a, b, c;
    glm::vec3 color = glm::vec3(0.1f, 0.8f, 0.2f);
    float d;
    glm::mat4 ProjectionMatrix = glm::perspective(50.f, (float)WIDTH / (float)HEIGHT, 0.1f, 1000.f);
};

struct PerFrame
{
    glm::mat4 gViewMatrix = glm::lookAt(glm::vec3(0, 0, -5), glm::vec3(0), glm::vec3(0, 1, 0));
};

struct PerOject
{
    glm::mat4 gTransformMatix;
};

int main(int, char**)
{
    std::cout << sizeof(PerResize) << std::endl;

    SDL_Init(SDL_INIT_VIDEO);
    auto window = SDL_CreateWindow("OpenGL test app.", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GL_SetSwapInterval(0);
    auto glContext = SDL_GL_CreateContext(window);
    GLint glew_init_result = glewInit();

    SDL_Event e;
    bool run = true;
    float bgColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

    CreateShader();

    GLuint vao, vboi, vbov, vbotx; int indices;

    CreateQuad(vao, vboi, vbov, vbotx, indices);


    auto perResize = CreateShaderBuffer(sizeof(PerResize));
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, perResize);

    PerResize perResizeBufferData;
    glBindBuffer(GL_UNIFORM_BUFFER, perResize);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerResize), &perResizeBufferData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //
    auto perFrame = CreateShaderBuffer(sizeof(PerFrame));
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, perFrame);

    PerFrame perFrameBufferData;
    glBindBuffer(GL_UNIFORM_BUFFER, perFrame);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerFrame), glm::value_ptr(perFrameBufferData.gViewMatrix));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    //

    auto perOject = CreateShaderBuffer(sizeof(PerOject));
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, perOject);

    PerOject perOjectBufferData;
    glBindBuffer(GL_UNIFORM_BUFFER, perOject);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerOject), glm::value_ptr(perOjectBufferData.gTransformMatix));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    glUseProgram(programId);
    //LoadValue(uniformLocations[Uniforms::ProjectionMatrix], gProjectionMatrix);
    //LoadValue(uniformLocations[Uniforms::ViewMatrix], gViewMatrix);
    //LoadValue(uniformLocations[Uniforms::TransformMatrix], gTransformMatix);

    glm::vec3 translation(0.01f, 0, 0);

    while (run)
    {
        while (SDL_PollEvent(&e))
        {
            if (not ProcessSdlEvent(e))
            {
                run = false;
            }

        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);

        {
            translation.x += 0.01f;
            if (translation.x > 8)translation.x = -8;
            perOjectBufferData.gTransformMatix = glm::translate(translation);


            glBindBuffer(GL_UNIFORM_BUFFER, perOject);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerOject), glm::value_ptr(perOjectBufferData.gTransformMatix));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        SimpleRenderVao(vao,indices, 2, GL_TRIANGLES);

        SDL_GL_SwapWindow(window);
    }
    SDL_GL_DeleteContext(glContext);
    SDL_Quit();
    return 0;
}

bool ProcessSdlEvent(SDL_Event& e)
{
    switch (e.type)
    {
    case SDL_QUIT:
        return false;
        break;
    case SDL_MOUSEBUTTONDOWN:
        break;
    case SDL_MOUSEWHEEL:
        break;
    case SDL_KEYDOWN:
        break;
    case SDL_KEYUP:
        break;
    case SDL_FINGERDOWN:
        break;
    }
    return true;
}
std::string LoadFile(const std::string& fn)
{
    std::string result;
    std::ifstream in(fn);
    if (!in.is_open())
    {
        std::cout << "The file " << fn << " cannot be opened\n";
        return {};
    }
    char tmp[300];
    while (!in.eof())
    {
        in.getline(tmp, 300);
        result += tmp;
        result += '\n';
    }
    in.close();

    return result;
}
unsigned int LoadShader(const std::string& filename, unsigned int mode)
{
    std::string source = LoadFile(filename);

    unsigned int id;
    id = glCreateShader(mode);

    const char* csource = source.c_str();

    glShaderSource(id, 1, &csource, NULL);
    glCompileShader(id);

    char error[1000];
    int length = 0;

    glGetShaderInfoLog(id, 1000, &length, error);

    GLint compiled = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        std::cout << "\n\n[Error] ERRORS in Shader! \nFile name:\t" << filename << "\nCompile status: \n\n" << error << std::endl;
    }

    if (id == GL_FALSE)
    {
        system("pause");
        exit(0);
    }
    return id;
}

void BindAttribute(unsigned int attribute, const std::string& variableName)
{
    glBindAttribLocation(programId, attribute, variableName.c_str());
}

void BindAttributes()
{
    BindAttribute(0, "position");
    BindAttribute(1, "textureCoords");
}

void CreateShader()
{
    programId = glCreateProgram();
    glUseProgram(programId);
    vertexShader = LoadShader("shader.vert", GL_VERTEX_SHADER);
    fragmentShader = LoadShader("shader.frag", GL_FRAGMENT_SHADER);
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);
    BindAttributes();
    glLinkProgram(programId);
    glValidateProgram(programId);

    uniformLocations[Uniforms::ProjectionMatrix] = glGetUniformLocation(programId, "ProjectionMatrix");
    uniformLocations[Uniforms::ViewMatrix] = glGetUniformLocation(programId, "ViewMatrix");
    uniformLocations[Uniforms::TransformMatrix] = glGetUniformLocation(programId, "TransformationMatrix");
    glUseProgram(0);
}

int CreateVao()
{
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    return vao_id;
}

GLuint BindIndicesBuffer(const std::vector<unsigned int>& indices)
{
    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    return vbo_id;
}

GLuint StoreDataInAttributesList(int attributeNumber, int coordinateSize, const std::vector<float>& data)
{
    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glVertexAttribPointer(attributeNumber, coordinateSize, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(attributeNumber);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return vbo_id;
}
GLuint StoreDataInAttributesList(int attributeNumber, int coordinateSize, const std::vector<int>& data)
{
    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(int), &data[0], GL_STATIC_DRAW);
    glVertexAttribIPointer(attributeNumber, coordinateSize, GL_INT, 0, 0);
    glEnableVertexAttribArray(attributeNumber);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return vbo_id;
}
GLuint StoreDataInAttributesListInstanced(unsigned int attributeNumber, unsigned int dataSize, unsigned int dataLength, unsigned int offset)
{
    glEnableVertexAttribArray(attributeNumber);
    glVertexAttribPointer(attributeNumber, dataSize, GL_FLOAT, GL_FALSE, dataLength,
        (const GLvoid*)(offset * sizeof(GLfloat)));
    glVertexAttribDivisor(attributeNumber, 1);
    glEnableVertexAttribArray(attributeNumber);
    return 0;
}

void UnbindVao()
{
    glBindVertexArray(0);
}

void EnableCulling()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void DisableCulling()
{
    glDisable(GL_CULL_FACE);
}

void SimpleRenderVao(GLuint vao, int indices, int attributes, GLenum mode)
{
    glBindVertexArray(vao);
    glDrawElements(mode, indices, GL_UNSIGNED_INT, 0);
}

void CreateQuad(GLuint& vao, GLuint& vbo_indices, GLuint& vbo_vertex, GLuint& vbo_text_coord, int& indices_size)
{
    // std::vector<float> vertex = { -0.5, 0.5, 0, -0.5, -0.5, 0, 0.5, -0.5, 0, 0.5, 0.5, 0 };
    std::vector<float> vertex = { -1, 1, 0, -1, -1, 0, 1, -1, 0, 1, 1, 0 };
    std::vector<float> text_coords = { 0, 0, 0, 1, 1, 1, 1, 0 };
    std::vector<unsigned int> indices = { 0, 1, 3, 3, 1, 2 };
    indices_size = indices.size();
    vao = CreateVao();
    vbo_indices = BindIndicesBuffer(indices);
    vbo_vertex = StoreDataInAttributesList(0, 3, vertex);
    vbo_text_coord = StoreDataInAttributesList(1, 2, text_coords);
    UnbindVao();
}
void LoadValue(unsigned int loacation, const glm::mat4& value)
{
    glUniformMatrix4fv(loacation, 1, GL_FALSE, glm::value_ptr(value));
}
unsigned int CreateShaderBuffer(int size)
{
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);  // allocate 150 bytes of memory
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return buffer;
}