#include "../HeaderFiles/Common.h"

#include "../HeaderFiles/Shaders.h"
#include "../HeaderFiles/Particle.h"
#include "../HeaderFiles/Window.h"
#include <cmath>
#include <limits> // MAX_INT

       const char  *APP_NAME     = "Fluid Physics Simulation";
static const char  *APP_VERSION  = "Version 1.5";

// Configuration
static float  g_nAspectRatio        = 1.0f;
static bool   benchmark             = false;
static bool   createGridCenters     = true;
static bool   pauseAtStart          = false;
static bool   pauseAtEnd            = false;
static bool   simulationPaused      = false;
static bool   showGrid              = false;
static bool   verbose               = false;
static bool   vsync                 = true;
static int    numFirstRenderFrame   = 0;
static double numLastPhysicsSeconds = 0.0;
static int    width = 25;
static int    height = 20;

static double g_nElapsed            = 0.0;


enum SimulationState
{
     STATE_INIT
    ,STATE_WAITING_TO_RUN
    ,STATE_RUNNING
    ,STATE_END_STATS
    ,STATE_WAITING_TO_END
    ,STATE_DONE
};
SimulationState g_eSimulationState = STATE_INIT;

// Defining static variables
std::vector <float> g_WorldBoundary = {
    -0.9f,  0.9f, // Top Left
     0.9f,  0.9f, // Top Right
     0.9f, -0.9f, // Bot Right
    -0.9f, -0.9f  // Bot Left
};

void usage()
{
    const char *HELP =
"-?              Display command line options and quit.\n"
"--help          Alias for -?.\n"
"-benchmark      Run simulation for 3 minutes, never render, no vysnc. Equivalent to:\n"
"                    -render -1 -time 180 -vsync\n"
"-benchslow      Run simulation for 3 minutes (~10,800 frames @ 60fps), render first frame at frame number 7,200.\n"
"-benchfast      Run simulation for 10 seconds (~600 frames @ 60fps), render first frame at frame number 300.\n"
"-createcenter   Generate particles in grid centers.\n"
"-createrandom   Generate particles in random positions.\n"
"-h              Specifiy grid height (rows).\n"
"-height         Alias for -h.\n"
"-pause          Pause at both stand and end of simulation waiting for ENTER to be pressed.\n"
"-pausestart     Pause at start of simulation waiting for ENTER to be pressed.\n"
"-pauseend       Pause at end of simulation waiting for ENTER to be pressed.\n"
"-render #       Don't render until specified frame number. -1 is never render. (Default 0).\n"
"-showgrid       Hide neighbor grid. (Press G to toggle displaying the grid.)\n"
"+showgrid       Show neighbor grid.\n"
"-time   #.##    Run simulation for specified seconds.\n"
"-v              Verbose mode off (default).\n"
"+v              Verbose mode on.\n"
"-V              Display version and quit.\n"
"--version       Alias for -V.\n"
"-vsync          VSync off.\n"
"+vsync          VSync on (default).\n"
"-w              Specify grid width (columns).\n"
"-width          Alias for -w.\n"
    ;
#if USE_CPP_IOSTREAM
    std::cout << HELP;
#else
    printf( HELP );
#endif
}

void version()
{
#if USE_CPP_IOSTREAM
    std::cout
        << APP_NAME    << std::endl
        << APP_VERSION << std::endl;
#else
    printf( "%s\n%s\n", APP_NAME, APP_VERSION );
#endif
}

