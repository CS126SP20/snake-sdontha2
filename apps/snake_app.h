// Copyright (c) 2020 CS126SP20. All rights reserved.

#ifndef SNAKE_SNAKEAPP_H_
#define SNAKE_SNAKEAPP_H_

#include <cinder/app/App.h>
#include <cinder/app/MouseEvent.h>
#include <cinder/audio/audio.h>
#include <cinder/gl/gl.h>
#include <snake/classifier.h>
#include <snake/engine.h>
#include <snake/image.h>
#include <snake/leaderboard.h>
#include <snake/location.h>
#include <snake/model.h>
#include <snake/player.h>

#include <random>
#include <string>
#include <vector>

namespace snakeapp {

enum class GameState {
  kPlaying,
  kCountDown,
  kGameOver,
};

class SnakeApp : public cinder::app::App {
 public:
  SnakeApp();
  void setup() override;
  void update() override;
  void draw() override;
  void keyDown(cinder::app::KeyEvent) override;
  void mouseDrag(cinder::app::MouseEvent event) override;
  template <typename C>
  void PrintText(const std::string& text, const C& color,
                 const cinder::ivec2& size, const cinder::vec2& loc);

 private:
  std::vector<cinder::vec2> points_;
  int digit_;
  bayes::Classifier classifier_;
  int count;

  void DrawPosition();
  void DrawPoints();
  void DrawPad();
  void DrawGuess();
};

}  // namespace snakeapp

#endif  // SNAKE_SNAKEAPP_H_
