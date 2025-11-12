#include "../HeaderFiles/Particle.h"

#define M_PI 3.1415926535897932384626433832f

#if PROFILE_NEIGHBORS
    int g_nMaxNeighbors = 0;
#endif

ParticleParameters g_ParticleParameters;

//Defining static members
std::vector <float> Particle::positions;
std::vector <unsigned int> Particle::indices;
std::vector <Particle> Particle::particles;

// Can't use ctor cells(size, GridCol(size)) since particle radius is no longer a static.
//int size = 2.0f / Particle::s_Radius;
//std::vector <std::vector <std::unordered_map<int, bool>>> Particle::cells(size, std::vector <std::unordered_map<int, bool>> (size));
std::vector <GridCol> Particle::cells(1,GridCol(1)); // [x][y][idx] = true/false

unsigned int Particle::vao = 0;
unsigned int Particle::vbo = 0;
unsigned int Particle::ibo = 0;

inline void utilPositionToGridXY(const glm::vec3 pos, int& x, int& y)
{
    const glm::vec3 translate(1.0f, 1.0f, 0.0f);
    const float ooGridRadius = g_ParticleParameters.ooGridRadius;

    glm::vec3 cellPos = pos;
    cellPos += translate;
    cellPos *= ooGridRadius;
    x = (int)cellPos.x;
    y = (int)cellPos.y;
}

void checkBoundary(Particle& p) {
    const float r = g_ParticleParameters.radius;

    // check left
    if (p.pos.x < -0.9f + r) p.pos.x = -0.9f + r, p.velocity.x = -p.velocity.x * 0.5f;

    // check right
    if (p.pos.x > 0.9f - r) p.pos.x = 0.9f  - r, p.velocity.x = -p.velocity.x * 0.5f;

    // check top
    if (p.pos.y > 0.9f - r) p.pos.y = 0.9f - r, p.velocity.y = -p.velocity.y * 0.5f;

    //check bottom
    if (p.pos.y < -0.9f + r) p.pos.y = -0.9f + r, p.velocity.y = -p.velocity.y * 0.5f;
}

void Particle::generateRandomCenters() {
    const float r = g_ParticleParameters.radius;

    for (int i = 0; i < g_ParticleParameters.numOfParticles; i++) {
        Particle::centers.push_back(glm::linearRand(-0.9f + r, 0.9f - r));
        Particle::centers.push_back(glm::linearRand(-0.9f + r, 0.9f - r));
    }
}

void Particle::generateGridCenters(int rows, int cols) {
    const float space1 =     g_ParticleParameters.radius + g_ParticleParameters.spacing;
    const float space2 = 2 * g_ParticleParameters.radius + g_ParticleParameters.spacing;

    float left = 0.0f - space2 * cols / 2.0f;
    float top = 0.9f - space1;
    for (int i = 0; i < rows; i++) {
        
        for (int j = 0; j < cols; j++) {
            Particle::centers.push_back(left + j * space2);
            Particle::centers.push_back(top);
        }
        top -= space2;
    }
}

void Particle::generateParticle(float aspectRatio) {
    const float partRadius = g_ParticleParameters.radius;
    const int   numSegment = g_ParticleParameters.numSegments;

    positions.push_back(0.0f);
    positions.push_back(0.0f);

    int startingIndex = (int)positions.size() / 2;

    for (int idxSegment = 0; idxSegment <= numSegment; idxSegment++) {
        float theta = 2.0f * M_PI * (float)idxSegment / (float)numSegment;
        float x = partRadius * cosf(theta);
        float y = partRadius * sinf(theta);
        positions.push_back((x) / aspectRatio);
        positions.push_back(y);

        if (idxSegment == 0) continue;

        indices.push_back(startingIndex                 );
        indices.push_back(startingIndex + idxSegment + 0);
        indices.push_back(startingIndex + idxSegment + 1);
    }
}

