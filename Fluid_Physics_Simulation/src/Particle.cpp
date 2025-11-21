#include "../HeaderFiles/Particle.h"

#if PROFILE_NEIGHBORS
    int g_nMaxNeighbors = 0;
#endif

ParticleParameters g_ParticleParameters;

//Defining static members
std::vector <float>        Particle::centersX;
std::vector <float>        Particle::centersY;
std::vector <float>        Particle::positions;
std::vector <unsigned int> Particle::indices;
std::vector <Particle>     Particle::particles;

// Spatial Partition
std::vector <GridCol>    Particle::vSpatialPartitionGridCells(1,GridCol(1)); // [x][y][idx] = true/false
std::vector< Neighbors > Particle::vSpatialPartitionNeighbors;

unsigned int Particle::vao = 0;
unsigned int Particle::vbo = 0;
unsigned int Particle::ibo = 0;

static glm::vec3 utilVelocityToColor(const Particle& p) {
    float speed = glm::length(p.velocity);
    float scale = std::min( speed / g_ParticleParameters.MAX_SPEED, 1.f ); // Clamp color to 0.0 .. 1.0
    glm::vec3 color = glm::vec3(0.0f);
    utilColorMapping( scale, color );
    return color;
}

glm::vec3 Particle::calculatePressure(int idx) {
    const float targetDensity          = g_ParticleParameters.targetDensity;
    const float pressureMultiplier     = g_ParticleParameters.pressureMultiplier;
    const float nearPressureMultiplier = g_ParticleParameters.nearPressureMultiplier;

    const glm::vec3 homePos    (particles[idx].pos    );
    const float     homeDensity(particles[idx].density);

    glm::vec3 force = glm::vec3(0.0f);
    const Neighbors& neighbors = Particle::vSpatialPartitionNeighbors[idx];

    for (int iNeighbor = 0; iNeighbor < neighbors.size(); ++iNeighbor) {
        // TODO: Since we only have max 64 neighbors we should probably multi-thread the particles not the neighbors
        const int jNeighbor = neighbors[iNeighbor];
        const Particle neighbor = particles[jNeighbor];
        const glm::vec3 neighborPos         = neighbor.pos;
        const float     neighborDensity     = neighbor.density;
        const float     neighborNearDensity = neighbor.nearDensity;
        const glm::vec3 delta(neighborPos - homePos);

        float dst = glm::length(delta);
        if (dst < 1e-6f) continue;
        glm::vec3 dir = delta / dst;
        float dens = std::max(neighborDensity, 1e-4f);

        float influence = kernelFarPressure(dst);
        float nearInfluence = kernelNearPressure(dst);

        float pressureA = (neighborDensity - targetDensity) * pressureMultiplier;
        float pressureB = (homeDensity     - targetDensity) * pressureMultiplier;

        float nearPressure = neighborNearDensity * nearPressureMultiplier;

        float sharedPressure = influence * (pressureA + pressureB) / (2.0f * dens);
        sharedPressure += nearInfluence * nearPressure;
        force += dir * sharedPressure;
    }
    return force + calculateViscosity(idx);
}

glm::vec3 Particle::calculateViscosity(int iParticle) {
    const float viscosityMultiplier = g_ParticleParameters.viscosityMultiplier;
    const glm::vec3 homeVel = particles[iParticle].velocity;

    glm::vec3 force = glm::vec3(0.0f);
    const Neighbors& neighbors = vSpatialPartitionNeighbors[ iParticle ];

    for (int iNeighbor = 0; iNeighbor < neighbors.size(); ++iNeighbor) {
        const int jNeighbor = neighbors[iNeighbor];
        const Particle neighbor = particles[jNeighbor];
        const glm::vec3 neighborPos = neighbor.pos;
        const glm::vec3 neighborVel = neighbor.velocity;

        const glm::vec3 delta(neighborPos - particles[iParticle].pos);
        float dst = glm::length(delta);
        if (dst < 1e-6f) continue;
        glm::vec3 dir = delta / dst;
        float influence = kernelViscosity(dst);
        force += (neighborVel - homeVel) * influence;
    }

    return force * viscosityMultiplier * particles[iParticle].density;
}

