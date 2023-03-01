HOMEWORK 3: RAY TRACING, RADIOSITY, & PHOTON MAPPING

NAME:  Mingyi Chen



ESTIMATE OF # OF HOURS SPENT ON THIS ASSIGNMENT:  < insert # hours >



COLLABORATORS AND OTHER RESOURCES: List the names of everyone you
talked to about this assignment and all of the resources (books,
online reference material, etc.) you consulted in completing this
assignment.

C++ reference
Fundamentals of Computer Graphics, 5th Edition (Steve Marschner, Peter Shirley)

Remember: Your implementation for this assignment must be done on your
own, as described in "Academic Integrity for Homework" handout.



OPERATING SYSTEM & VERSION & GRAPHICS CARD:  < insert info... >



SELF GRADING TOTAL:  [ < ?? > / 20 ]


< Please insert notes on the implementation, known bugs, extra credit
in each section. >



2 PROGRESS POSTS [ ?? / 5 ] Posted on Submitty Discussion Forum on the
dates specified on the calendar.  Includes short description of
status, and at least one image.  Reasonable progress has been made.


SPHERE INTERSECTIONS, SHADOWS, & REFLECTION [ ?? / 2 ]
  ./render -size 200 200 -input reflective_spheres.obj 
  ./render -size 200 200 -input reflective_spheres.obj -num_bounces 1
  ./render -size 200 200 -input reflective_spheres.obj -num_bounces 3 -num_shadow_samples 1 


RAY TREE VISUALIZATION OF SHADOW & REFLECTIVE RAYS [ ?? / 1 ]


DISTRIBUTION RAY TRACING: SOFT SHADOWS & ANTIALIASING [ ?? / 2 ]
  ./render -size 200 200 -input textured_plane_reflective_sphere.obj -num_bounces 1 -num_shadow_samples 1
  ./render -size 200 200 -input textured_plane_reflective_sphere.obj -num_bounces 1 -num_shadow_samples 4
  ./render -size 200 200 -input textured_plane_reflective_sphere.obj -num_bounces 1 -num_shadow_samples 9 -num_antialias_samples 9


EXTRA CREDIT: SAMPLING [ ?? ]
1 point for stratified sampling of pixel in image plane
1 point for stratified sampling of soft shadows
includes discussion of performance/quality


OTHER DISTRIBUTION RAY TRACING EXTRA CREDIT [ ?? ]
glossy surfaces, motion blur, or depth of field, etc.


BASIC FORM FACTOR COMPUTATION [ ?? / 2 ]
Description of method in README.txt.  
  ./render -size 200 200 -input cornell_box.obj


RADIOSITY SOLVER [ ?? / 3 ]
May be iterative (solution fades in) or done by inverting the form
factor matrix.


FORM FACTORS WITH VISIBILITY / OCCLUSION RAY CASTING [ ?? / 1 ]
  ./render -size 300 150 -input l.obj 
  ./render -size 300 150 -input l.obj -num_form_factor_samples 100
  ./render -size 300 150 -input l.obj -num_shadow_samples 1 
  ./render -size 300 150 -input l.obj -num_form_factor_samples 10 -num_shadow_samples 1 
  ./render -size 200 200 -input cornell_box_diffuse_sphere.obj -sphere_rasterization 16 12
  ./render -size 200 200 -input cornell_box_diffuse_sphere.obj -sphere_rasterization 16 12 -num_shadow_samples 1


RADIOSITY EXTRA CREDIT [ ?? ]
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


OTHER EXTRA CREDIT [ ?? ]
1-2 points for new test scene or visualization
1 point for writing the ray traced image to a file
1-3 points extra credit for performance improvements
2-5 points for irradiance caching

<Insert instructions for use and test cases and sample output as appropriate.>



KNOWN BUGS IN YOUR CODE
Please be concise!