void Particle::populate(float aspectRatio) {
    const int gridDim = g_ParticleParameters.gridDim;

    std::vector <GridCol> grid(gridDim, GridCol(gridDim)); // cells[x][y][idx]
    Particle::cells = grid;

    // generating Centers
    for (int i = 0; i < centers.size(); i += 2) {
        Particle p;
        p.velocity = glm::vec3(0.0f);
        p.acceleration = glm::vec3(0.0f);
        p.pos = glm::vec3(centers[i], centers[i + 1], 0.0f);
        p.density = 0.0f;
        p.generateParticle(aspectRatio);
        particles.push_back(p);

        // populating cells
        int cellX, cellY;
        utilPositionToGridXY( p.pos, cellX, cellY );
        cells[cellX][cellY][i/2] = true;
    }
}

void Particle::updateCell(int idx, int prevX, int prevY) {
    cells[prevX][prevY][idx] = false;
    int x, y;
    utilPositionToGridXY( particles[idx].pos, x, y );
    cells[x][y][idx] = true;
}

std::vector<Particle> Particle::findNeighbors(int idx) {
    const int   gridDim       = g_ParticleParameters.gridDim - 1;

    Particle& p = particles[idx];
    int cellX, cellY;
    utilPositionToGridXY( p.pos, cellX, cellY );

    std::vector <Particle> neighborsOut;
    for (int i = -1; i <= 1; i++) {
        if (cellX + i < 0 || cellX + i > gridDim) continue;
        for (int j = -1; j <= 1; j++) {
            if (cellY + j < 0 || cellY + j > gridDim) continue;
            for (std::pair<int, bool> neighbor : cells[cellX + i][cellY + j]) {
                if (neighbor.first != idx && neighbor.second) neighborsOut.push_back(particles[neighbor.first]);
            }
        }
    }
#if PROFILE_NEIGHBORS
    int numNeighbors = neighborsOut.size();
    if (g_nMaxNeighbors < numNeighbors)
        g_nMaxNeighbors = numNeighbors;
#endif
    return neighborsOut;
}

float Particle::densityKernel(float dst) {
    const float radius = g_ParticleParameters.gridRadius;

    if (dst >= radius) return 0;
    float scale = 4.0f / (M_PI * std::powf(radius, 8.0f));
    float val = radius*radius - dst*dst;
    return val * val * val * scale;
}

float Particle::nearDensityKernel(float dst) {
    const float gridRadius   = g_ParticleParameters.gridRadius;
    const float ooGridRadius = g_ParticleParameters.ooGridRadius;

    if (dst >= gridRadius) return 0;
    float val = 1 - dst * ooGridRadius;
    return val * val * val;
}

float Particle::pressureKernel(float dst) {
    const float gridRadius   = g_ParticleParameters.gridRadius;

    if (dst >= gridRadius) return 0;
    float scale = -30.0f / (M_PI * std::powf(gridRadius, 5.0f));
    float val = gridRadius - dst;
    return val * val * scale;
}

float Particle::nearPressureKernel(float dst) {
    const float gridRadius   = g_ParticleParameters.gridRadius;
    const float ooGridRadius = g_ParticleParameters.ooGridRadius;

    if (dst >= gridRadius) return 0;
    float scale = -3.0f * ooGridRadius;
    float val = 1 - dst * ooGridRadius;
    return val * val * scale;
}

float Particle::viscosityKernel(float dst) {
    const float gridRadius   = g_ParticleParameters.gridRadius;

    if (dst >= gridRadius) return 0;
    float scale = 40.0f / (M_PI * std::powf(gridRadius, 5.0f));
    float val = gridRadius - dst;
    return val * scale;
}

glm::vec3 Particle::pressure(int idx) {
    const float targetDensity          = g_ParticleParameters.targetDensity;
    const float pressureMultiplier     = g_ParticleParameters.pressureMultiplier;
    const float nearPressureMultiplier = g_ParticleParameters.nearPressureMultiplier;

    glm::vec3 force = glm::vec3(0.0f);
    std::vector <Particle> neighbors = findNeighbors(idx);

    for (int i = 0; i < neighbors.size(); ++i) {
        // TODO: each thread processes 64 neighbors

        float dst = glm::length(neighbors[i].pos - particles[idx].pos);
        if (dst < 1e-6f) continue;
        glm::vec3 dir = (neighbors[i].pos - particles[idx].pos) / dst;
        float dens = std::max(neighbors[i].density, 1e-4f);

        float influence = pressureKernel(dst);
        float nearInfluence = nearPressureKernel(dst);

        float pressureA = (neighbors[i].density - targetDensity) * pressureMultiplier;
        float pressureB = (particles[idx].density - targetDensity) * pressureMultiplier;

        float nearPressure = neighbors[i].nearDensity * nearPressureMultiplier;

        float sharedPressure = influence * (pressureA + pressureB) / (2.0f * dens);
        sharedPressure += nearInfluence * nearPressure;
        force += dir * sharedPressure;
    }

    return force + viscosity(idx, neighbors);
}

