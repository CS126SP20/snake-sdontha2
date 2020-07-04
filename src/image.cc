// Copyright 2020 [Your Name]. All rights reserved.

#include <snake/image.h>

#include <iostream>
#include <vector>

using std::vector;

namespace bayes {

Image::Image(const vector<vector<char>>& pixels, int digit) {
  pixels_ = pixels;
  digit_ = digit;
}

const vector<vector<char>>& Image::get_pixels() const { return pixels_; }

int Image::get_digit() const { return digit_; }

vector<Image> Image::GenerateImages(istream& image_data, istream& label_data) {
  vector<Image> images;
  string image_line;
  string label_line;
  vector<vector<char>> curr_image_vector;
  int curr_line = 1;

  vector<int> image_labels;
  while (std::getline(label_data, label_line)) {
    image_labels.push_back(std::stoi(label_line));
  }

  while (std::getline(image_data, image_line)) {
    curr_image_vector.emplace_back(image_line.begin(), image_line.end());

    if (curr_line % kImageSize == 0) {
      Image curr_image(curr_image_vector,
                       image_labels[curr_line / kImageSize - 1]);
      images.push_back(curr_image);
      curr_image_vector.clear();
    }

    curr_line++;
  }

  return images;
}

void Image::PrintImage() {
  for (vector<char> row : pixels_) {
    for (char pixel : row) {
      std::cout << pixel;
    }
    std::cout << std::endl;
  }
}
}  // namespace bayes
