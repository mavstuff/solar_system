/*
Copyright (c) 2025 Artem Moroz
*/

#include <cmath>           // Include cmath for math functions
#include <cstring>         // Include cstring for string functions
#include <cstdio>          // Include cstdio for printf
#include <cstdlib>         // Include cstdlib for exit
#include <vector>          // Include vector for dynamic arrays
#include <string>          // Include string for moon names
#include <iostream>

#include <GL/glew.h>       // Include GLEW for OpenGL function loading
#include <glm/glm.hpp>     // Include GLM for matrix and vector operations
#include <glm/gtc/matrix_transform.hpp> // Include GLM transformations
#include <glm/gtc/type_ptr.hpp> // Include GLM type pointers
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_main.h>
#include <SDL_opengl.h>

#include "stb_easy_font.h"

#ifdef main
#undef main
#endif /* main */

#ifndef M_PI
#    define  M_PI  3.14159265358979323846
#endif

glm::mat4 gProjection;
glm::mat4 gView;

// Global variables for rotation angles
std::vector<float> planetRotations = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // Rotations for each planet
std::vector<float> planetOrbits = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};    // Orbits for each planet

// Planet distances from the Sun (scaled down to fit the screen)
std::vector<float> planetDistances = {2.0f, 3.0f, 4.0f, 5.0f, 6.5f, 8.0f, 9.5f, 11.0f, 12.5f};

// Planet sizes (scaled down for visualization)
std::vector<float> planetSizes = {0.1f, 0.15f, 0.2f, 0.15f, 0.4f, 0.35f, 0.3f, 0.3f, 0.05f}; // Reduced sizes

// Planet colors (RGB)
std::vector<std::vector<float>> planetColors = {
    {0.75f, 0.75f, 0.75f}, // Mercury (Gray)
    {0.95f, 0.64f, 0.37f}, // Venus (Orange)
    {0.0f, 0.0f, 1.0f},    // Earth (Blue)
    {1.0f, 0.0f, 0.0f},    // Mars (Red)
    {0.8f, 0.6f, 0.4f},    // Jupiter (Brown)
    {0.9f, 0.8f, 0.5f},    // Saturn (Beige)
    {0.4f, 0.6f, 1.0f},    // Uranus (Cyan)
    {0.0f, 0.4f, 0.8f},    // Neptune (Blue)
    {0.6f, 0.6f, 0.6f}     // Pluto (Gray)
};

// Planet names
std::vector<std::string> planetNames = {
    "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Pluto"
};

// Moons data
struct Moon {
    float distance; // Distance from the planet
    float size;     // Size of the moon
    float orbit;    // Current orbit angle
    float speed;    // Orbit speed
    std::string name; // Name of the moon
};

std::vector<std::vector<Moon>> planetMoons = {
    {}, // Mercury has no moons
    {}, // Venus has no moons
    { {0.2f, 0.05f, 0.0f, 1.0f, "Moon"} }, // Earth has 1 moon (Moon)
    { {0.15f, 0.03f, 0.0f, 1.5f, "Phobos"}, {0.25f, 0.04f, 0.0f, 1.2f, "Deimos"} }, // Mars has 2 moons (Phobos, Deimos)
    { {0.5f, 0.1f, 0.0f, 0.8f, "Europa"}, {0.8f, 0.12f, 0.0f, 0.7f, "Ganymede"}, {1.2f, 0.15f, 0.0f, 0.6f, "Callisto"} }, // Jupiter has 3 moons (Europa, Ganymede, Callisto)
    { {0.6f, 0.1f, 0.0f, 0.7f, "Titan"}, {0.9f, 0.12f, 0.0f, 0.6f, "Rhea"}, {1.3f, 0.14f, 0.0f, 0.5f, "Iapetus"} }, // Saturn has 3 moons (Titan, Rhea, Iapetus)
    { {0.4f, 0.08f, 0.0f, 0.9f, "Titania"}, {0.7f, 0.1f, 0.0f, 0.8f, "Oberon"} }, // Uranus has 2 moons (Titania, Oberon)
    { {0.3f, 0.07f, 0.0f, 1.0f, "Triton"}, {0.6f, 0.09f, 0.0f, 0.9f, "Nereid"} }, // Neptune has 2 moons (Triton, Nereid)
    { {0.1f, 0.02f, 0.0f, 1.2f, "Charon"} } // Pluto has 1 moon (Charon)
};

