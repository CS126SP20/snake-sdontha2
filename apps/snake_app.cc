// Copyright (c) 2020 CS126SP20. All rights reserved.

#include "snake_app.h"

#include <cinder/Font.h>
#include <cinder/Text.h>
#include <cinder/Vector.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>
#include <gflags/gflags.h>
#include <snake/player.h>
#include <snake/segment.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>

namespace snakeapp {

using cinder::Color;
using cinder::ColorA;
using cinder::Rectf;
using cinder::TextBox;
using cinder::app::KeyEvent;
using snake::Direction;
using snake::Location;
using snake::Segment;
using std::string;
using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::system_clock;

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

SnakeApp::SnakeApp()
    : engine_{FLAGS_size, FLAGS_size},
      leaderboard_{cinder::app::getAssetPath(kDbPath).string()},
      paused_{false},
      player_name_{FLAGS_name},
      printed_game_over_{false},
      size_{FLAGS_size},
      speed_{FLAGS_speed},
      state_{GameState::kPlaying},
      tile_size_{FLAGS_tilesize},
      time_left_{0},
      last_food_location_{engine_.GetFood().GetLocation()} {}

void SnakeApp::setup() {
  cinder::gl::enableDepthWrite();
  cinder::gl::enableDepthRead();
  last_color_time_ = system_clock::now();
  last_color_ = {0, 1, 0};

  cinder::audio::SourceFileRef source_file = cinder::audio::load(
      cinder::app::loadAsset("lofi-rhodes-chords-melody_155bpm_G#.wav"));
  background_music_ = cinder::audio::Voice::create(source_file);
  background_music_->start();

  source_file = cinder::audio::load(
      cinder::app::loadAsset("Apple_Bite-Simon_Craggs-1683647397.wav"));
  eating_sound_ = cinder::audio::Voice::create(source_file);
}

void SnakeApp::update() {
  if (state_ == GameState::kGameOver) {
    background_music_->stop();
    if (top_players_.empty()) {
      leaderboard_.AddScoreToLeaderBoard({player_name_, engine_.GetScore()});
      top_players_ = leaderboard_.RetrieveHighScores(kLimit);

      // It is crucial the this vector be populated, given that `kLimit` > 0.
      assert(!top_players_.empty());
    }
    return;
  }

  if (!background_music_->isPlaying()) {
    background_music_->start();
  }

  if (paused_) return;
  const auto time = system_clock::now();

  if (engine_.GetSnake().IsChopped()) {
    if (state_ != GameState::kCountDown) {
      state_ = GameState::kCountDown;
      last_intact_time_ = time;
    }

    // We must be in countdown.
    const auto time_in_countdown = time - last_intact_time_;
    if (time_in_countdown >= kCountdownTime) {
      state_ = GameState::kGameOver;
    }

    using std::chrono::seconds;
    const auto time_left_s =
        duration_cast<seconds>(kCountdownTime - time_in_countdown);
    time_left_ = static_cast<size_t>(
        std::min(kCountdownTime.count() - 1, time_left_s.count()));
  }

  if (time - last_time_ > std::chrono::milliseconds(speed_)) {
    engine_.Step();
    last_time_ = time;
  }
}

void SnakeApp::draw() {
  cinder::gl::enableAlphaBlending();

  if (state_ == GameState::kGameOver) {
    if (!printed_game_over_) cinder::gl::clear(Color(1, 0, 0));
    DrawGameOver();
    return;
  }

  if (paused_) return;

  cinder::gl::clear();
  DrawBackground();
  DrawSnake();
  DrawFood();
  DrawScore();
  if (state_ == GameState::kCountDown) DrawCountDown();
}

template <typename C>
void PrintText(const string& text, const C& color, const cinder::ivec2& size,
               const cinder::vec2& loc) {
  cinder::gl::color(color);

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

float SnakeApp::PercentageOver() const {
  if (state_ != GameState::kCountDown) return 0.;

  using std::chrono::milliseconds;
  const double elapsed_time =
      duration_cast<milliseconds>(system_clock::now() - last_intact_time_)
          .count();
  const double countdown_time = milliseconds(kCountdownTime).count();
  const double percentage = elapsed_time / countdown_time;
  return static_cast<float>(percentage);
}

void SnakeApp::DrawBackground() const {
  const float percentage = PercentageOver();
  cinder::gl::clear(Color(percentage, 0, 0));
}

void SnakeApp::DrawGameOver() {
  // Lazily print.
  if (printed_game_over_) return;
  if (top_players_.empty()) return;

  const cinder::vec2 center = getWindowCenter();
  const cinder::ivec2 size = {500, 50};
  const Color color = Color::black();

  size_t row = 0;
  PrintText("Game Over :(", color, size, center);

  PrintText("Leaderboard", color, size,
            {center.x - center.x / 2, center.y + (++row) * 50});
  for (const snake::Player& player : top_players_) {
    std::stringstream ss;
    ss << player.name << " - " << player.score;
    PrintText(ss.str(), color, size,
              {center.x - center.x / 2, center.y + (++row) * 50});
  }

  row = 0;

  PrintText(player_name_ + "'s Scores", color, size,
            {center.x + center.x / 2, center.y + (++row) * 50});
  const std::vector<snake::Player>& player_history =
      leaderboard_.RetrieveHighScores({player_name_, engine_.GetScore()},
                                      kLimit);
  for (const snake::Player& player : player_history) {
    std::stringstream ss;
    ss << player.score;
    PrintText(ss.str(), color, size,
              {center.x + center.x / 2, center.y + (++row) * 50});
  }

  printed_game_over_ = true;
}

void SnakeApp::DrawSnake() const {
  int num_visible = 0;
  for (const Segment& part : engine_.GetSnake()) {
    const Location loc = part.GetLocation();
    if (part.IsVisibile()) {
      const double opacity = std::exp(-(num_visible++) / kRate);
      cinder::gl::color(ColorA(0, 0, 1, static_cast<float>(opacity)));
    } else {
      const float percentage = PercentageOver();
      cinder::gl::color(Color(percentage, 0, 0));
    }

    cinder::gl::drawSolidRect(Rectf(tile_size_ * loc.Row(),
                                    tile_size_ * loc.Col(),
                                    tile_size_ * loc.Row() + tile_size_,
                                    tile_size_ * loc.Col() + tile_size_));
  }
  const cinder::vec2 center = getWindowCenter();
}

void SnakeApp::DrawFood() {
  const auto time = system_clock::now();

  if (time - last_color_time_ > std::chrono::seconds(1 / engine_.GetScore())) {
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_real_distribution<double> uniform_dist(0, 1);

    last_color_[0] = uniform_dist(engine);
    last_color_[1] = uniform_dist(engine);
    last_color_[2] = uniform_dist(engine);

    cinder::gl::color(last_color_[0], last_color_[1], last_color_[2]);
    last_color_time_ = time;
  } else {
    cinder::gl::color(last_color_[0], last_color_[1], last_color_[2]);
  }

  const Location loc = engine_.GetFood().GetLocation();
  if (last_food_location_ != loc) {
    eating_sound_->start();
    last_food_location_ = loc;
  }

  cinder::gl::drawSolidRect(Rectf(tile_size_ * loc.Row(),
                                  tile_size_ * loc.Col(),
                                  tile_size_ * loc.Row() + tile_size_,
                                  tile_size_ * loc.Col() + tile_size_));
}

void SnakeApp::DrawCountDown() const {
  const float percentage = PercentageOver();
  const string text = std::to_string(time_left_);
  const Color color = {1 - percentage, 0, 0};
  const cinder::ivec2 size = {50, 50};
  const cinder::vec2 loc = {50, 50};

  PrintText(text, color, size, loc);
}

void SnakeApp::DrawScore() const {
  const cinder::vec2 center = getWindowCenter();
  const cinder::ivec2 size = {500, 50};
  const Color color = Color::white();
  const cinder::vec2 loc = {center.x, 50};

  std::stringstream ss;
  ss << engine_.GetScore();
  PrintText("Score: " + ss.str(), color, size, loc);
}

void SnakeApp::keyDown(KeyEvent event) {
  switch (event.getCode()) {
    case KeyEvent::KEY_UP:
    case KeyEvent::KEY_k:
    case KeyEvent::KEY_w: {
      engine_.SetDirection(Direction::kLeft);
      break;
    }
    case KeyEvent::KEY_DOWN:
    case KeyEvent::KEY_j:
    case KeyEvent::KEY_s: {
      engine_.SetDirection(Direction::kRight);
      break;
    }
    case KeyEvent::KEY_LEFT:
    case KeyEvent::KEY_h:
    case KeyEvent::KEY_a: {
      engine_.SetDirection(Direction::kUp);
      break;
    }
    case KeyEvent::KEY_RIGHT:
    case KeyEvent::KEY_l:
    case KeyEvent::KEY_d: {
      engine_.SetDirection(Direction::kDown);
      break;
    }
    case KeyEvent::KEY_p: {
      paused_ = !paused_;

      if (paused_) {
        last_pause_time_ = system_clock::now();
      } else {
        last_intact_time_ += system_clock::now() - last_pause_time_;
      }
      break;
    }
    case KeyEvent::KEY_r: {
      ResetGame();
      break;
    }
  }
}

void SnakeApp::ResetGame() {
  engine_.Reset();
  paused_ = false;
  printed_game_over_ = false;
  state_ = GameState::kPlaying;
  time_left_ = 0;
  top_players_.clear();
}
}  // namespace snakeapp
