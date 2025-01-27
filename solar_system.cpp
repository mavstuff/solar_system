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
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>     // Include GLM for matrix and vector operations
#include <glm/gtc/matrix_transform.hpp> // Include GLM transformations
#include <glm/gtc/type_ptr.hpp> // Include GLM type pointers
#include "stb_easy_font.h"

#ifndef M_PI
#    define  M_PI  3.14159265358979323846
#endif

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
GLuint sunShaderProgram;
GLuint saturnShaderProgram;
GLuint asteroidShaderProgram;

// Asteroid belt data
std::vector<glm::vec3> asteroidPositions; // Positions of asteroids
GLuint asteroidVAO, asteroidVBO, asteroidIBO; // Vertex Array Object, Vertex Buffer Object, Index Buffer Object
GLuint numAsteroids = 1000; // Number of asteroids
float asteroidBeltRotation = 0.0f; // Rotation angle for the asteroid belt



// OpenGL error callback function
void GLAPIENTRY openglErrorCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam) {
    std::cerr << "OpenGL Error:" << std::endl;
    std::cerr << "  Source: " << source << std::endl;
    std::cerr << "  Type: " << type << std::endl;
    std::cerr << "  ID: " << id << std::endl;
    std::cerr << "  Severity: " << severity << std::endl;
    std::cerr << "  Message: " << message << std::endl;
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
    float scale = 1.0f + (bigger * 0.1f); // Increase size by 10% for each 'bigger' step

    // Transform vertices to apply scaling
    for (int i = 0; i < num_quads * 4; i++) {
        float* v = (float*)(buffer + i * 16); // Each vertex is 16 bytes (4 floats: x, y, z, padding)
        v[0] = x + v[0] * scale; // Scale x
        v[1] = y + v[1] * scale; // Scale y
        v[2] = z;                // Set z (depth)
    }

    // Render the text
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 16, buffer); // Use 3D vertices (x, y, z)
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

// Function to draw a circle (for planet orbits)
void drawCircle(float radius, int segments) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        glVertex3f(radius * cos(angle), 0.0f, radius * sin(angle));
    }
    glEnd();
}

// Function to initialize OpenGL settings and shaders
void init() {
    // Initialize GLEW
    glewExperimental = GL_FALSE;
    if (glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW\n");
        exit(1);
    }
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
    glEnable(GL_DEPTH_TEST);          // Enable depth testing for 3D rendering

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
    
}

void drawSolidSphere(float radius, int slices, int stacks) {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;

    for (int i = 0; i <= stacks; ++i) {
        float phi = static_cast<float>(i) / static_cast<float>(stacks) * M_PI;
        for (int j = 0; j <= slices; ++j) {
            float theta = static_cast<float>(j) / static_cast<float>(slices) * 2.0f * M_PI;

            float x = cos(theta) * sin(phi);
            float y = cos(phi);
            float z = sin(theta) * sin(phi);

            float u = static_cast<float>(j) / static_cast<float>(slices);
            float v = static_cast<float>(i) / static_cast<float>(stacks);

            vertices.push_back(radius * x);
            vertices.push_back(radius * y);
            vertices.push_back(radius * z);

            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);

            texCoords.push_back(u);
            texCoords.push_back(v);
        }
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glNormalPointer(GL_FLOAT, 0, normals.data());
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords.data());

    for (int i = 0; i < stacks; ++i) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            int index = i * (slices + 1) + j;
            glArrayElement(index);
            glArrayElement(index + slices + 1);
        }
        glEnd();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

// Function to draw the Sun with a burning effect
void drawSun() {
    glUseProgram(sunShaderProgram);

    // Pass time uniform to the shader
    float time = glfwGetTime(); // Get time in seconds
    GLint timeLocation = glGetUniformLocation(sunShaderProgram, "time");
    glUniform1f(timeLocation, time);

    // Calculate MVP matrix for the Sun using GLM
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix
    // Get the View matrix
    glm::mat4 view;
    glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);

    // Get the Projection matrix
    glm::mat4 projection;
    glGetFloatv(GL_PROJECTION_MATRIX, &projection[0][0]);

    glm::mat4 MVP = projection * view * model;

    // Pass MVP matrix to the shader
    GLint mvpLocation = glGetUniformLocation(sunShaderProgram, "MVP");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));

    drawSolidSphere(0.5, 50, 50); // Draw the Sun with a smaller radius (0.5)

    glUseProgram(0); // Switch back to fixed-function pipeline
}

// Function to draw Saturn's rings
void drawSaturnRings(float radius) {
    glUseProgram(saturnShaderProgram);

    // Calculate MVP matrix for Saturn's rings using GLM
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix

    // Get the View matrix
    glm::mat4 view;
    glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);

    // Get the Projection matrix
    glm::mat4 projection;
    glGetFloatv(GL_PROJECTION_MATRIX, &projection[0][0]);

    // Calculate the Model-View-Projection matrix
    glm::mat4 MVP = projection * view * model;

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
void drawMoon(float distance, float size, float orbitAngle, float speed, const std::string& name) {
    glPushMatrix();
    glRotatef(orbitAngle, 0.0, 1.0, 0.0); // Rotate around the planet
    glTranslatef(distance, 0.0, 0.0);     // Move to the moon's orbit
    glColor3f(0.8f, 0.8f, 0.8f); // Gray color for moons
    drawSolidSphere(size, 20, 20); // Draw the moon

    // Render the moon's name
    glColor3f(1.0, 1.0, 1.0); // White color for text
    renderText(name.c_str(), 0, 0.0, size + 0.05, 0.0); // Display name above the moon

    glPopMatrix();
}

