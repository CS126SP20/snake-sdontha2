// Copyright (c) 2020 [Your Name]. All rights reserved.

#ifndef BAYES_CLASSIFIER_H_
#define BAYES_CLASSIFIER_H_

#include "model.h"

namespace bayes {

class Classifier {
 public:
  // Consructs a classifier from an istream of model data.
  explicit Classifier(istream& model_data);
  Classifier();

  // Returns the digit class of a given image using this classifier's model.
  int Classify(const Image& image);

  // Returns the percentage of correct classifications of the given images.
  double CalculateAccuracy(const vector<Image>& images);

 private:
  // Returns the posterior probability of a given digit and image, in
  // particular its logarithm in order to avoid arithmetic underflow. Negative
  // values are expected, but do not change the efficacy of MAP.
  double CalculatePosterior(int digit, const Image& image);

  // Houses the model this classifier is based on.
  Model model_;
};

}  // namespace bayes

#endif  // BAYES_CLASSIFIER_H_
