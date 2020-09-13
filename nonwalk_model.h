union NonWalkModelEntry {
  int missing;
  float fvalue;
  int qvalue;
};

float NonWalkModelPredict(union NonWalkModelEntry* data);