// Shader program IDs
GLuint planetShaderProgram;
GLuint orbitShaderProgram;
GLuint sunShaderProgram;
GLuint saturnShaderProgram;
GLuint asteroidShaderProgram;

// Asteroid belt data
std::vector<glm::vec3> asteroidPositions; // Positions of asteroids
GLuint asteroidVAO, asteroidVBO, asteroidIBO; // Vertex Array Object, Vertex Buffer Object, Index Buffer Object
GLuint gvaoPlanet, gvboPlanet, gnboPlanet, geboPlanet;
GLuint gvaoOrbit, gvboOrbit;

GLuint numAsteroids = 1000; // Number of asteroids
float asteroidBeltRotation = 0.0f; // Rotation angle for the asteroid belt

SDL_Window* g_Window = NULL;
SDL_GLContext g_glContext = NULL;
bool g_bQuit = false;

void prepareSolidSphere(float radius, int slices, int stacks);
void prepareCircle(float radius, int segments);
void cleanup();

glm::mat4 createViewMatrix(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    return glm::lookAt(eye, center, up);
}


void openglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    (void)source;
    (void)id;
    (void)length;
    (void)userParam;

    const char* severityStr = "Unknown";
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: severityStr = "High"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "Medium"; break;
    case GL_DEBUG_SEVERITY_LOW: severityStr = "Low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    }

    const char* typeStr = "Unknown";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
    case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
    case GL_DEBUG_TYPE_MARKER: typeStr = "Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP: typeStr = "Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;
    }

    fprintf(stderr, "OpenGL Debug Message: Severity = %s, Type = %s\nMessage: %s\n", severityStr, typeStr, message);
}

// Function to load and compile a shader
GLuint loadShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
    }

    return shader;
}

