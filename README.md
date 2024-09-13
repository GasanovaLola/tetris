# Tetris Game

## Overview

This project is a classic implementation of the Tetris game, developed in C++ using the WinAPI 32. It features traditional Tetris mechanics such as random tetromino generation, grid-based movement, and line-clearing functionality.

## Features

- **Random Tetromino Generation**: Tetrominoes are randomly selected from the seven standard shapes.
- **Grid-Based Movement**: Tetrominoes move and rotate within a grid.
- **Line Clearing**: Complete lines are cleared from the grid when they are fully occupied.
- **Scoring System**: Points are awarded for each line cleared.
- **Game Over Condition**: The game ends when new tetrominoes cannot be placed on the grid.

## Requirements

- **Operating System**: Windows (WinAPI 32 specific)
- **Compiler**: Microsoft Visual Studio or any other C++ compiler supporting WinAPI 32
- **Libraries**: WinAPI 32 (included with Windows SDK)

## Installation

1. **Clone the Repository**:
    ```bash
    git clone (https://github.com/GasanovaLola/tetris.git)
    ```

2. **Open the Project**:
    - Open the project file (`.sln`) in Microsoft Visual Studio.

3. **Build the Project**:
    - Use the Build option in Visual Studio to compile the project.

4. **Run the Game**:
    - After building, you can run the executable from within Visual Studio or from the output directory.

## Controls

- **Arrow Keys**: Move tetrominoes left, right, and down.
- **Space Bar**: Rotate tetrominoes.
- **Esc Key**: Pause the game.
