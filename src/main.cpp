#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <random>
#include <vector>

class Particle {
public:
    const float GRAVITY = -1.0f; // Adjust the value as needed
    const float DAMPING_FACTOR = 0.99;
    Particle(float minX, float minY, float maxX, float maxY, float minVX, float minVY, float maxVX, float maxVY, float aspectRatio)
        : aspectRatio(aspectRatio), mass(1.0f) {  // Assume mass of 1 for all particles
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> disX(minX, maxX);
        std::uniform_real_distribution<float> disY(minY, maxY);
        std::uniform_real_distribution<float> disVX(minVX, maxVX);
        std::uniform_real_distribution<float> disVY(minVY, maxVY);
        std::uniform_int_distribution<int> disColor(0, 1);

        x = disX(gen);
        y = disY(gen);
        vx = disVX(gen);
        vy = disVY(gen);
        do {
            color.r = disColor(gen);
            color.g = disColor(gen);
            color.b = disColor(gen);
        } while ((color.r == 0 && color.g == 0 && color.b == 0) || (color.r == 1 && color.g == 1 && color.b == 1)); // Ensure the ball isn't black or white
    }

    void update(float dt) {
        vy += GRAVITY *dt;
        x += vx * dt;
        y += vy * dt;
        handleBoundaryCollision();
    }

    void draw() const {
        const int numSegments = 50;
        float radius = 0.05f;

        glColor3f(color.r, color.g, color.b);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);  // Center of the circle
        for (int i = 0; i <= numSegments; i++) {
            float theta = 2.0f * M_PI * i / numSegments;
            float dx = radius * cosf(theta);
            float dy = radius * sinf(theta);
            glVertex2f(x + dx, y + dy);
        }
        glEnd();
    }

    void updateAspectRatio(float newAspectRatio) {
        aspectRatio = newAspectRatio;
    }

    // Detect and resolve collision with another particle
    void resolveCollision(Particle& other) {
        float dx = other.x - x;
        float dy = other.y - y;
        float distance = sqrt(dx * dx + dy * dy);
        float radius = 0.05f;

        // Check if the particles are colliding
        if (distance < 2 * radius) {
            // Normal vector
            float nx = dx / distance;
            float ny = dy / distance;

            // Relative velocity
            float dvx = other.vx - vx;
            float dvy = other.vy - vy;

            // Relative velocity in normal direction
            float relVel = dvx * nx + dvy * ny;

            // Only resolve if particles are moving towards each other
            if (relVel > 0) return;

            // Impulse scalar
            float impulse = (2 * relVel) / (mass + other.mass);

            // Apply impulse to the particles
            vx += impulse * other.mass * nx;
            vy += impulse * other.mass * ny;
            other.vx -= impulse * mass * nx;
            other.vy -= impulse * mass * ny;

            // Separate the particles to prevent overlap
            float overlap = 2 * radius - distance;
            float separationX = overlap * nx / 2.0f;
            float separationY = overlap * ny / 2.0f;
            x -= separationX;
            y -= separationY;
            other.x += separationX;
            other.y += separationY;
        }
    }

private:
    float x, y;
    float vx, vy;
    float aspectRatio;
    float mass; // Add mass property

    struct Color {
        float r, g, b;
    } color;

    void handleBoundaryCollision() {
        float radius = 0.05f;
        float adjustedRadiusX = radius * aspectRatio;

        if (x < (-aspectRatio + adjustedRadiusX)) {
            x = (-aspectRatio + adjustedRadiusX);
            vx = std::abs(vx);
        } else if (x > (aspectRatio - adjustedRadiusX)) {
            x = (aspectRatio - adjustedRadiusX);
            vx = -std::abs(vx);
        }
        if (y < (-1.0f + radius)) {
            y = (-1.0f + radius);
            vy = std::abs(vy);
        } else if (y > (1.0f - radius)) {
            y = (1.0f - radius);
            vy = -std::abs(vy);
        }
    }
};

void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

void setupProjection(int screenWidth, int screenHeight) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspectRatio = screenWidth / static_cast<float>(screenHeight);
    if (aspectRatio >= 1.0f) {
        // Wider than tall
        glOrtho(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        // Taller than wide
        glOrtho(-1.0f, 1.0f, -1.0f / aspectRatio, 1.0f / aspectRatio, -1.0f, 1.0f);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    setupProjection(width, height);

    float aspectRatio = width / static_cast<float>(height);

    // Update the aspect ratio for all particles
    auto particles = static_cast<std::vector<Particle>*>(glfwGetWindowUserPointer(window));
    for (auto& particle : *particles) {
        particle.updateAspectRatio(aspectRatio);
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = glfwCreateWindow(3456, 2102, "Particle Simulation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    std::cout << "Screen Width: " << screenWidth << std::endl;
    std::cout << "Screen Height: " << screenHeight << std::endl;

    float aspectRatio = screenWidth / static_cast<float>(screenHeight);

    setupProjection(screenWidth, screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    // Number of particles
    int numParticles = 500; // Change this to set the number of particles

    std::vector<Particle> particles;
    particles.reserve(numParticles);
    for (int i = 0; i < numParticles; ++i) {
        particles.emplace_back(-aspectRatio, -1.0f, aspectRatio, 1.0f, -0.05f, -0.05f, 0.05f, 0.05f, aspectRatio);
    }

    glfwSetWindowUserPointer(window, &particles);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    float dt = 0.016f;  // Approximately 60 frames per second

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Check for collisions and update particles
        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = i + 1; j < particles.size(); ++j) {
                particles[i].resolveCollision(particles[j]);
            }
        }

        for (auto& particle : particles) {
            particle.update(dt);
            particle.draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
