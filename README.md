SearchVisualizer
================

A C++ program that visualizes the A* search algorithm, using a hexagonal grid. Users can draw starting nodes, goal nodes, and wall nodes by clicking and dragging their mouse, and watch the search as it progresses.

[Click here for a video demonstration](https://www.youtube.com/watch?v=qiZVRTDb_Ns)

Building
----------
To build the project, open Searchvisualizer.pro in Qt Creator, or run qmake on SearchVisualizer.pro to generate a makefile.

This project requires Qt 5.2 and a fully compliant C++11 compiler.

Usage
----------
When the program starts, you're presented with a grid of green tiles. Green is the "open" state.

Walls are dark blue. To draw a wall, hold the W key, click a cell, then drag with your mouse.
Goals are red. To draw a goal node, hold G, click a cell, then drag with your mouse.
Starting nodes are yellow. To draw a start node, hold S, click a cell, then drag with your mouse.

To erase anything, hold the O key, click a cell, then drag with your mouse.

To start the search, press enter or return.
To pause/unpuase the search press space.
To cancel the search, press backspace or delete.
To erase all the cells and revert to the intial state, press the escape key.

License
----------
[MIT License](http://opensource.org/licenses/MIT)

To facilitate inter-thread communication, this also uses the [CPPChannels](https://github.com/ejmahler/CPPChannels) project, which is also MIT licensed.
