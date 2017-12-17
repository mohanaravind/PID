#include "PID.h"
#include <iostream>

/*
* TODO: Complete the PID class.
*/

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd, std::string filename) {
    Kp_ = Kp;
    Ki_ = Ki;
    Kd_ = Kd;

    epoch_ = 0;
    
    // open a file for writing the metrics data
    file_.open(filename);

    // Epoch | Runs | Kp | Ki | Kd | sum | mean | std
    file_ << "Epoch" << ",";
    file_ << "Runs" << ",";
    file_ << "Kp" << ",";
    file_ << "Ki" << ",";
    file_ << "Kd" << ",";
    file_ << "Total CTE" << ",";
    file_ << "Mean CTE" << ",";
    file_ << "Standard Deviation of CTE" << std::endl;

    this->Reset();
}

void PID::Reset() {
    p_error_ = 0;
    i_error_ = 0;
    d_error_ = 0;

    timestamp_ = clock();

    errors_.clear();

    run_ = 0;        
}


void PID::UpdateError(double cte, bool improve) {
    d_error_ = cte - p_error_;
    p_error_ = cte;
    i_error_ += cte;

    dp_sum_ = 0;   
    for (double dp : dp_) {
        dp_sum_ += dp;
    }
    
    // check if we reached the limit
    if (dp_sum_ <= TOLERANCE || epoch_ == EPOCH_LIMIT * 3) {
        std::cout << "EPOCH LIMIT REACHED" << std::endl;        
        file_.close();
        //std::exit(0);
    } else if (++run_ == RUN_LIMIT) {
        std::cout << "PRINTIG METRICS TO THE FILE" << std::endl;        
        // set this as the current error
        curr_error_ = this->PrintMetrics(file_);
        
        this->Reset();
        // increment the epoch count
        epoch_++;

        // if improve flag is set
        if (improve) {
            this->Improve();
        }
    }

    // add this to the list of errors
    errors_.push_back(cte);
}

double PID::TotalError() {
    // compute the time taken between the measurements        
    double now = clock();
    double dt = (now - timestamp_)/CLOCKS_PER_SEC;
    
    // update the timestamp
    timestamp_ = now;    

    double Kp = Kp_ * dp_[0];
    double Ki = Ki_ * dp_[1];
    double Kd = Kd_ * dp_[2];

    // PID
    return -(Kp * p_error_ + Ki * i_error_ * dt + Kd * d_error_/dt);
}

double PID::PrintMetrics(std::ofstream &file) {    
    double Kp = Kp_ * dp_[0];
    double Ki = Ki_ * dp_[1];
    double Kd = Kd_ * dp_[2];

    int count = errors_.size();

    // compute the mean
    double mean = i_error_ / count;
    
    // compute the standard deviation
    double std = 0.0;
    for (double err : errors_) {
        std += pow(err - mean, 2);
    }    
    std = sqrt(std/count);
    
    std::cout << "Kp " << Kp << "| Ki " << Ki << "| Kd " << Kd << "| std " << std << std::endl;
    
    /*         
       Epoch | Runs | Kp | Ki | Kd | count | mean | std
    */
    file << epoch_ << ",";
    file << run_ << ",";
    file << Kp << ",";
    file << Ki << ",";
    file << Kd << ",";
    file << i_error_ << ",";
    file << mean << ",";
    file << std;

    file << std::endl;

    return std;  
}

void PID::Improve() {
    // if the current error is better than the best
    if (curr_error_ < best_error_ || best_error_ == std::numeric_limits<double>::max()) {
        best_error_ = curr_error_;
        dp_[tune_idx_] *= 1.1;
    } else {
        dp_[tune_idx_] *= 0.9;
    }
    
    // if it is improving continue what is being done
    // if (curr_error_ < best_error_) {
    //     best_error_ = curr_error_;        
    // } else {
    //     // change the strategy
    //     try_incrementing_ = !try_incrementing_;
    // }
       
    // if (try_incrementing_) {
    //     dp_[tune_idx_] *= 1.1;        
    // } else {
    //     dp_[tune_idx_] *= 0.9;        
    // }
        
    // move on to the next parameter to tune
    tune_idx_ = epoch_/EPOCH_LIMIT;
}

