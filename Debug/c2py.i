/* module */
%module c2py
%include "carrays.i"
%include "stdint.i"
%include "typemaps.i"
%include "cdata.i"

// %array_class(double, double_array);
// %array_class(int16_t, int16_t_array); // 将int16_t_array数组的元素定义为int16_t
// Grab a 3 element array as a Python 3-tuple
%typemap(in) int16_t[3](const int16_t temp[3]) {   // temp[4] becomes a local variable
  int i;
  if (PyTuple_Check($input)) {
    if (!PyArg_ParseTuple($input, "hhh", temp, temp+1, temp+2)) {
      PyErr_SetString(PyExc_TypeError, "tuple must have 3 elements");
      SWIG_fail;
    }
    $1 = &temp[0];
  } else {
    PyErr_SetString(PyExc_TypeError, "expected a tuple.");
    SWIG_fail;
  }
}
// %apply (const int16_t INPUT[ANY]) {(const int16_t arr[3])};

/* C wrapper */
%{
#include "debug.h"
#include "step.h"
%}

void DebugInit();
void Step_Init(void);
uint8_t Step_Detection(const int16_t acc[3]);