// Copyright (c) 2020 CS126SP20. All rights reserved.

#include "snake_app.h"

#include <cinder/Font.h>
#include <cinder/Text.h>
#include <cinder/Vector.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>
#include <gflags/gflags.h>
#include <snake/classifier.h>
#include <snake/image.h>
#include <snake/model.h>
#include <snake/player.h>
#include <snake/segment.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

namespace snakeapp {

using cinder::Color;
using cinder::ColorA;
using cinder::Rectf;
using cinder::TextBox;
using cinder::app::KeyEvent;
using namespace ci::app;
using snake::Direction;
using snake::Location;
using snake::Segment;
using std::string;
using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::system_clock;

const int kImageScale = 11;
const int kImageSize = 28;
const int kWindowSize = 800;
const int kMarkerWidth = 16;

const double kRate = 25;
const size_t kLimit = 3;
const char kDbPath[] = "snake.db";
const seconds kCountdownTime = seconds(10);
#if defined(CINDER_COCOA_TOUCH)
const char kNormalFont[] = "Arial";
const char kBoldFont[] = "Arial-BoldMT";
const char kDifferentFont[] = "AmericanTypewriter";
#elif defined(CINDER_LINUX)
const char kNormalFont[] = "Arial Unicode MS";
const char kBoldFont[] = "Arial Unicode MS";
const char kDifferentFont[] = "Purisa";
#else
const char kNormalFont[] = "Arial";
const char kBoldFont[] = "Arial Bold";
const char kDifferentFont[] = "Papyrus";
#endif

DECLARE_uint32(size);
DECLARE_uint32(tilesize);
DECLARE_uint32(speed);
DECLARE_string(name);

SnakeApp::SnakeApp() {}

void SnakeApp::setup() {
  std::ifstream ifs(
      "/Users/suchetan/Desktop/Cinder/cinder_0.9.2_mac/my-projects/"
      "snake-sdontha2/resources/model2.txt");
  classifier_ = bayes::Classifier(ifs);
}

void SnakeApp::update() {
  count++;
  int upper_left = (kWindowSize - kImageSize * kImageScale) / 2;

  // all pixels initially unshaded
  vector<vector<bool>> pixels = vector<vector<bool>>(
      kImageSize * kImageScale, vector<bool>(kImageSize * kImageScale, false));

  // shade all pixels that mouse dragged over
  for (const auto& point : points_) {
    int x = point.x - upper_left;
    int y = point.y - upper_left;
    for (int i = std::max(0, x - kMarkerWidth / 2);
         i < std::min(kImageSize * kImageScale, x + kMarkerWidth / 2); i++) {
      for (int j = std::max(0, y - kMarkerWidth / 2);
           j < std::min(kImageSize * kImageScale, y + kMarkerWidth / 2); j++) {
        pixels[i][j] = true;
      }
    }
  }

  // all pixels are spaces initially
  vector<vector<char>> scaled_pixels =
      vector<vector<char>>(kImageSize, vector<char>(kImageSize, ' '));

  // scale pixels by kImageScale
  for (int i = 0; i < kImageSize; i++) {
    for (int j = 0; j < kImageSize; j++) {
      int num_shaded = 0;

      for (int x = i * kImageScale; x < i * kImageScale + kImageScale; x++) {
        for (int y = j * kImageScale; y < j * kImageScale + kImageScale; y++) {
          if (pixels[x][y]) {
            num_shaded++;
          }
        }
      }

      if (num_shaded > kImageScale * kImageScale / 2) {
        scaled_pixels[j][i] = '#';
      }
    }
  }

  bayes::Image drawn(scaled_pixels, -1);
  digit_ = classifier_.Classify(drawn);
}

void SnakeApp::draw() {
  cinder::gl::clear(Color(0, 0, 0));

  DrawPad();
  DrawPosition();
  DrawPoints();
  DrawGuess();
}

void SnakeApp::keyDown(KeyEvent event) {
  switch (event.getCode()) {
    case KeyEvent::KEY_c: {
      points_.clear();
    }
  }
}

void SnakeApp::mouseDrag(cinder::app::MouseEvent event) {
  int upper_left = (kWindowSize - kImageSize * kImageScale) / 2;
  int bottom_right = upper_left + kImageSize * kImageScale;
  auto point = event.getPos();

  if ((point.x > upper_left && point.x < bottom_right) &&
      (point.y > upper_left && point.y < bottom_right)) {
    points_.emplace_back(point);
  }
}

template <typename C>
void SnakeApp::PrintText(const std::string& text, const C& color,
                         const cinder::ivec2& size, const cinder::vec2& loc) {
  auto box = TextBox()
                 .alignment(TextBox::CENTER)
                 .font(cinder::Font(kNormalFont, 30))
                 .size(size)
                 .color(color)
                 .backgroundColor(ColorA(0, 0, 0, 0))
                 .text(text);

  const auto box_size = box.getSize();
  const cinder::vec2 locp = {loc.x - box_size.x / 2, loc.y - box_size.y / 2};
  const auto surface = box.render();
  const auto texture = cinder::gl::Texture::create(surface);
  cinder::gl::draw(texture, locp);
}

void SnakeApp::DrawPosition() {
  const cinder::vec2 center = getWindowCenter();
  const cinder::ivec2 size = {300, 50};
  const Color color = ColorA::hex(0xFF0000);

  const cinder::vec2 loc = {center.x + center.x / 2, 50};
  std::stringstream ss;
  if (!points_.empty()) {
    ss << "(";
    ss << points_.back().x;
    ss << ", ";
    ss << points_.back().y;
    ss << ")";

    PrintText("(X, Y): " + ss.str(), color, size, loc);
  }
}

void SnakeApp::DrawPoints() {
  int upper_left = (kWindowSize - kImageSize * kImageScale) / 2;
  int bottom_right = upper_left + kImageSize * kImageScale;
  cinder::gl::color(0, 0, 0);

  for (const auto& point : points_) {
    int x = point.x;
    int y = point.y;
    cinder::gl::drawSolidRect(
        Rectf(std::max(upper_left, x - kMarkerWidth / 2),
              std::max(upper_left, y - kMarkerWidth / 2),
              std::min(bottom_right, x + kMarkerWidth / 2),
              std::min(bottom_right, y + kMarkerWidth / 2)));
  }
}

void SnakeApp::DrawPad() {
  int upper_left = (kWindowSize - kImageSize * kImageScale) / 2;
  int bottom_right = upper_left + kImageSize * kImageScale;
  cinder::gl::color(1, 1, 1);
  cinder::gl::drawSolidRect(
      Rectf(upper_left, upper_left, bottom_right, bottom_right));
}

void SnakeApp::DrawGuess() {
  const cinder::vec2 center = getWindowCenter();
  const cinder::ivec2 size = {300, 50};
  const Color color = ColorA::hex(0xFF0000);
  cinder::gl::color(1, 0, 0);

  const cinder::vec2 loc = {center.x, 700};
  std::stringstream ss;
  ss << digit_;
  PrintText("Model Guess: " + ss.str(), color, size, loc);
}
}  // namespace snakeapp
