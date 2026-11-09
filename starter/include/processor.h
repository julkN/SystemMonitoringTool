#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();
 private:
  long previous_total_{0};
  long previous_idle_{0};
};

#endif