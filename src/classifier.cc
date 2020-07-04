// Copyright 2020 [Your Name]. All rights reserved.

#include <math.h>
#include <snake/classifier.h>
#include <snake/image.h>
#include <snake/model.h>

namespace bayes {

Classifier::Classifier(istream& model_data) { model_ = Model(model_data); }

double Classifier::CalculatePosterior(int digit, const Image& image) {
  double posterior_prob = log(model_.get_priors()[digit]);

  for (int x = 0; x < kImageSize; x++) {
    for (int y = 0; y < kImageSize; y++) {
      if (image.get_pixels()[x][y] == ' ') {
        posterior_prob += log(model_.get_probs()[x][y][digit][0]);
      } else {
        posterior_prob += log(model_.get_probs()[x][y][digit][1]);
      }
    }
  }

  return posterior_prob;
}

int Classifier::Classify(const Image& image) {
  vector<double> posterior_probs(kNumClasses);

  for (int digit = 0; digit < kNumClasses; digit++) {
    posterior_probs[digit] = CalculatePosterior(digit, image);
  }

  // Returns the index of the maximum element of posterior_probs.
  return std::distance(
      posterior_probs.begin(),
      std::max_element(posterior_probs.begin(), posterior_probs.end()));
}

double Classifier::CalculateAccuracy(const vector<Image>& images) {
  int numerator = 0;
  int denominator = images.size();

  for (const Image& image : images) {
    if (image.get_digit() == Classify(image)) {
      numerator++;
    }
  }

  return (double)numerator / denominator;
}

Classifier::Classifier() = default;
}  // namespace bayes
