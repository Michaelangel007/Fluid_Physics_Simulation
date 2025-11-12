# Fluid_Physics_Simulation

YouTube link for a demonstration video: [https://www.youtube.com/watch?v=iZVx1ZC-qsQ](https://www.youtube.com/watch?v=iZVx1ZC-qsQ)

# Optimizations

| Description         | Timing | Branch | % Faster |
|:--------------------|-------:|:-------|---------:|
| Original            | 4.3 ms | [cleanup_benchmark](https://github.com/Michaelangel007/Fluid_Physics_Simulation/tree/cleanup_benchmark) |   0% |
| Particle Properties | 4.3 ms | [cleanup_particle](https://github.com/Michaelangel007/Fluid_Physics_Simulation/tree/cleanup_particle)   |   0% |
| Neighbor index      | 3.8 ms | fluid cleanup                                                                                           |  13% |
| Fixed Neighbor array| 1.3 ms | [fluid cleanup](https://github.com/Michaelangel007/Fluid_Physics_Simulation/tree/fluid_cleanup)         | 230% |

# Command Line Options

The command line options can be displayed with `-?` or `--help`:

```
-?              Display command line options and quit.
--help          Alias for -?.
-benchmark      Run simulation for 3 minutes (~10,800 frames @ 60fps), render first frame at frame number 7,200.
-benchfast      Run simulation for 10 seconds (~600 frames @ 60fps), render first frame at frame number 300.
-h              Specifiy grid height (rows).
-height         Alias for -h.
-pause          Pause at end of simulation waiting for RETURN.
-render #       Don't render until specified frame number. -1 is never render. (Default 0).
-time   #.##    Run simulation for specified seconds.
-v              Verbose mode off (default).
+v              Verbose mode on.
-V              Display version and quit.
--version       Alias for -V.
-vsync          VSync off.
+vsync          VSync on (default).
-w              Specify grid width (columns).
-width          Alias for -w.
```

# Benchmarking

There are two benchmark modes:

* `-benchmark`
* `-benchfast`

| Command | Rendering starts at frame # | Simulation ends at time |
|:-------------|------:|-----------:|
| `-benchfast` |   300 | 10 seconds |
| `-benchmark` | 7,200 |  3 minutes |

I used `-render -1 -time 180 -vsync` for benchmarking without rendering.