void Particle::drawParticles(int object_Location, int color_Location) {
    const int numSegments  = g_ParticleParameters.numSegments;
    const int numParticles = (int)particles.size();
    const int offset       = numParticles * (numSegments + 2);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 2 * offset * sizeof(float), positions.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * offset * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    glEnableVertexAttribArray(0);

    // Draw Loop
    for (int i = 0; i < numParticles; ++i) {
        Particle& p = particles[i];
        glm::vec3 color = utilVelocityToColor(p);

        glUniform4f(object_Location, p.pos.x, p.pos.y, 0.0, 0.0f);
        glUniform3f(color_Location, color.r, color.g, color.b);

        glDrawElements(GL_TRIANGLES, 3 * numSegments, GL_UNSIGNED_INT, (void*)(i * 3 * numSegments * sizeof(unsigned int)));
    }
}

// See:
// * generateRandomCenters()
// * generateGridCenters()
void Particle::generateGridCenters(int gridRows, int gridCols) {
    const float space1 =     g_ParticleParameters.radius + g_ParticleParameters.spacing;
    const float space2 = 2 * g_ParticleParameters.radius + g_ParticleParameters.spacing;
    const float y0     =     g_ParticleParameters.vWorldVertices[1]; // Top

    float left = 0.0f - space2 * gridCols / 2.0f;
    float top  = y0   - space1;
    for (int y = 0; y < gridRows; y++) {
        for (int x = 0; x < gridCols; x++) {
            Particle::centersX.push_back(left + x*space2);
            Particle::centersY.push_back(top);
        }
        top -= space2;
    }
    g_ParticleParameters.numOfParticles = (int)Particle::centersX.size();
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

// See:
// * generateRandomCenters()
// * generateGridCenters()
void Particle::generateRandomCenters() {
    const int   numParticles = g_ParticleParameters.numOfParticles;
    const float nMin         = g_ParticleParameters.vCollisionMinMax.x;
    const float nMax         = g_ParticleParameters.vCollisionMinMax.y;

    for (int iParticle = 0; iParticle < numParticles; iParticle++) {
        Particle::centersX.push_back(glm::linearRand(nMin, nMax));
        Particle::centersY.push_back(glm::linearRand(nMin, nMax));
    }
}

float Particle::kernelFarDensity(float dst) {
    const float gridRadius = g_ParticleParameters.gridRadius;
    const float scale      = g_ParticleParameters.farDensityScale;

    if (dst >= gridRadius) return 0.f;
    float val = gridRadius*gridRadius - dst*dst;
    return val * val * val * scale;
}

float Particle::kernelFarPressure(float dst) {
    const float gridRadius   = g_ParticleParameters.gridRadius;
    const float scale        = g_ParticleParameters.farPressureScale;

    if (dst >= gridRadius) return 0.f;
    float val = gridRadius - dst;
    return val * val * scale;
}

float Particle::kernelNearDensity(float dst) {
    const float gridRadius   = g_ParticleParameters.gridRadius;
    const float ooGridRadius = g_ParticleParameters.ooGridRadius;

    if (dst >= gridRadius) return 0.f;
    float val = 1 - dst * ooGridRadius;
    return val * val * val;
}

float Particle::kernelNearPressure(float dst) {
    const float gridRadius   = g_ParticleParameters.gridRadius;
    const float ooGridRadius = g_ParticleParameters.ooGridRadius;

    if (dst >= gridRadius) return 0.f;
    float scale = -3.0f * ooGridRadius;
    float val = 1 - dst * ooGridRadius;
    return val * val * scale;
}

float Particle::kernelViscosity(float dst) {
    const float gridRadius  = g_ParticleParameters.gridRadius;
    const float scale       = g_ParticleParameters.viscosityScale;

    if (dst >= gridRadius) return 0;
    float val = gridRadius - dst;
    return val * scale;
}

// Read vector centers create particles
void Particle::populate(float aspectRatio) {
    const int gridDim    = g_ParticleParameters.gridDim;
    const int numCenters = g_ParticleParameters.numOfParticles;

    particles.clear();
    initSpatialPartition( numCenters, gridDim );

    // generating Centers
    for (int iCenter = 0; iCenter < numCenters; iCenter++) {
        Particle p;
        p.velocity = glm::vec3(0.0f);
        p.acceleration = glm::vec3(0.0f);
        p.pos = glm::vec3(centersX[iCenter], centersY[iCenter], 0.0f);
        p.density = 0.0f;
        p.generateParticle(aspectRatio);
        particles.push_back(p);

        // populating cells
        addPositionToGrid( iCenter );
    }
}

void Particle::reset(float aspectRatio) {
    populate(aspectRatio);
}

void Particle::updateBoundary() {
    const float nMin = g_ParticleParameters.vCollisionMinMax.x;
    const float nMax = g_ParticleParameters.vCollisionMinMax.y;

         if (pos.x < nMin) pos.x = nMin, velocity.x = -velocity.x * 0.5f;
    else if (pos.x > nMax) pos.x = nMax, velocity.x = -velocity.x * 0.5f;

         if (pos.y > nMax) pos.y = nMax, velocity.y = -velocity.y * 0.5f;
    else if (pos.y < nMin) pos.y = nMin, velocity.y = -velocity.y * 0.5f;
}

void Particle::updateDensities(int idx) {
    const glm::vec3 homePredictedPos = particles[idx].predictedPos;

    float density = 0.0f;
    float nearDensity = 0.0f;
    Particle& p = particles[idx];

    const Neighbors& neighbors = vSpatialPartitionNeighbors[ idx ];
    for (int iNeighbor = 0; iNeighbor < neighbors.size(); iNeighbor++) {
//        if (iNeighbor == idx) continue; // BUG? Why ignore first neighbor??

        const int jNeighbor = neighbors[iNeighbor];
        const Particle neighbor = particles[jNeighbor];
        const glm::vec3 neighborPredictedPos = neighbor.predictedPos;

        float dst = glm::length(neighborPredictedPos - homePredictedPos);
        density += kernelFarDensity(dst);
        nearDensity += kernelNearDensity(dst);
    }
    p.density = density;
    p.nearDensity = nearDensity;
}

void Particle::updateParticles() {
    const float stepSize     = g_ParticleParameters.stepSize;
    const float gravity      = g_ParticleParameters.GRAVITY_MAGNITUDE;
    const float maxSpeed     = g_ParticleParameters.MAX_SPEED;

    // change position and cell
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        int cellX, cellY;
        utilPositionToGridXY( p.pos, cellX, cellY );
        p.pos += stepSize * p.velocity;
        p.updateBoundary();
        updateCell(i, cellX, cellY);
    }

    // predict positions for density calculations
#pragma omp parallel for
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        p.predictedPos = p.pos + stepSize * p.velocity;
        updateNeighbors(i);
    }

    // calculate densities
