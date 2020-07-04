// Copyright (c) 2020 [Your Name]. All rights reserved.

#ifndef BAYES_MODEL_H_
#define BAYES_MODEL_H_

#include <cstdlib>
#include <queue>

#include "image.h"

using std::queue;

namespace bayes {

// Represents digits 0-9.
constexpr size_t kNumClasses = 10;

// Represents number of pixel shades.
constexpr size_t kNumShades = 2;

// Represents the Laplace Smoothing constant.
constexpr size_t kLapalceConstant = 2;

// Represents the error allowed when checking the equality of floats.
constexpr double kFloatEqualThresh = 0.000001;

class Model {
 public:
  // Constructs a model from images.
  explicit Model(const vector<Image>& images);

  // Constructs a model from an istream (e.g. file). The format must be space
  // separated values, with the first line being the priors_ and each
  // subsequent line the probabilities of the image at (0, 0), (0, 1), ...
  // listed in order of digits 0-9 alternating between the different shades.
  // e.x. 0.995859 0.00414079 0.996473 0.00352734 0.995935 0.00406504 ...
  //          ^ prob[0][0][0][0]            ^ prob[0][1][1][1]
  explicit Model(istream& model_data);

  // Default constructor.
  Model();

  // Getter for private probs_ member.
  [[nodiscard]] const vector<vector<vector<vector<double>>>>& get_probs() const;

  // Getter for private priors_ member.
  [[nodiscard]] const vector<double>& get_priors() const;

  // Writes this model into a given file name in the format described in
  // the Model constructor.
  void WriteModel(const string& file_name);

  // Equality operator, overloaded to
  bool operator==(const Model& rhs) const;

 private:
  // Returns the probability that the pixel at (x, y) is a particular shade
  // when looking at images of digit by using the data of images. Utilizes
  // Bayes theorem. Non-zero as a result of Laplace Smoothing.
  static double CalculateProbability(int x, int y, int digit, int shade,
                                     const vector<Image>& images);

  // Returns the prior of digit by using the data of images. This is the
  // probability that an image is of the given digit.
  static double CalculatePrior(int digit, const vector<Image>& images);

  // Parses the numbers from an istream of model data into a 2D queue. Each
  // internal queue is a line from the file, with each element being split
  // around spaces. (< > represent a queue)
  // e.x. 0.995859 0.00414079 ... -> <0.995859, 0.00414079, ...>
  static queue<queue<double>> ParseModelData(istream& model_data);

  // Private member housing the probabilities of a shade at (x, y) when
  // looking at images of a digit.
  vector<vector<vector<vector<double>>>> probs_;

  // Private member housing the priors of a digit.
  vector<double> priors_;
};

}  // namespace bayes

#endif  // BAYES_MODEL_H_
