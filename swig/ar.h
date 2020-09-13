#define MAXENTROPY 0
#define LEASTSQUARES 1

// typedef float float;

int AutoRegression(float *, int, int, float *, int);
int ARMaxEntropy(float *, int, int, float **, float *, float *, float *,
                 float *);
int ARLeastSquare(float *, int, int, float *);
int SolveLE(float **, float *, unsigned int);

void AutoRegressionPy(float *inputseries, int length, int degree,
                      float *coefficients, int method);
