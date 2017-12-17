#ifndef PID_H
#define PID_H

#include <time.h>
#include <fstream>
#include <vector>
#include <math.h>
#include <fstream>
#include <string>

class PID {
public:
  /*
  * Errors
  */
  double p_error_;
  double i_error_;
  double d_error_;

  std::vector<double> errors_;
  int run_;
  int epoch_;

  /*
  * Coefficients
  */ 
  double Kp_;
  double Ki_;
  double Kd_;

  // for tuning
  double dp_[3] = {1, 1, 1};
  double dp_sum_ = 0;
  // the parameter index to tune
  int tune_idx_ = 0;
  bool try_incrementing_ = true;
    
  // timestamp when the last error was received
  double timestamp_;

  double curr_error_ = std::numeric_limits<double>::max();
  double best_error_ = std::numeric_limits<double>::max();

  // per parameter
  const int EPOCH_LIMIT = 5;
  const int RUN_LIMIT = 3500; //3500; ///3500;
  const double TOLERANCE = 0.2;

  std::ofstream file_;

  /*
  * Constructor
  */
  PID();

  /*
  * Destructor.
  */
  virtual ~PID();

  /*
  * Initialize PID.
  */
  void Init(double Kp, double Ki, double Kd, std::string filename);

  void Reset();

  /*
  * Update the PID error variables given cross track error.
  */
  void UpdateError(double cte, bool improve);

  /*
  * Calculate the total PID error.
  */
  double TotalError();

  double PrintMetrics(std::ofstream &file);

  void Improve();
};

#endif /* PID_H */