void Particle::calculateDensities(int idx) {
    float density = 0.0f;
    float nearDensity = 0.0f;
    Particle& p = particles[idx];
    std::vector <Particle> neighbors = findNeighbors(idx);
    for (int i = 0; i < neighbors.size(); i++) {
        if (i == idx) continue;
        float dst = glm::length(neighbors[i].predictedPos - particles[idx].predictedPos);
        density += densityKernel(dst);
        nearDensity += nearDensityKernel(dst);
    }
    p.density = density;
    p.nearDensity = nearDensity;
}

glm::vec3 Particle::viscosity(int idx, std::vector<Particle> neighbors) {
    const float viscosityMultiplier = g_ParticleParameters.viscosityMultiplier;

    glm::vec3 force = glm::vec3(0.0f);
    for (int i = 0; i < neighbors.size(); ++i) {
        float dst = glm::length(neighbors[i].pos - particles[idx].pos);
        if (dst < 1e-6f) continue;
        glm::vec3 dir = (neighbors[i].pos - particles[idx].pos) / dst;
        float influence = viscosityKernel(dst);
        force += (neighbors[i].velocity - particles[idx].velocity) * influence;
    }
    return force * viscosityMultiplier * particles[idx].density;
}

glm::vec3 velToColor(Particle p) {
    float speed = glm::length(p.velocity);
    float scale = std::min( speed / g_ParticleParameters.MAX_SPEED, 1.f ); // Clamp color to 0.0 .. 1.0
    glm::vec3 color = glm::vec3(0.0f);
    color.r = scale;
    color.g = 1.0f - std::abs(scale - 0.5f);
    color.b = 1.0f - scale;
    
    return color;
}

void Particle::drawElements(int object_Location, int color_Location, bool bDraw,int frame) {
    if (bDraw)
    {
        const int numSegments = g_ParticleParameters.numSegments;
        const int offset = centers.size() / 2 * (numSegments + 2);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * offset * sizeof(float), positions.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * offset * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
        glEnableVertexAttribArray(0);

        // Draw Loop
        for (int i = 0; i < particles.size(); ++i) {
            Particle& p = particles[i];
            glm::vec3 color = velToColor(p);

            glUniform4f(object_Location, p.pos.x, p.pos.y, 0.0, 0.0f);
            glUniform3f(color_Location, color.r, color.g, color.b);

            glDrawElements(GL_TRIANGLES, 3 * numSegments, GL_UNSIGNED_INT, (void*)(i * 3 * numSegments * sizeof(unsigned int)));
        }
    }
}

void Particle::updateElements(int object_Location, int color_Location) {
    const float stepSize     = g_ParticleParameters.stepSize;
    const float gravity      = g_ParticleParameters.GRAVITY_MAGNITUDE;
    const float maxSpeed     = g_ParticleParameters.MAX_SPEED;

    // change position and cell
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        int cellX, cellY;
        utilPositionToGridXY( p.pos, cellX, cellY );
        p.pos += stepSize * p.velocity;
        checkBoundary(p);
        updateCell(i, cellX, cellY);
    }

    // predict positions for density calculations
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        p.predictedPos = p.pos + stepSize * p.velocity;
    }
    
    // calculate densities
    for (int i = 0; i < particles.size(); ++i) {
        calculateDensities(i);
    }

    // apply pressure force
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        float dens = std::max(particles[i].density, 1e-4f);
        p.acceleration = pressure(i) / dens;
        p.acceleration.y -= gravity;
        p.velocity += stepSize * p.acceleration;
        float velMag = glm::length(p.velocity);
        // velocity clamp
        if (velMag > maxSpeed) p.velocity = maxSpeed * p.velocity / velMag;
    }
}
