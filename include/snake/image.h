// Copyright (c) 2020 [Your Name]. All rights reserved.

#ifndef BAYES_IMAGE_H_
#define BAYES_IMAGE_H_

#include <cstdlib>
#include <istream>
#include <string>
#include <vector>

using std::istream;
using std::string;
using std::vector;

namespace bayes {

// The size of the input images.
constexpr size_t kImageSize = 28;

class Image {
 public:
  // Constructs an image from a 2D char vector and a digit.
  Image(const vector<vector<char>>& pixels, int digit);

  // Getter for pixels_ private member.
  [[nodiscard]] const vector<vector<char>>& get_pixels() const;

  // Getter for digit_ private member.
  [[nodiscard]] int get_digit() const;

  // Generates a vector of images from istreams of images and labels.
  static vector<Image> GenerateImages(istream& images, istream& labels);

  void PrintImage();

 private:
  // Private member that houses the pixels of this image.
  vector<vector<char>> pixels_;

  // Private member that houses what digit this image is.
  int digit_;
};

}  // namespace bayes

#endif  // BAYES_IMAGE_H_