void parseCommandLine(int nArgs, const char* aArgs[])
{
    const char *pArg = nullptr;
    int         iArg = 1;

    while (iArg < nArgs)
    {
        pArg = aArgs[ iArg ];
        if (pArg[0] == '-')
        {
            if ((strcmp(pArg, "-?"    ) == 0)
            ||  (strcmp(pArg, "--help") == 0)) {
                usage();
                exit(0);
            }
            else
            if (strcmp(pArg, "-benchmark") == 0) {
                numFirstRenderFrame = INT_MAX; // never render
                numLastPhysicsSeconds = 3.0 * 60.0; // 3 min * 60 s/min = 180 seconds
                benchmark = true;
                vsync = false;
            }
            else
            if (strcmp(pArg, "-benchslow") == 0) {
                numFirstRenderFrame   = 2*60 * 60; // 2 min * 60 s/min * 60 frames/s = 7,200 frames
                numLastPhysicsSeconds = 3.0 * 60.0; // 3 min * 60 s/min = 180 seconds
                benchmark = true;
            }
            else
            if (strcmp(pArg, "-benchfast") == 0) {
                numFirstRenderFrame   = 5 * 60; // 5 s * 60 frames/s = 300 frames
                numLastPhysicsSeconds = 10.0; // 10 s
                benchmark = true;
            }
            else
            if (strcmp(pArg, "-createcenter"     ) == 0) {
                createGridCenters = true;
            }
            else
            if (strcmp(pArg, "-createrandom"     ) == 0) {
                createGridCenters = false;
            }
            else
            if ((strcmp(pArg, "-h"     ) == 0)
            ||  (strcmp(pArg, "-height") == 0)) {
                iArg++;
                if (iArg >= nArgs) {
                    const char *ERROR = "ERROR: Grid height not specified.\ni.e.\n    -height 25\n";
#if USE_CPP_IOSTREAM
                    std::cout << ERROR;
#else
                    printf( ERROR );
#endif
                    exit(1);
                }
                pArg = aArgs[ iArg ];
                height = atoi( pArg );
                if (height < 1)
                    height = 1;
            }
            else
            if (strcmp(pArg, "-pause") == 0) {
                pauseAtStart = true;
                pauseAtEnd = true;
            }
            else
            if (strcmp(pArg, "-pausestart") == 0) {
                pauseAtStart = true;
            }
            else
            if (strcmp(pArg, "-pauseend") == 0) {
                pauseAtEnd = true;
            }
            else
            if ((strcmp(pArg, "-w"    ) == 0)
            ||  (strcmp(pArg, "-width") == 0)) {
                iArg++;
                if (iArg >= nArgs) {
                    const char *ERROR = "ERROR: Grid width not specified.\ni.e.\n    -width 20\n";
#if USE_CPP_IOSTREAM
                    std::cout << ERROR;
#else
                    printf( ERROR );
#endif
                    exit(1);
                }
                pArg = aArgs[ iArg ];
                width = atoi( pArg );
                if (width < 1)
                    width = 1;
            }
            else
            if (strcmp(pArg, "-render") == 0) {
                iArg++;
                if (iArg >= nArgs) {
                    const char *ERROR = "ERROR: First frame to render was not specified.\ni.e.\n    -render 300\n";
#if USE_CPP_IOSTREAM
                    std::cout << ERROR;
#else
                    printf( ERROR );
#endif
                    exit(1);
                }
                pArg = aArgs[ iArg ];

                numFirstRenderFrame = atoi( pArg );
                if (numFirstRenderFrame < 0)
                    numFirstRenderFrame = INT_MAX;
            }
            else
            if (strcmp(pArg, "-showgrid") == 0) {
                showGrid = false;
            }
            else
            if (strcmp(pArg, "-time") == 0) {
                iArg++;
                if (iArg >= nArgs) {
                    const char *ERROR = "ERROR: Time to run simulation was not specified.\ni.e.\n    -time 10.0\n";
#if USE_CPP_IOSTREAM
                    std::cout << ERROR;
#else
                    printf( ERROR );
#endif
                    exit(1);
                }
                pArg = aArgs[ iArg ];

                numLastPhysicsSeconds = atof( pArg );
                if (numLastPhysicsSeconds < 0.0)
                    numLastPhysicsSeconds = 0.0;
            }
            else
            if (strcmp(pArg, "-v") == 0) {
                verbose = false;
            }
            else
            if (strcmp(pArg, "-V") == 0) {
                version();
                exit(0);
            }
            else
            if (strcmp(pArg, "-vsync") == 0) {
                vsync = false;
            }
            else
            if (strcmp(pArg, "--version") == 0) {
                version();
                exit(0);
            }
            else {
#if USE_CPP_IOSTREAM
                std::cout << "Warning: Skipping unknown argument: " << pArg << std::endl;
#else
                printf( "Warning: Skipping unknown argument: %s\n", pArg );
#endif
            }
        }
        else
        if (pArg[0] == '+')
        {
            if (strcmp(pArg, "+v") == 0) {
                verbose = true;
            }
            else
            if (strcmp(pArg, "+showgrid") == 0) {
                showGrid = true;
            }
            else
            if (strcmp(pArg, "+vsync") == 0) {
                vsync = true;
            }
        }

        iArg++;
    }
}

