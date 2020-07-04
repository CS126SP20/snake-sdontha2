// Copyright 2020 [Your Name]. All rights reserved.

#include <snake/model.h>

#include <cmath>
#include <fstream>
#include <istream>
#include <queue>
#include <sstream>

using std::queue;

namespace bayes {

Model::Model() = default;

Model::Model(const vector<Image>& images) {
  // Predefined space to prevent wasted memory; vectors double the space they
  // take up in memory once they run out to achieve O(1)* runtime on push_back.
  // Since the space required for these vectors is known, defining their space
  // on initialization prevents this from happening. Thus all subsequent vector
  // initializations define their space as well.
  priors_ = vector<double>(kNumClasses);

  // Populate priors for each digit.
  for (int digit = 0; digit < kNumClasses; digit++) {
    priors_[digit] = CalculatePrior(digit, images);
  }

  // See above for the reason the initialization is complicated.
  probs_ = vector<vector<vector<vector<double>>>>(
      kImageSize,
      vector<vector<vector<double>>>(
          kImageSize,
          vector<vector<double>>(kNumClasses, vector<double>(kNumShades))));

  // Populate probabilities for each pixel, digit, and shade.
  for (int x = 0; x < kImageSize; x++) {
    for (int y = 0; y < kImageSize; y++) {
      for (int digit = 0; digit < kNumClasses; digit++) {
        for (int shade = 0; shade < kNumShades; shade++) {
          probs_[x][y][digit][shade] =
              CalculateProbability(x, y, digit, shade, images);
        }
      }
    }
  }
}

Model::Model(istream& model_data) {
  // Create the 2D queue of doubles from the given istream.
  queue<queue<double>> data = ParseModelData(model_data);

  // Convert the first inner queue into a vector. This is precisely what priors_
  // should be. Afterwards, remove the first inner queue to leave just the data
  // for probs_.
  while (!data.front().empty()) {
    priors_.push_back(data.front().front());
    data.front().pop();
  }
  data.pop();

  probs_ = vector<vector<vector<vector<double>>>>(
      kImageSize,
      vector<vector<vector<double>>>(
          kImageSize,
          vector<vector<double>>(kNumClasses, vector<double>(kNumShades))));

  // By the way the queue was constructed, probs_ can be populated line by line
  // the same way it calculated in the previous constructor (as well as how
  // how it was written to a file).
  for (int x = 0; x < kImageSize; x++) {
    for (int y = 0; y < kImageSize; y++) {
      for (int digit = 0; digit < kNumClasses; digit++) {
        for (int shade = 0; shade < kNumShades; shade++) {
          probs_[x][y][digit][shade] = data.front().front();
          data.front().pop();
        }
      }
      data.pop();
    }
  }
}

double Model::CalculatePrior(int digit, const vector<Image>& images) {
  int digit_count = 0;

  for (const Image& image : images) {
    if (image.get_digit() == digit) {
      digit_count++;
    }
  }

  return (double)digit_count / images.size();
}

double Model::CalculateProbability(int x, int y, int digit, int shade,
                                   const vector<Image>& images) {
  // Initialize numerator and denominator with corresponding Laplace Smoothing
  // constants.
  double numerator = kLapalceConstant;
  double denominator = kNumShades * kLapalceConstant;

  for (const Image& image : images) {
    // If this is an image of the given digit, add to the denominator.
    if (image.get_digit() == digit) {
      denominator++;

      // Furthermore, if the image at the given pixel is the given shade,
      // add to the numerator.
      if (((image.get_pixels()[x][y] == '#' ||
            image.get_pixels()[x][y] == '+') &&
           shade == 1) ||
          (image.get_pixels()[x][y] == ' ' && shade == 0)) {
        numerator++;
      }
    }
  }

  return (double)numerator / denominator;
}

const vector<vector<vector<vector<double>>>>& Model::get_probs() const {
  return probs_;
}

const vector<double>& Model::get_priors() const { return priors_; }

void Model::WriteModel(const string& file_name) {
  // Creates a file with given file name.
  std::ofstream out_file(file_name);

  // The first line is the space separated priors_ vector.
  for (double prior : priors_) {
    out_file << prior << " ";
  }

  out_file << std::endl;

  // Then each subsequent line is space separated, with the pattern described
  // in model.h.
  for (int x = 0; x < kImageSize; x++) {
    for (int y = 0; y < kImageSize; y++) {
      for (int digit = 0; digit < kNumClasses; digit++) {
        for (int shade = 0; shade < kNumShades; shade++) {
          out_file << probs_[x][y][digit][shade] << " ";
        }
      }
      out_file << std::endl;
    }
  }

  // Close the file once finished writing model.
  out_file.close();
}
queue<queue<double>> Model::ParseModelData(istream& model_data) {
  // Initialize the 2D queue of data.
  queue<queue<double>> data;
  string curr_line;

  // Iterate through each line of the istream.
  while (std::getline(model_data, curr_line)) {
    // Remove the trailing space of each line.
    curr_line.pop_back();

    // Convert the space separated string into a queue of doubles.
    std::stringstream ss(curr_line);
    string element;
    queue<double> line_queue;
    while (std::getline(ss, element, ' ')) {
      line_queue.push(std::stod(element));
    }

    // Populate the outer queue with this line's queue.
    data.push(line_queue);
  }

  return data;
}

// The == operator of std::vector cannot be used because it houses doubles.
// Information is lost on doubles when writing to a file, and thus when
// checking equality only needs to be equal up to a certain defined threshold.
// Hence, the need for a complicated method.
bool Model::operator==(const Model& rhs) const {
  bool areEqual = true;

  for (int i = 0; i < kNumClasses; i++) {
    // Logically, because of the loss of information on doubles,
    // expected == actual <-> |expected - actual| <= E,
    // where E is the precision that is lost upon writing to a file. Thus,
    // expected != actual <-> |expected - actual| > E, which is what is shown
    // below.
    if (std::abs(priors_[i] - rhs.get_priors()[i]) > kFloatEqualThresh) {
      return false;
    }
  }

  for (int x = 0; x < kImageSize; x++) {
    for (int y = 0; y < kImageSize; y++) {
      for (int digit = 0; digit < kNumClasses; digit++) {
        for (int shade = 0; shade < kNumShades; shade++) {
          // See above comment for explanation.
          if (std::abs(probs_[x][y][digit][shade] -
                       rhs.get_probs()[x][y][digit][shade]) >
              kFloatEqualThresh) {
            return false;
          }
        }
      }
    }
  }

  return areEqual;
}
}  // namespace bayes
