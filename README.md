Introduction
------------
![Sound and Vision](https://raw.githubusercontent.com/peter-juritz/flying-blind/master/images/vision-movement.png)

This project was written as a visual experiment while in university. The goal of the project was to see how well people could navigate a three dimensional environment using only sound. This was built using C++, OpenGL and the irrKlang 3d sound library. Additionally I wrote some code to do some [procedural terrain generation](https://en.wikipedia.org/wiki/Fractal_landscape).

To answer this question I built an application which could read in various “maps” (defined as PGM files) and render these as well as generate sound to help the user navigate the environment. Results are displayed at the bottom but in summary, it turned out that some participants could navigate the environment quite well! You can find traces of all the participatns movements in the participants directory.

Sound Generation
---------------
All the non-video participants played the game while blindfolded. The system would generate audio to help them navigate.
Two rays were cast from the user’s position at 30 degrees from forward to the left as well as to the right, if they collided with an obstable a ping would be heard from the direction of the obstacle. These acted as a kind of radar wave or sonar ping When the user was approaching an obstacle on the right a ping would be heard in the right ear. As the user got more close the ping would be repeated more frequently. If the obstacle was directly in front a sound would be heard in both ears.

I also wrote code to generate a pitch at different frequencies depending on the users height but this turned out to not be very helpful.
![Sound navigation](https://raw.githubusercontent.com/peter-juritz/flying-blind/master/images/sound-movement.png).

The participant could then attempt to navigate a number of different maps, which can be found in the maps directory. The position of the user was then recorded at each frame. The data for the participants can be found in the participants directory.

Maps and Terrain Generation
---------------------------
All maps were defined as PGM files which acted as heightmaps. I implemented functionality to generate terrain heightmaps using both the [diamond square alogrithm](https://en.wikipedia.org/wiki/Diamond-square_algorithm) as well as the [fault line method](http://www.lighthouse3d.com/opengl/terrain/index.php?fault).

![Procedural terrain](https://raw.githubusercontent.com/peter-juritz/flying-blind/master/images/bump.png).

These maps where rendered using some pretty ghetto OpenGL (lots of glBegin() glEnd()). I also implemented an extremely simple [level of detail](https://en.wikipedia.org/wiki/Level_of_detail) system which would render the terrain at  a higher poly count near to the user. All in all the graphics code could be far more efficient (index buffers and such) - but this was a brief project intended mainly to understand navigation with sound.

Usage
-----
To build the code run:
```
make
```
then to begin in a map run one of the map scripts (the .sh files in the root) followed by a participant name.
eg:
```
./bump.sh peter
```
Your positions will then be recorded to DATA/something-peter.OUTPUT

keyboard controls
```
w a s d : turn left,right. move up down
h: speed up
b: slow down
y: generate a new landscape using diamond square
u: generate a new landscape using fault line method
g: Save current landsapce to “out.pgm”
```

Images and results
------
![Bump Map](https://raw.githubusercontent.com/peter-juritz/flying-blind/master/images/bump-map.png)
![Vision Movement ](https://raw.githubusercontent.com/peter-juritz/flying-blind/master/images/vision-movement-2.png)
![Sound Movement](https://raw.githubusercontent.com/peter-juritz/flying-blind/master/images/sound-movement-2.png)
![Spiral](https://raw.githubusercontent.com/peter-juritz/flying-blind/master/images/spiral.png)

Observations from the experiment
--------------------------------
* A participant (using only sound) said they felt they had traveled much further than they did (after seeing the map in 3d).
* As pings got more intense people panicked.
* Sound in both ears (meaning an obstacle right ahead) made people panic.
* Silence for a long time made people quite scared, and they would even start making preemptive turns even though no beeps were heard.
* People who played with vision almost never got close enough to obstacles to hear a beep and never “crashed”.

![Square Spiral](https://raw.githubusercontent.com/peter-juritz/flying-blind/master/images/square-spiral.png)