// Function to create a shader program
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = loadShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("Shader program linking error: %s\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}
void renderText(const char* text, int bigger, float x, float y, float z) {
    static char buffer[60000]; // ~300 chars

    // Generate vertex data for the text
    int num_quads = stb_easy_font_print(0, 0, (char*)text, nullptr, buffer, sizeof(buffer));

    // Apply scaling based on the 'bigger' parameter
    float scale = 0.1f + (bigger * 0.1f); // Increase size by 10% for each 'bigger' step

    // Transform vertices to apply scaling
    for (int i = 0; i < num_quads * 4; i++) {
        float* v = (float*)(buffer + i * 16); // Each vertex is 16 bytes (4 floats: x, y, z, padding)
        v[0] = x + v[0] * scale; // Scale x
        v[1] = y - v[1] * scale; // Scale y
        v[2] = z;                // Set z (depth)
    }

    // Render the text
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 16, buffer); // Use 3D vertices (x, y, z)
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

// Function to initialize OpenGL settings and shaders
void init() {
    // Initialize GLEW
    glewExperimental = GL_FALSE;
    if (glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW\n");
        exit(1);
    }

    // Enable debug output
    if (GLEW_KHR_debug) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglDebugCallback, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        printf("OpenGL debug output enabled.\n");
    }
    else {
        fprintf(stderr, "GL_KHR_debug extension not supported!\n");
    }
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
    glEnable(GL_DEPTH_TEST);          // Enable depth testing for 3D rendering


    const char* planetVertexShaderSource =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "uniform mat4 MVP;\n" // Model-View-Projection matrix
        "void main() {\n"
        "    gl_Position = MVP * vec4(aPos, 1.0);\n" // Transform vertex position
        "}\n";

    const char* planetFragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec3 uColor;\n" // Color
        "void main() {\n"
        "    FragColor = vec4(uColor, 1.0);\n" 
        "}\n";

    planetShaderProgram = createShaderProgram(planetVertexShaderSource, planetFragmentShaderSource);


    // Vertex and fragment shaders for orbits
    const char* orbitVertexShaderSource =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "uniform mat4 MVP;\n"
        "void main() {\n"
        "    gl_Position = MVP * vec4(aPos, 1.0);\n" // Transform vertex position
        "}\n";

    const char* orbitFragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    FragColor = vec4(0.5, 0.5, 0.5, 1.0);\n"
        "}\n";

    orbitShaderProgram = createShaderProgram(orbitVertexShaderSource, orbitFragmentShaderSource);


    // Vertex and fragment shaders for the Sun (burning effect)
    const char* sunVertexShaderSource =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "uniform mat4 MVP;\n" // Model-View-Projection matrix
        "void main() {\n"
        "    gl_Position = MVP * vec4(aPos, 1.0);\n" // Transform vertex position
        "}\n";

    const char* sunFragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform float time;\n"
        "void main() {\n"
        "    float intensity = 0.8 + 0.2 * sin(time * 5.0);\n" // Pulsating effect
        "    FragColor = vec4(1.0, 0.5 * intensity, 0.0, 1.0);\n" // Yellow-orange color
        "}\n";

    sunShaderProgram = createShaderProgram(sunVertexShaderSource, sunFragmentShaderSource);

    // Vertex and fragment shaders for Saturn's rings
    const char* saturnVertexShaderSource =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "uniform mat4 MVP;\n" // Model-View-Projection matrix
        "void main() {\n"
        "    gl_Position = MVP * vec4(aPos, 1.0);\n" // Transform vertex position
        "}\n";

    const char* saturnFragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    FragColor = vec4(0.9, 0.8, 0.5, 1.0);\n" // Beige color for Saturn's rings
        "}\n";

    saturnShaderProgram = createShaderProgram(saturnVertexShaderSource, saturnFragmentShaderSource);

    // Vertex and fragment shaders for asteroids
    const char* asteroidVertexShaderSource =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "uniform mat4 MVP;\n" // Model-View-Projection matrix
        "uniform float time;\n" // Time for rotation
        "void main() {\n"
        "    float angle = time * 0.1;\n" // Rotate over time
        "    vec3 rotatedPos = vec3(\n"
        "        aPos.x * cos(angle) - aPos.z * sin(angle),\n"
        "        aPos.y,\n"
        "        aPos.x * sin(angle) + aPos.z * cos(angle)\n"
        "    );\n"
        "    gl_Position = MVP * vec4(rotatedPos, 1.0);\n" // Transform vertex position
        "}\n";

    const char* asteroidFragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform float fogDensity;\n" // Fog density
        "void main() {\n"
        "    float fogFactor = exp(-fogDensity * gl_FragCoord.z / gl_FragCoord.w);\n" // Fog calculation
        "    fogFactor = clamp(fogFactor, 0.0, 1.0);\n"
        "    vec3 fogColor = vec3(0.5, 0.5, 0.5);\n" // Gray fog color
        "    vec3 objectColor = vec3(0.7, 0.7, 0.7);\n" // Gray asteroid color
        "    FragColor = vec4(mix(fogColor, objectColor, fogFactor), 1.0);\n" // Apply fog
        "}\n";

    asteroidShaderProgram = createShaderProgram(asteroidVertexShaderSource, asteroidFragmentShaderSource);

    // Generate asteroid positions
    asteroidPositions.resize(numAsteroids);
    for (int i = 0; i < numAsteroids; i++) {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float distance = 7.0f + static_cast<float>(rand()) / RAND_MAX * 1.0f; // Between 7.0 and 8.0
        float height = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f; // Random height
        asteroidPositions[i] = glm::vec3(distance * cos(angle), height, distance * sin(angle));
    }

    // Create VAO, VBO, and IBO for asteroids
    glGenVertexArrays(1, &asteroidVAO);
    glGenBuffers(1, &asteroidVBO);
    glGenBuffers(1, &asteroidIBO);

    glBindVertexArray(asteroidVAO);

    //// Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, asteroidVBO);
    glBufferData(GL_ARRAY_BUFFER, asteroidPositions.size() * sizeof(glm::vec3), asteroidPositions.data(), GL_STATIC_DRAW);

    //// Set up vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    //// Generate indices for asteroids (each asteroid is a point)
    std::vector<GLuint> indices(numAsteroids);
    for (int i = 0; i < numAsteroids; i++) {
        indices[i] = i;
    }

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, asteroidIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
        
    glBindVertexArray(0); // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind buffers
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    prepareSolidSphere(0.5f, 50, 50);

    prepareCircle(1.0f, 100);
    
}


