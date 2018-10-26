#include "LocalDate.h"

namespace ace_time {

const uint8_t LocalDate::sDayOfWeek[12] = {
  5 /*1=Jan=31*/,
  1 /*2=Feb=28*/,
  0 /*3=Mar=31, start of "year"*/,
  3 /*4=Apr=30*/,
  5 /*5=May=31*/,
  1 /*6=Jun=30*/,
  3 /*7=Jul=31*/,
  6 /*8=Aug=31*/,
  2 /*9=Sep=30*/,
  4 /*10=Oct=31*/,
  0 /*11=Nov=30*/,
  2 /*12=Dec=31*/,
};

}
