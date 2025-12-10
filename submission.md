## Project 6: Final Project Gear Up

The project handout can be found [here](https://cs1230.graphics/projects/final/gear-up).

## Test Cases

**NOTE: For most of these involving motion I hard-coded a path for the camera to move on, the implementation does not meet the requirements for full camera-path tracing (will be done for project 7).**



### Depth Buffers

*For each of these cases, I used a shader to display Depth visualized linearly proportional to the near/far plane*

Test 1: Empty Scene
Expected result: Black screen



Test 2: Looking into open space
Expected result: Also a black screen 



Test 3: Small distance away from surface
Expected result: Mostly white, some grayer outside of close surface



Test 4: In motion - lower far plane
Expected result: Depth map should change based on updated position


Test 5: In motion - higher far plane
Expected result: Depth map should change based on updated position, more contrast than previous test.



Test 6: Other types of shapes (using primitive salad)
Expected result: Consistent with cubes


i
### Geometry Buffers (for motion blur)



Test 1: Position visualization

Test 2: Position visualization
Expected result, color boundaries between axis and different colors for pits


Test 3: Normal visualization



### Sceen Space Motion Blur

Test 1: 






#### Extra test: Recursive sphere traversal (ngl just looked cool)



### Rigidbody Translation

For this project we attached the camera to a rigidbody element in the scene that has a toggle for gravity and a 


#### Test 1: Simple jump up and down


#### Test 2: Walking on a flat surface


#### Test 3: Walking and falling from a block


#### Test 4: Falling and intersecting a block at a high height (building up a lot of speed)


#### Test 5: Switching between flying/gravity mode


#### Bonus Test: Enemy generation 


## Design Choices

Uhhh there's a lot fo 


## Collaboration/References

Most of the written code for the features mirrored similar processes outlined the LearnGL tutorials listed on the handout. I also used ChatGPT when debugging my motion blur frambuffer operations and for setting up the mapbuilder widget (not part of feature list).

I also used chatgpt 

## Known Bugs

Not sure if this is entirely the 

## Extra Credit


