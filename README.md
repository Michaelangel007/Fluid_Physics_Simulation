# Fluid_Physics_Simulation

![pic](pics/preview.png)

[Demo](https://youtu.be/dvEPeeAJQFo) showing reset and neighbor grid display.

Original code by Spleen0291. Optimized, QoL and cleanup by me.

# Optimizations

| Description         | ms/frame | Branch | % Faster |
|:--------------------|---------:|:-------|---------:|
| Original            | 4.312 ms | [cleanup_benchmark](https://github.com/Michaelangel007/Fluid_Physics_Simulation/tree/cleanup_benchmark)      |   0% |
| Particle Properties | 4.312 ms | [cleanup_particle](https://github.com/Michaelangel007/Fluid_Physics_Simulation/tree/cleanup_particle)        |   0% |
| Neighbor index      | 3.844 ms | fluid cleanup                                                                                                |  12% |
| Fixed Neighbor array| 1.329 ms | [fluid cleanup](https://github.com/Michaelangel007/Fluid_Physics_Simulation/tree/fluid_cleanup)              | 224% |
| Cache kernel scalars| 1.236 ms | [kernel_optimizations](https://github.com/Michaelangel007/Fluid_Physics_Simulation/tree/kernel_optimizations)| 248% |
| 1-pass find neighbors| 0.825 ms | [cleanup_spatial_partition](https://github.com/Michaelangel007/Fluid_Physics_Simulation/tree/cleanup_spatial_partition)| 422% |

# Command Line Options

The command line options can be displayed with `-?` or `--help`:

```
-?              Display command line options and quit.
--help          Alias for -?.
-benchmark      Run simulation for 3 minutes (~10,800 frames @ 60fps), render first frame at frame number 7,200.
-benchfast      Run simulation for 10 seconds (~600 frames @ 60fps), render first frame at frame number 300.
-createcenter   Generate particles in grid centers.
-createrandom   Generate particles in random positions.
-h              Specifiy grid height (rows).
-height         Alias for -h.
-pause          Pause at both stand and end of simulation waiting for ENTER to be pressed.
-pausestart     Pause at start of simulation waiting for ENTER to be pressed.
-pauseend       Pause at end of simulation waiting for ENTER to be pressed.
-render #       Don't render until specified frame number. -1 is never render. (Default 0).
-showgrid       Hide neighbor grid. (Press G to toggle displaying the grid.)
+showgrid       Show neighbor grid.
-time #.##      Run simulation for specified seconds.
-v              Verbose mode off (default).
+v              Verbose mode on.
-V              Display version and quit.
--version       Alias for -V.
-vsync          VSync off.
+vsync          VSync on (default).
-w              Specify grid width (columns).
-width          Alias for -w.
```

# Hotkeys

* `ESC` to quit the simulation.
* `ENTER` or `SPACE` to start the simulation if paused on start. The title bar will show the status.
* `SPACE` to toggle pause/running.
* `.` to single step the simulation forward one frame.
* `G` to toggle display of the (neighbor spatial) grid.
* `R` to reset the simulation.

# Example Command-Line

`run -time 10 -pause`

# Benchmarking

There are three benchmark modes:

* `-benchmark`
* `-benchfast`
* `-benchslow`

| Command | Rendering starts at frame # | Simulation ends at time | VSync |
|:-------------|------:|-----------:|----:|
| `-benchmark` |   n/a | 3 minutes  | Off |
| `-benchfast` |   300 | 10 seconds | On |
| `-benchslow` | 7,200 |  3 minutes | On |

The command line `-benchmark` is equivalent to `-render -1 -time 180 -vsync`.

# Cleanup and Optimization History

Someone asked for help on reddit why their fluid sim was slow. I decided to take a look since the codebase was relatively small.

* First, I needed a way to run the benchmark for a fixed amount of time.
  * Added command-line option: `-time #.#`.
* Next, I needed a way to skip rendering for the first N frames.
  * Added command-line option: `-render #`.
* I added a summary of Total frames, Total elapsed, Average FPS, and Average frametime.
* I needed a way to turn off VSync so we can run "flat-out" and not worry about rendering time.
  * Added command-line option: `-vsync`.
* Added a way to turn on VSync for completeness.
  * Added command-line option: `+vsync`.
* Added `-render -1` to keep rendering permanently disabled.
* Split up rendering and updating into `drawElements()` and `updateElements()` respectively.
* `Particle` is a "fat" class that does three things:
  * Particle data,
  * Simulation Properties,
  * Rendering data.
* I moved most of the simulation properties to `ParticleParameters`. No change in performance as expected.
* Looking at `findNeighbors `I then looked at the maximum number of neighbors returned via `PROFILE_NEIGHBORS`. This was 64 which means a LOT of temporary copies of Particles are being returned!
* Replaced the `std::vector<particle>` with a typedef for `Neighbor` and fixed up the `findNeighbors()` and `viscosity()` API.  This allows us to re-factor the underlying implementation for Neighbor without breaking too much code.
* Added a define `USE_NEIGHBORS_INDEX` to replace Neighbors with `typedef std::vector<int16_t> Neighbors;` With some minor cleanup `const Particle neighbor = particles[neighbors[iNeighbor]]` that brought the average frame time down to 3.8 ms. Not much but it was a start.
* Seeing a LOT of temporary copies I switched from a dynamic vector to a static array for neighbors.
* Added a define `USE_FIXED_NEIGHBORS_SIZE` and added a `std::vector` replacement I called `Neighbors` that has `size()` and `push_back()` functions along with `[]` array overloading so it is API compatible with std::vector. This brought the average frame time down to 1.3 ms
* Continued cleanup by splitting `centers` vector into `centersX` and `centersY`. This lets us get rid of a few `centers.size() / 2` shenanigans.
* Some QoL were long overdue.
  * Added ability to display the spatial neighbor grid cells.
  * Added command-line option: `+showgrid` and `-showgrid`
  * Added `G` key to toggle this at run-time.
  * Added ability to pause/unpause the simulation.
  * Added ability to reset the simulation via `R`.
* There are a bunch of kernel functions that calculate various scale factors based on gridRadius but this is a constant even though it wasn't declared `const`!
* In `densityKernel()` the far density scale is constantly being recalculated even though this is a constant:
  * This gives us a 1.329 ms -> 1.287 ms speedup, another 3.2% faster.

Before:
```c++
    float scale = 4.0f / (M_PI * std::powf(gridRadius, 8.0f));
    :
    return val * val * val * scale;
```

After:
```c++
    const float scale  = g_ParticleParameters.farDensityScale;
    :
    return val * val * val * scale;
```


* Likewise, in `pressureKernel()` the  far pressure scale is constantly being recalculated even though this is a constant:
  * This gives us 1.287ms -> 1.264 ms speedup, another 1.8% faster.

Before:

```c++
    float scale = -30.0f / (M_PI * std::powf(gridRadius, 5.0f));
    :
    return val * val * scale;
```

After:
```c++
    const float scale        = g_ParticleParameters.farPressureScale;
    :
    return val * val * scale;
```


* And again, in `viscosityKernel()` the far viscosity scale is constantly being recalculated even though this is a constant:
  * This gives us 1.264ms -> 1.232 ms speedup, another 2.5% faster.

Before:
```c++
    float scale = 40.0f / (M_PI * std::powf(gridRadius, 5.0f));
    :
    return val * scale;
```

After:
```c++
    const float scale  = g_ParticleParameters.viscosityScale;
    :
    return val * scale;
```

We are only single-threaded (!) so there is still performance on the table!