// Function to draw a planet and its moons
void drawPlanet(float radius, float distance, const std::vector<float>& color, float orbitAngle, float rotationAngle, const std::string& name, const std::vector<Moon>& moons) {
    glPushMatrix();
    glRotatef(orbitAngle, 0.0, 1.0, 0.0); // Rotate around the Sun
    glTranslatef(distance, 0.0, 0.0);     // Move to the planet's orbit
    glRotatef(rotationAngle, 0.0, 1.0, 0.0); // Rotate the planet on its axis
    glColor3fv(color.data()); // Set planet color
    drawSolidSphere(radius, 20, 20); // Draw the planet

    // Draw Saturn's rings if it's Saturn
    if (name == "Saturn") {
        drawSaturnRings(radius * 1.5); // Draw rings around Saturn
    }

    // Draw moons
    for (const Moon& moon : moons) {
        drawMoon(moon.distance, moon.size, moon.orbit, moon.speed, moon.name);
    }

    // Render the planet's name
    glColor3f(1.0, 1.0, 1.0); // White color for text
    renderText(name.c_str(), 1, 0.0, radius + 0.2, 0.0); // Display name above the planet

    glPopMatrix();
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
    float time = glfwGetTime(); // Get time in seconds
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

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // Reset the model-view matrix
  
    //glm::mat3 mvp = glm::lookAt(glm::vec3(0.0f, 30.0f, 50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat3 mvp = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));


    glLoadMatrixf(glm::value_ptr(mvp));


    // Draw a simple object (e.g., a colored cube)
    glBegin(GL_QUADS);
    // Front face (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    // Back face (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    // Other faces omitted for brevity...
    glEnd();

    // Draw the Sun at the center
    //drawSun();
    /*
    // Draw planet orbits
    glColor3f(0.5f, 0.5f, 0.5f); // Gray color for orbits
    for (int i = 0; i < 9; i++) {
        drawCircle(planetDistances[i], 100); // Draw orbit for each planet
    }

    // Draw all 9 planets with their names and moons
    for (int i = 0; i < 9; i++) {
        //drawPlanet(planetSizes[i], planetDistances[i], planetColors[i], planetOrbits[i], planetRotations[i], planetNames[i], planetMoons[i]);
    }

    // Draw the asteroid belt
    //drawAsteroidBelt();

    */
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
void reshape(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h); // Set the viewport to cover the new window
    glMatrixMode(GL_PROJECTION); // Switch to the projection matrix
    glLoadIdentity(); // Reset the projection matrix
    gluPerspective(30.0, (double)w / (double)h, 1.0, 200.0); // Adjust FOV to 30 degrees
    glMatrixMode(GL_MODELVIEW); // Switch back to the model-view matrix
}

glm::mat4 createViewMatrix(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    return glm::lookAt(eye, center, up);
}

// Main function
int main(int argc, char** argv) {
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // Enable debug context

    // Set OpenGL version to 3.3
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Solar System Simulation", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set the swap interval to 1 to enable vsync (optional)
    glfwSwapInterval(1);

    init(); // Initialize OpenGL settings
    
    //glfwSetWindowSizeCallback(window, reshape);
    //reshape(window, 800, 600);

    // Initialize OpenGL debug context and set the error callback
    if (glfwExtensionSupported("GL_ARB_debug_output")) {
        std::cout << "Debug output supported" << std::endl;
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglErrorCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
    else {
        std::cerr << "Debug output not supported" << std::endl;
    }

    double lastTime = glfwGetTime();
    const double updateInterval = 0.016; // 16 milliseconds

    glm::vec3 eye(0.0f, 0.0f, 5.0f);    // Camera position
    glm::vec3 center(0.0f, 0.0f, 0.0f); // Point the camera is looking at
    glm::vec3 up(0.0f, 1.0f, 0.0f);     // Up vector

    while (!glfwWindowShouldClose(window)) {


        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set the projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

        // Set the modelview matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Create the view matrix using glm::lookAt
        glm::mat4 viewMatrix = createViewMatrix(eye, center, up);

        // Apply the view matrix to the current OpenGL context
        glLoadMatrixf(glm::value_ptr(viewMatrix));

        // Draw a simple object (e.g., a colored cube)
        glBegin(GL_QUADS);
        // Front face (red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);

        // Back face (green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);

        // Other faces omitted for brevity...
        glEnd();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();


        /*
        double currentTime = glfwGetTime();
        // Update logic
        if ((currentTime - lastTime) > updateInterval) {
            //update();
            lastTime = currentTime;
        }

        display();


        glfwSwapBuffers(window);
        glfwPollEvents();
        */


    }

    //glutTimerFunc(0, update, 0); // Register timer callback function

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
