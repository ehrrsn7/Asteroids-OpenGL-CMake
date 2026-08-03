# CS 165 - Asteroids Project Instructions

## Overview

You will create a simplified Asteroids game using C++ and OpenGL. Your program must follow the object-oriented design expectations of this course.

## Project setup

Use the README for setup instructions. The README contains the required steps for installing the tools, configuring CMake, and building the project.

## Required game behavior

Your program must include the following core game features:

- A ship that can rotate and move.
- Bullets that are fired from the ship.
- Asteroids that move and rotate.
- Wrapping behavior so objects reappear on the opposite side of the screen.
- A game loop that updates and draws the scene each frame.

## Required object-oriented design

This project is not just a coding exercise. It is a final demonstration of object-oriented programming.

Your final solution must include:

1. A Game class that manages the overall game state.
2. A Ship class that represents the player-controlled ship.
3. A Bullet class that represents bullets fired by the ship.
4. A Rock hierarchy (or equivalent asteroid classes) that uses inheritance and polymorphism.
5. Encapsulation of data and behavior inside classes.
6. Clear separation between input handling, game updates, and rendering.

## Required implementation expectations

Your submission must show that you understand the following concepts:

- classes and objects
- member variables and methods
- encapsulation
- inheritance
- polymorphism
- responsibility-driven design

Your code should not be a loose collection of functions. The game objects should be modeled as classes with clear responsibilities.

## Minimum acceptable outcome

Your project is not complete unless it demonstrates all of the following:

- The program builds successfully.
- The game window opens.
- The ship can rotate and move.
- Bullets can be fired.
- Asteroids appear and move.
- The game uses multiple classes with clear object-oriented structure.

## Recommended structure

You should organize your code around the following ideas:

- The Game class manages the world and the game loop.
- The Ship class handles ship movement and orientation.
- The Bullet class handles bullet movement and lifespan.
- The Rock classes handle asteroid movement, rotation, and breaking apart.
- The drawing code should remain separate from the game logic.

## What to submit

Submit a final project that shows thoughtful OOP design, clear class responsibilities, and a working game implementation.