void updateTitleBarState(GLFWwindow* pWindow) {
    const SimulationState state = g_eSimulationState;
    const char *aPaused[2] =
    {
        "",
        " (PAUSED)"
    };
    char sTitle[128];

    if (state == STATE_WAITING_TO_RUN)
        sprintf( sTitle, "%s (PAUSED - Press ENTER to start)", APP_NAME);
    else
    if (state == STATE_WAITING_TO_END)
        sprintf( sTitle, "%s (PAUSED - Press ENTER to end)", APP_NAME);
    else
        sprintf( sTitle, "%s%s", APP_NAME, aPaused[simulationPaused]);
    glfwSetWindowTitle(pWindow, sTitle);
}

static void callbackInput(GLFWwindow* pWindow, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ENTER) {
            switch (g_eSimulationState) {
                case STATE_WAITING_TO_RUN:
                    g_eSimulationState = STATE_RUNNING;
                    simulationPaused = false;
                    updateTitleBarState(pWindow);
                    break;
                case STATE_WAITING_TO_END:
                    if( pauseAtEnd ) {
#if USE_CPP_IOSTREAM
                        std::cout << "Done.\n";
#else
                        printf( "Done.\n" );
#endif
                    }
                    g_eSimulationState = STATE_DONE;
                    break;
                default:
                    break;
            }
        }
        else
        if (key == 'G')
            showGrid = !showGrid;
        else
        if (key == 'R') {
            Particle::reset(g_nAspectRatio);
            g_nElapsed = 0.0;

            if (simulationPaused)
                g_eSimulationState = STATE_WAITING_TO_RUN;
            else
                g_eSimulationState = STATE_RUNNING;
            updateTitleBarState(pWindow);
        }
        else
        if (key == ' ') { // Can only toggle pause if the simulation is running, not waiting
            if (g_eSimulationState == STATE_RUNNING) {
                simulationPaused = !simulationPaused;
                updateTitleBarState(pWindow);
            }
        }
    }
}

void displayStats(const int numFrame) {
    double elapsed = g_nElapsed;
    if (elapsed < 1e-6)
        elapsed = 1e-6; // Alt.: std::numeric_limits<float>::infinity();

    double frames  = (double)numFrame; // frames
    double avgFPS  = frames / elapsed; // frames/second
    double avgFTms = (1.0 / avgFPS) * 1000.0; // ms
#if USE_CPP_IOSTREAM
    std::cout
        <<   "Total Frames: "  <<                                         numFrame << " "
        << "/ Total Elapsed: " << std::setw(7) << std::setprecision(3) << elapsed << " s "
        << "= Avg FPS: "       << std::setw(7) << std::setprecision(3) << avgFPS
        << ", Avg Frametime: " << std::setw(7) << std::setprecision(3) << avgFTms << " ms"
        << std::endl;
#else
    printf( "Total Frames: %d / Total Elapsed: %7.3f s = Avg FPS: %7.3f, Avg Frametime: %7.3f ms \n", numFrame, elapsed , avgFPS, avgFTms );

    #if PROFILE_NEIGHBORS
        printf( "Max neighbors: %d\n", g_nMaxNeighbors );
    #endif
#endif
}

