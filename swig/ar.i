/* module */
%module ar
%include "carrays.i"
%include "stdint.i"
%include "typemaps.i"
%include "cdata.i"

%array_class(float, float_array);

/* C wrapper */
%{
#include "ar.h"
%}

/* */
void AutoRegressionPy(float *inputseries, int length, int degree,
                      float *coefficients, int method);
