# Fractal Project (The Logistic Map)
## Overview

I stumbled across the logistic map and James Gleick's _Chaos_ online, and the first time I saw the math behind the logistic map I knew I had to try and recreate it for myself. The simplicity of how it's implemented in code is fascinating to me and I loved just playing around with the fractal and reading about all the places the logistic map shows up. For this project I knew I'd need to make use of a graphics framework, and since I was trying to challenge myself to make use of C++ more, I decided to go with an SFML project (Simple Fast Multimedia Library). Most of the challenge in this project ended up being in creating a space to display the points I wanted to, managing pixel spaces, world spaces, rendering only what I need, and the grid lines.

## Tools

I knew that if I wanted to use tools like matplotlib the project would be a lot easier than I wanted it to be. I used SFML, but made an attempt to use my own implementations of its features wherever I could.
From SFML I made use of

- Windowing
- Rendering points in pixel space
- Rendering lines in pixel space given 2 points
- Rendering fonts
- Keyboard and mouse input

Features like transforms and cameras I wanted to implement myself.
I also made use of Eigen again for linear algebra.

## The Future of the Project

The title implies a project that's a lot more general than just the logistic map, and I do have plans of doing more, but I don't think I'll continue off of this repo. I started to run into performance issues, which I'd have to fix by learning more involved techniques to display things, not just feeding SFML 10,000 points to figure out how to render. I'd need to do a lot more learning in a framework like SFML and really start to mold the project to whatever framework I'm using. But longer term I'd like to be building more projects inside of Vulkan, so that's where I'm going to be spending more of my time, and once I get more comfortable within the framework this project might see a much faster sequel with some better fractal renders.

# Screenshots
<img src="screenshots/1.5.png" alt="Alt Text" width="1000" height="1000">
<img src="screenshots/10.png" alt="Alt Text" width="1000" height="1000">