void prepareSolidSphere(float radius, int slices, int stacks) {
    // Generate vertices, normals, and indices for the sphere
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    // Generate vertices and normals
    for (int i = 0; i <= stacks; ++i) {
        float phi = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(stacks); // Latitude
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(j) / static_cast<float>(slices); // Longitude

            // Vertex position
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            // Normal (same as vertex position normalized)
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));

            vertices.push_back(glm::vec3(x, y, z));
            normals.push_back(normal);
        }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    // Create and bind a VAO
    glGenVertexArrays(1, &gvaoPlanet);
    glBindVertexArray(gvaoPlanet);

    // Create and bind a VBO for vertices
    glGenBuffers(1, &gvboPlanet);
    glBindBuffer(GL_ARRAY_BUFFER, gvboPlanet);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Create and bind a VBO for normals
    glGenBuffers(1, &gnboPlanet);
    glBindBuffer(GL_ARRAY_BUFFER, gnboPlanet);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Create and bind an EBO for indices
    glGenBuffers(1, &geboPlanet);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geboPlanet);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0); // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind buffers
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void drawSolidSphere()
{
    // Draw the sphere
    glBindVertexArray(gvaoPlanet);

    GLint elementArrayBufferID;
    GLint elementArrayBufferSize = 0;

    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementArrayBufferID);

    if (elementArrayBufferID != 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBufferID);
        glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &elementArrayBufferSize);
    }
    glDrawElements(GL_TRIANGLES, elementArrayBufferSize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void cleanupSolidSphere()
{
    // Clean up
    glDeleteVertexArrays(1, &gvaoPlanet);
    glDeleteBuffers(1, &gvboPlanet);
    glDeleteBuffers(1, &gnboPlanet);
    glDeleteBuffers(1, &geboPlanet);
}

void prepareCircle(float radius, int segments)
{
    std::vector<float> vertices;
    vertices.reserve(segments * 3);

    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(radius * cos(angle)); // X
        vertices.push_back(0.0f);                 // Y
        vertices.push_back(radius * sin(angle));  // Z
    }

    glGenVertexArrays(1, &gvaoOrbit);
    glGenBuffers(1, &gvboOrbit);

    glBindVertexArray(gvaoOrbit);

    glBindBuffer(GL_ARRAY_BUFFER, gvboOrbit);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);


    /*
    // Draw the circle
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, segments);
    glBindVertexArray(0);
    */

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void cleanupCircle()
{
    glDeleteVertexArrays(1, &gvaoOrbit);
    glDeleteBuffers(1, &gvboOrbit);
}

void drawCircle(float radius)
{
    glUseProgram(orbitShaderProgram);

    glm::mat4 mvorbit = gProjection * gView;
    mvorbit = glm::scale(mvorbit, glm::vec3(radius, 0.0f, radius));

    // Pass MVP matrix to the shader
    GLint mvpLocation = glGetUniformLocation(planetShaderProgram, "MVP");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvorbit));

    glBindVertexArray(gvaoOrbit);

    GLint arrayBufferID = 0;
    GLint arrayBufferSize = 0;

    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, (GLint*)&arrayBufferID);

    if (arrayBufferID != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, arrayBufferID);
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &arrayBufferSize);
    }
    
    // Draw the circle
    glDrawArrays(GL_LINE_LOOP, 0, arrayBufferSize / (3 * sizeof(float)));


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(0);
}

// Function to draw the Sun with a burning effect
void drawSun() {
    glUseProgram(sunShaderProgram);

    // Calculate MVP matrix for the Sun using GLM
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix
    glm::mat4 MVP = gProjection * gView * model;

    // Pass MVP matrix to the shader
    GLint mvpLocation = glGetUniformLocation(sunShaderProgram, "MVP");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));

    // Pass time uniform to the shader
    float time = SDL_GetTicks() / 1000.0f; // Get time in seconds
    GLint timeLocation = glGetUniformLocation(sunShaderProgram, "time");
    glUniform1f(timeLocation, time);

    drawSolidSphere();      // Draw the Sun


    glUseProgram(0); // Switch back to fixed-function pipeline
}

// Function to draw Saturn's rings
void drawSaturnRings(float radius) {
    glUseProgram(saturnShaderProgram);

    // Calculate MVP matrix for Saturn's rings using GLM
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix
    glm::mat4 MVP = gProjection * gView * model;

    // Pass MVP matrix to the shader
    GLint mvpLocation = glGetUniformLocation(saturnShaderProgram, "MVP");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++) {
        float angle = 2.0f * M_PI * i / 100;
        glVertex3f(radius * cos(angle), 0.0f, radius * sin(angle));
    }
    glEnd();

    glUseProgram(0); // Switch back to fixed-function pipeline
}