void drawGrid(GLint object_Location, GLint color_Location) {
    static std::vector<float> gridCells =
    {
        -1.0f,  1.0f, // Top Left
         1.0f,  1.0f, // Top Right
         1.0f, -1.0f, // Bot Right
        -1.0f, -1.0f  // Bot Left
    };

    const int       nGridCell   = g_ParticleParameters.gridDim;
    const float     nGridRadius = g_ParticleParameters.gridRadius;
    const glm::vec3 vGridColor( 0.2f, 0.2f, 0.2f );
          glm::vec3 vGridColorOccupancy;

    for (int iGridCellY = 0; iGridCellY < nGridCell; iGridCellY++) {
        for (int iGridCellX = 0; iGridCellX < nGridCell; iGridCellX++) {
            float x = -1.0f + ((float)(iGridCellX) * nGridRadius);
            float y = -1.0f + ((float)(iGridCellY) * nGridRadius);
            const float x0 = x;
            const float x1 = x + nGridRadius;
            const float y0 = y;
            const float y1 = y + nGridRadius;

            gridCells[0] = x0; gridCells[1] = y1;
            gridCells[2] = x1; gridCells[3] = y1;
            gridCells[4] = x1; gridCells[5] = y0;
            gridCells[6] = x0; gridCells[7] = y0;
            Window::drawRectangle(object_Location, color_Location, &gridCells, &vGridColor );
        }
    }

#if PROFILE_OCCUPANCY
    static int nMaxOccupancy = 0;
           int nMaxOccupancyThisFrame = 0;
#endif

    const int nParticles = g_ParticleParameters.numOfParticles;// Alt. (int) Particle::particles.size();
    const int nGridCells = nGridCell * nGridCell;
    static std::vector<int> vGridOccupancy;

    if (vGridOccupancy.size() != nGridCells) {
        vGridOccupancy.clear();
        vGridOccupancy.reserve( nGridCells );
        std::fill_n( std::back_inserter( vGridOccupancy), nGridCells, 0 );
    }

    for( int iGridCell = 0; iGridCell < nGridCells; iGridCell++ ) {
        vGridOccupancy[ iGridCell ] = 0;
    }

    for (int iParticle = 0; iParticle < nParticles; iParticle++) {
        const Particle* pParticle = &Particle::particles[iParticle];
        int iGridCellX, iGridCellY;
        utilPositionToGridXY( pParticle->pos, iGridCellX, iGridCellY );
        int iGridCell = (iGridCellY* nGridCell) + iGridCellX;
        vGridOccupancy[ iGridCell ]++;

#if PROFILE_OCCUPANCY
        const int nGridOccupancy = vGridOccupancy[iGridCell];

        if( nMaxOccupancyThisFrame < nGridOccupancy)
            nMaxOccupancyThisFrame = nGridOccupancy;

        if (nMaxOccupancy < nGridOccupancy) {
            nMaxOccupancy = nGridOccupancy;
#if USE_CPP_IOSTREAM
        std::cout
            << "Frame #" << numFrame << ": "
            << "Cell" << iGridCellX << "," << iGridCellY << " = "
            << nMaxOccupancyThisFrame << "/"
            << nGridOccupancy << " particles" << std::endl;
#else
        printf( "Frame #%d: Cell[%d,%d] = %d/%d particles\n", numFrame, iGridCellX, iGridCellY, nMaxOccupancyThisFrame, nGridOccupancy );
#endif
        }
#endif
    }

    for( int iGridCell = 0; iGridCell < nGridCells; iGridCell++ ) {
        int nCells = vGridOccupancy[ iGridCell ];
        int iGridCellX = iGridCell % nGridCell;
        int iGridCellY = iGridCell / nGridCell;

        float x = -1.0f + ((float)(iGridCellX) * nGridRadius);
        float y = -1.0f + ((float)(iGridCellY) * nGridRadius);
        const float x0 = x;
        const float x1 = x + nGridRadius;
        const float y0 = y;
        const float y1 = y + nGridRadius;

        gridCells[0] = x0; gridCells[1] = y1;
        gridCells[2] = x1; gridCells[3] = y1;
        gridCells[4] = x1; gridCells[5] = y0;
        gridCells[6] = x0; gridCells[7] = y0;

#if SHOW_MAX_OCCUPANCY_ONLY
        if (nCells >= nMaxOccupancyThisFrame) {
#else
        if (nCells) {
#endif
            const float nMaxOccupancy = (float) 6;
            const float t = std::min( (float)nCells / nMaxOccupancy, 1.0f );
            utilColorMappingHotToCold( t, vGridColorOccupancy );
            Window::drawRectangle(object_Location, color_Location, &gridCells, &vGridColorOccupancy );
        }
    }
}

int main(int numArgs, const char *aArgs[])
{
    g_ParticleParameters.initParticleParameters();
    parseCommandLine( numArgs, aArgs );

    Window window(1600, 1000, vsync);
    GLFWwindow *pWindow = window.win;
    g_nAspectRatio = window.aspectRatio;

    // Generating Buffers
    glGenVertexArrays(1, &Window::vao);
    glGenBuffers(1, &Window::vbo);

    glGenVertexArrays(1, &Particle::vao);
    glGenBuffers(1, &Particle::vbo);
    glGenBuffers(1, &Particle::ibo);

    if( createGridCenters )
        Particle::generateGridCenters(height, width);
    else
        Particle::generateRandomCenters();
    Particle::populate(window.aspectRatio); // create particles using center positions

    // TODO: Create render grid

    // creating and compiling shaders
    Shader::shaderProgramSource source = Shader::parse("res/shaders/Basic.shader");
    unsigned int shader = Shader::create(source.vertexSource, source.fragmentSource);
    glUseProgram(shader);

    // Uniforms Declaration
    GLint color_Location = glGetUniformLocation(shader, "u_Color");
    glUniform3f(color_Location, 0.2f, 0.3f, 0.8f);

    GLint object_Location = glGetUniformLocation(shader, "u_pos");
    glUniform4f(object_Location, 0.0f, 0.0f, 0.0f, 0.0f);

    assert( color_Location != -1 );
    assert( object_Location != -1 );

    /* Loop until the user closes the window */

    static double lastTime              = 0.0f;
    static int    numFrame              = 0;

    char sFirstFrame[16] = "n/a";
    if (numFirstRenderFrame != INT_MAX)
        sprintf( sFirstFrame, "%d", numFirstRenderFrame );
    const size_t numParticles = Particle::particles.size();
    const size_t numCenters   = Particle::centersX.size();
    const int    gridDim      = g_ParticleParameters.gridDim;

#if USE_CPP_IOSTREAM
    std::cout.precision(6);
    std::cout
        << "Configuration: (C++ iostream)" << std::endl
        << std::fixed
        << "    First Render Frame: # " <<                                         sFirstFrame           << std::endl
        << "    Last Physics Seconds: " << std::setw(7) << std::setprecision(3) << numLastPhysicsSeconds << std::endl
        << "    Particles: "            <<                                         width << " x " << height << std::endl
        << "    Total particles: "      <<                                         numParticles          << std::endl
        << "    Centers: "              <<                                         numCenters            << std::endl
        << "    Grid dimensions: "      <<                                         gridDim               << std::endl;
#else
    printf( "Configuration: (C printf)\n" );
    printf( "    First Render Frame: # %s\n", sFirstFrame );
    printf( "    Last Physics Seconds: %7.3f\n", numLastPhysicsSeconds );
    printf( "    Particles: %d x %d\n", width, height );
    printf( "    Total particles: %llu\n", numParticles );
    printf( "    Centers: %llu\n", numCenters );
    printf( "    Grid dimensions: %d\n", gridDim );
#endif

    simulationPaused = pauseAtStart;
    if (pauseAtStart) {
        g_eSimulationState = STATE_WAITING_TO_RUN;
        updateTitleBarState(pWindow);
    }
    else
        g_eSimulationState = STATE_RUNNING;
    glfwSetKeyCallback(pWindow, callbackInput);

    while (!glfwWindowShouldClose(pWindow))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        bool bDraw = (numFrame >= numFirstRenderFrame);
        bool bRunning = (g_eSimulationState == STATE_RUNNING) && !simulationPaused;
        if (bRunning)
            Particle::updateParticles();
        if (bDraw)
            Particle::drawParticles(object_Location, color_Location);
        if (showGrid)
            drawGrid(object_Location, color_Location);
        Window::drawRectangle(object_Location, color_Location, &g_WorldBoundary);

        //calculate FPS
        double currentTime = glfwGetTime();
        if (bRunning) {
            numFrame++;
            double deltaTime = currentTime - lastTime;
                   g_nElapsed += deltaTime;

            if (verbose) {
    #if USE_CPP_IOSTREAM
                std::cout
                    << "FPS: "          << std::setw(7) << std::setprecision(3) << (1.f / deltaTime)
                    << " / Frametime: " << std::setw(7) << std::setprecision(3) << deltaTime * 1000.f << "ms"
                    << "  Frame #: "    << std::setw(7)                         << numFrame
                    << "  Elapsed: "    << std::setw(7) << std::setprecision(3) << g_nElapsed << " s"
                    << std::endl;
    #else
                printf( "FPS: %7.3f / Frametime: %7.3f ms  Frame #: %7d  Elapsed: %7.3f s\n", (1.f / deltaTime), deltaTime * 1000.f, numFrame, g_nElapsed );
    #endif
            }
        }
        lastTime = currentTime;

        /* Swap front and back buffers */
        glfwSwapBuffers(window.win);

        /* Poll for and process events */
        glfwPollEvents();
        if (numLastPhysicsSeconds > 0.0 && (g_nElapsed >= numLastPhysicsSeconds)) {
            switch (g_eSimulationState) {
                case STATE_RUNNING:
                    displayStats(numFrame);

                    if (pauseAtEnd) {
                        g_eSimulationState = STATE_WAITING_TO_END;
                        simulationPaused = true;
                        updateTitleBarState(pWindow);
                    }
                    else {
                        g_eSimulationState = STATE_DONE;
                        glfwSetWindowShouldClose(pWindow, 1);
                    }
                    break;
                case STATE_DONE:
                    glfwSetWindowShouldClose(pWindow, 1);
                    break;
                default:
                    break;
            }
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}
