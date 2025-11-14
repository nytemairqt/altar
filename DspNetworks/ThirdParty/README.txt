Place libraries in "src" folder:

Eigen (not eigen-3.4.0, the actual "Eigen" folder inside)
json
math_approx
RTNeural
RTNeural-NAM
rubberband

You also have to fix all nested includes manually eg:

#include <Eigen/Dense>

becomes:

#include "../../Eigen/Dense"