// Function to draw a moon
void drawMoon(glm::mat4 mvorig, float distance, float size, float orbitAngle, float planetAngle, float speed, const std::string& name) {
        
    glm::mat4 mvorbit, mvmoonscaled, mvtext;

    mvorbit = mvorig;
    // Move to the moon's orbit
    mvorbit = glm::rotate(mvorbit, glm::radians(orbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Rotate the moon on its axis
    mvorbit = glm::translate(mvorbit, glm::vec3(distance, 0.0f, 0.0f));
    
    mvmoonscaled = glm::scale(mvorbit, glm::vec3(size / 0.5f));

    mvtext = glm::rotate(mvorbit, glm::radians(360.0f - planetAngle - orbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    glUseProgram(planetShaderProgram);
    // Pass MVP matrix to the shader
    GLint mvpLocation = glGetUniformLocation(planetShaderProgram, "MVP");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvmoonscaled));

    GLint colorLocation = glGetUniformLocation(planetShaderProgram, "uColor");
    glm::vec3 color = glm::vec3(0.8f, 0.8f, 0.8f);
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));

    drawSolidSphere();

    glUseProgram(0);
    

    // Render the moon's name
    //glColor3f(1.0f, 1.0f, 1.0f); // White color for text
    //renderText(name.c_str(), 0, 0.0f, - (size + 0.5f), 0.0f); // Display name above the moon

    
}

// Function to draw a planet and its moons
void drawPlanet(float radius, float distance, const std::vector<float>& color, float orbitAngle, float rotationAngle, const std::string& name, const std::vector<Moon>& moons) {
    glm::mat4 mvorig = gProjection * gView, mvorbit, mvplanet, mvplanetscaled, mvtext;

    mvorbit = mvorig;
    // Rotate around the Sun
    mvorbit = glm::rotate(mvorbit, glm::radians(orbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    // Move to the planet's orbit
    mvorbit = glm::translate(mvorbit, glm::vec3(distance, 0.0f, 0.0f));
    // Rotate the planet on its axis
    mvplanet = glm::rotate(mvorbit, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
   
    mvplanetscaled = glm::scale(mvplanet, glm::vec3(radius / 0.5f));

    // Rotate the text
    mvtext = glm::rotate(mvorbit, glm::radians(360.0f - orbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));


    glUseProgram(planetShaderProgram);
    // Pass MVP matrix to the shader
    GLint mvpLocation = glGetUniformLocation(planetShaderProgram, "MVP");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvplanetscaled));

    GLint colorLocation = glGetUniformLocation(planetShaderProgram, "uColor");
    glUniform3fv(colorLocation, 1, color.data());

    drawSolidSphere();

    glUseProgram(0);

    // Draw Saturn's rings if it's Saturn
    if (name == "Saturn") {
        //drawSaturnRings(radius * 1.5); // Draw rings around Saturn
    }

    // Draw moons
    for (const Moon& moon : moons) {
        drawMoon(mvplanet, moon.distance, moon.size, moon.orbit, rotationAngle + orbitAngle, moon.speed, moon.name);
    }

    //glLoadMatrixf(glm::value_ptr(mvtext));
   
    // Render the planet's name
    //glColor3f(1.0, 1.0, 1.0); // White color for text
    //renderText(name.c_str(), 1, 0.0f, radius + 1.0f, 0.0f); // Display name above the planet

    //glPopMatrix();
}

// Function to draw the asteroid belt
void drawAsteroidBelt() {
    glUseProgram(asteroidShaderProgram);

    // Calculate MVP matrix for the asteroid belt using GLM
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix
    // Get the View matrix
    glm::mat4 view;
    glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);

    // Get the Projection matrix
    glm::mat4 projection;
    glGetFloatv(GL_PROJECTION_MATRIX, &projection[0][0]);

    glm::mat4 MVP = projection * view * model;

    // Pass MVP matrix to the shader
    GLint mvpLocation = glGetUniformLocation(asteroidShaderProgram, "MVP");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));

    // Pass time uniform to the shader for rotation
    float time = SDL_GetTicks() / 1000.0f; // Get time in seconds
    GLint timeLocation = glGetUniformLocation(asteroidShaderProgram, "time");
    glUniform1f(timeLocation, time);

    // Pass fog density to the shader
    GLint fogDensityLocation = glGetUniformLocation(asteroidShaderProgram, "fogDensity");
    glUniform1f(fogDensityLocation, 0.05f); // Adjust fog density as needed

    // Draw asteroids
    glBindVertexArray(asteroidVAO);
    glDrawElements(GL_POINTS, numAsteroids, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0); // Switch back to fixed-function pipeline
}

