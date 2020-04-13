// Copyright (c) 2020 CS126SP20. All rights reserved.

#include <snake/leaderboard.h>
#include <snake/player.h>
#include <sqlite_modern_cpp.h>

#include <string>
#include <vector>

namespace snake {

using std::string;
using std::vector;

// See examples: https://github.com/SqliteModernCpp/sqlite_modern_cpp/tree/dev

LeaderBoard::LeaderBoard(const string& db_path) : db_{db_path} {
  db_ << "CREATE TABLE if not exists leaderboard (\n"
         "  name  TEXT NOT NULL,\n"
         "  score INTEGER NOT NULL\n"
         ");";
}

void LeaderBoard::AddScoreToLeaderBoard(const Player& player) {
  db_ << "INSERT INTO leaderboard (name, score)\n"
         "VALUES (?, ?);"
      << player.name
      << player.score;
}

vector<Player> GetPlayers(sqlite::database_binder* rows) {
  vector<Player> players;

  for (auto&& row : *rows) {
    string name;
    size_t score;
    row >> name >> score;
    Player player = {name, score};
    players.push_back(player);
  }

  return players;
}

vector<Player> LeaderBoard::RetrieveHighScores(const size_t limit) {
  auto rows = db_ << "SELECT name, MAX(score)\n"
                     "FROM leaderboard\n"
                     "GROUP BY name\n"
                     "ORDER BY MAX(score) DESC, name\n"
                     "LIMIT ?;"
                     << limit;
  return GetPlayers(&rows);
}

vector<Player> LeaderBoard::RetrieveHighScores(const Player& player,
                                               const size_t limit) {
  auto rows = db_ << "SELECT name, score\n"
                     "FROM leaderboard\n"
                     "WHERE name = ?\n"
                     "ORDER BY score DESC, name\n"
                     "LIMIT ?;"
                  << player.name
                  << limit;
  return GetPlayers(&rows);
}

}  // namespace snake
