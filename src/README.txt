HOMEWORK 3: RAY TRACING, RADIOSITY, & PHOTON MAPPING

NAME:  Mingyi Chen



ESTIMATE OF # OF HOURS SPENT ON THIS ASSIGNMENT:  30



COLLABORATORS AND OTHER RESOURCES: List the names of everyone you
talked to about this assignment and all of the resources (books,
online reference material, etc.) you consulted in completing this
assignment.

C++ reference
Fundamentals of Computer Graphics, 5th Edition (Steve Marschner, Peter Shirley)
A progressive refinement approach to fast radiosity image generation, ACM SIGGRAPH Computer Graphics, Volume 22, Issue 4, Aug. 1988, pp 75–84, https://doi.org/10.1145/378456.378487

Remember: Your implementation for this assignment must be done on your
own, as described in "Academic Integrity for Homework" handout.



OPERATING SYSTEM & VERSION & GRAPHICS CARD:
Windows 10,
AMD Radeon(TM) RX Vega 10 Graphics



SELF GRADING TOTAL:  [ 22.5 / 20 ]


< Please insert notes on the implementation, known bugs, extra credit
in each section. >



2 PROGRESS POSTS [ 5 / 5 ] Posted on Submitty Discussion Forum on the
dates specified on the calendar.  Includes short description of
status, and at least one image.  Reasonable progress has been made.


SPHERE INTERSECTIONS, SHADOWS, & REFLECTION [ 2 / 2 ]
  ./render -size 200 200 -input reflective_spheres.obj
  ./render -size 200 200 -input reflective_spheres.obj -num_bounces 1
  ./render -size 200 200 -input reflective_spheres.obj -num_bounces 3 -num_shadow_samples 1


RAY TREE VISUALIZATION OF SHADOW & REFLECTIVE RAYS [ 1 / 1 ]


DISTRIBUTION RAY TRACING: SOFT SHADOWS & ANTIALIASING [ 2 / 2 ]
  ./render -size 200 200 -input textured_plane_reflective_sphere.obj -num_bounces 1 -num_shadow_samples 1
  ./render -size 200 200 -input textured_plane_reflective_sphere.obj -num_bounces 1 -num_shadow_samples 4
  ./render -size 200 200 -input textured_plane_reflective_sphere.obj -num_bounces 1 -num_shadow_samples 9 -num_antialias_samples 9


EXTRA CREDIT: SAMPLING [ 1 ]
1 point for stratified sampling of pixel in image plane
1 point for stratified sampling of soft shadows
includes discussion of performance/quality
Stratified sampling of soft shadows implemented. Good quality and performance. The time it takes is very close to random sampling but produces less noise in the soft shadows.


OTHER DISTRIBUTION RAY TRACING EXTRA CREDIT [ 0 ]
glossy surfaces, motion blur, or depth of field, etc.


BASIC FORM FACTOR COMPUTATION [ 2 / 2 ]
Description of method in README.txt.
  ./render -size 200 200 -input cornell_box.obj


RADIOSITY SOLVER [ 3 / 3 ]
May be iterative (solution fades in) or done by inverting the form
factor matrix.
The solver is iterative.


FORM FACTORS WITH VISIBILITY / OCCLUSION RAY CASTING [ 0.5 / 1 ]
  ./render -size 300 150 -input l.obj
  ./render -size 300 150 -input l.obj -num_form_factor_samples 100
  ./render -size 300 150 -input l.obj -num_shadow_samples 1
  ./render -size 300 150 -input l.obj -num_form_factor_samples 10 -num_shadow_samples 1
  ./render -size 200 200 -input cornell_box_diffuse_sphere.obj -sphere_rasterization 16 12
  ./render -size 200 200 -input cornell_box_diffuse_sphere.obj -sphere_rasterization 16 12 -num_shadow_samples 1
See known bugs section.

RADIOSITY EXTRA CREDIT [ 0 ]
1 point for ambient term in radiosity
1-2 points for new test scene or visualization
1 point for writing the ray traced image to a file
1-3 points extra credit for performance improvements
1-3 points for other ray tracing effects
1-3 points for gradient or discontinuity meshing in radiosity


PHOTON DISTRIBUTION [ ?? / 2 ]
Shoots photons into the scene and the visualization looks reasonable
(the heart shape can be seen in the ring).
  ./render -size 200 200 -input reflective_ring.obj -num_photons_to_shoot 10000 -num_bounces 2 -num_shadow_samples 10
  ./render -size 200 200 -input reflective_ring.obj -num_photons_to_shoot 500000 -num_bounces 2 -num_shadow_samples 10 -num_antialias_samples 4


RAY TRACING WITH PHOTONS [ ?? / 2 ]
Searching for photons in the kdtree to use in ray tracing.  The
caustic ring is visible, and there are no significant artifacts in
illumination.


PHOTON MAPPING MATCHES RADIOSITY [ ?? ]
The intensity and color bleeding of photon mapping for indirect
illumination are correctly weighted and closely matches the results
from radiosity.  2 points extra credit.
  ./render -size 200 200 -input cornell_box_diffuse_sphere.obj -num_photons_to_shoot 500000 -num_shadow_samples 500 -num_photons_to_collect 500


OTHER EXTRA CREDIT [ 6 ]
1-2 points for new test scene or visualization
1 point for writing the ray traced image to a file
1-3 points extra credit for performance improvements
2-5 points for irradiance caching

2 points: New test scene: tea_table.obj (See Note 1)
1 point: Rendering to image file (See Note 2)
3 points: Performance improvements: Multithreading when writing to file

<Insert instructions for use and test cases and sample output as appropriate.>

Note 1: Sample Render
  A render is included as tea_table.png (in ./img). It is converted from an output ppm file to reduce file size. Render settings: size 640 360, 16 AA samples, 4 shadow samples, 4 bounces
Note 2: How To Render To File
  Press Ctrl + S in the main window. Follow the prompts in the standard output. You can hit Enter to select the default option. The program will hang during rendering.


KNOWN BUGS IN YOUR CODE
Please be concise!

The radiosity render of cornell_box_diffuse_sphere produces incorrect brightness.