// Function to display the solar system
void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
    
    // Create the view matrix using glm::lookAt
    gView = glm::lookAt(
        glm::vec3(0.0f, 30.0f, 50.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f));

   
    // Draw the Sun at the center
    drawSun();
    
    
    // Draw planet orbits
    //glColor3f(0.5f, 0.5f, 0.5f); // Gray color for orbits
    for (int i = 0; i < 9; i++) {
        drawCircle(planetDistances[i]); // Draw orbit for each planet
    }

    // Draw all 9 planets with their names and moons
    for (int i = 0; i < 9; i++) {
        drawPlanet(planetSizes[i], planetDistances[i], planetColors[i], planetOrbits[i], planetRotations[i], planetNames[i], planetMoons[i]);
    }

    // Draw the asteroid belt
    //drawAsteroidBelt();
    
    
}

// Function to update the rotation and orbit angles
void update() {
    // Update planet rotations and orbits
    for (int i = 0; i < 9; i++) {
        planetRotations[i] += 1.0f; // Rotate each planet on its axis
        planetOrbits[i] += 0.1f * (i + 1); // Orbit each planet around the Sun
        if (planetRotations[i] > 360) planetRotations[i] -= 360;
        if (planetOrbits[i] > 360) planetOrbits[i] -= 360;
    }

    // Update moon orbits
    for (int i = 0; i < 9; i++) {
        for (Moon& moon : planetMoons[i]) {
            moon.orbit += moon.speed; // Rotate moon around its planet
            if (moon.orbit > 360) moon.orbit -= 360;
        }
    }

    //glutPostRedisplay(); // Redraw the scene
    //glutTimerFunc(16, update, 0); // Call update function every 16ms (~60 FPS)
}

// Function to handle window resizing
void reshape(int w, int h) {
    glViewport(0, 0, w, h); // Set the viewport to cover the new window
    // Create a perspective projection matrix using GLM
    gProjection = glm::perspective(
        glm::radians(30.0f), // Field of View (FOV) in radians (30 degrees)
        (float)w / (float)h, // Aspect ratio
        1.0f,               // Near clipping plane
        200.0f              // Far clipping plane
    );
}


void mainloop()
{
    bool bRedraw = false;
    
    const Uint32 nFrameTime = 1000 / 60;
    Uint32 nFrameStart = SDL_GetTicks();


    SDL_Event e = {};
    while (SDL_PollEvent(&e))
    {
        switch (e.type) {

        case SDL_QUIT:
        {
            g_bQuit = true;
            return;
            break;
        }
        case SDL_WINDOWEVENT:
        {
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_EXPOSED:
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    int w, h;
                    SDL_GetWindowSize(g_Window, &w, &h);
                    reshape(w,h);
               }
            break;
            }

            break;
        }

        

        default:
            break;
        }
    }

    update();
    display();
    SDL_GL_SwapWindow(g_Window);

    Uint32 nElapsedTime = SDL_GetTicks() - nFrameStart;
    if (nFrameTime > nElapsedTime)
    {
        SDL_Delay(nFrameTime - nElapsedTime);
    }
}



// Main function
int main(int argc, char** argv) 
{
    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        // Set OpenGL attributes before window creation
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // Use Core profile
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Enable double buffering

        //Create window
        g_Window = SDL_CreateWindow("Solar System Simulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
        if (g_Window != NULL)
        {
            g_glContext = SDL_GL_CreateContext(g_Window);
            if (g_glContext != NULL)
            {
                init();

                while (!g_bQuit)
                {
                    mainloop();
                }

                cleanup();

               
                SDL_GL_DeleteContext(g_glContext);
            }
          

            SDL_DestroyWindow(g_Window);
        }
        SDL_Quit();
    }
    return 0;
}

void cleanup()
{
    cleanupSolidSphere();
    cleanupCircle();
}

