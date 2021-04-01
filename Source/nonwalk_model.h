union NonWalkModelEntry {
  int missing;
  float fvalue;
  int qvalue;
};
#define NonWalkModelPredict _step_nn
float NonWalkModelPredict(union NonWalkModelEntry* data);