#pragma omp parallel for
    for (int i = 0; i < particles.size(); ++i) {
        updateDensities(i); // findNeighbors() -> getNeighbors
    }

    // apply pressure force
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        float dens = std::max(particles[i].density, 1e-4f);
        p.acceleration = calculatePressure(i) / dens; // findNeighbors() -> getNeighbors()
        p.acceleration.y -= gravity;
        p.velocity += stepSize * p.acceleration;
        float velMag = glm::length(p.velocity);
        // velocity clamp
        if (velMag > maxSpeed) p.velocity = maxSpeed * p.velocity / velMag;
    }
}

// Spatial Partitioning
void Particle::addPositionToGrid(const int iParticle) {
    const glm::vec3 vPosition = particles[iParticle].pos;
    int cellX, cellY;
    utilPositionToGridXY( vPosition, cellX, cellY );
    vSpatialPartitionGridCells[cellX][cellY][iParticle] = true;
}

void Particle::initSpatialPartition(const int nParticles, const int nGridDim) {
    std::vector <GridCol> grid(nGridDim, GridCol(nGridDim)); // cells[x][y][idx]
    vSpatialPartitionGridCells = grid;

    vSpatialPartitionNeighbors.reserve(nParticles);
    for (int iParticle = 0; iParticle < (int)nParticles; iParticle++) {
        Neighbors neighbor;
        vSpatialPartitionNeighbors.push_back( neighbor );
    }
}

void Particle::updateNeighbors(const int iParticle) {
    Particle& p = particles[iParticle];
    int cellX, cellY;
    utilPositionToGridXY( p.pos, cellX, cellY );

    Neighbors& neighborsOut = vSpatialPartitionNeighbors[iParticle];
    neighborsOut.clear();

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (std::pair<int, bool> neighbor : vSpatialPartitionGridCells[cellX + i][cellY + j]) {
                if (neighbor.first != iParticle && neighbor.second)
                    neighborsOut.push_back( neighbor.first & 0xFFFF );
            }
        }
    }
}

void Particle::updateCell(const int iParticle, const int iPrevCol, const int iPrevRow) {
    vSpatialPartitionGridCells[iPrevCol][iPrevRow][iParticle] = false;
    addPositionToGrid( iParticle );
}
