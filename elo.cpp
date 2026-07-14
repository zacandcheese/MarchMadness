// Load all the schedules into one mega object
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <bits/stdc++.h>
#include <algorithm>

namespace fs = std::filesystem;

struct Game {
    int team_id;
    long long game_id;
    std::string date;
    std::string home_away;
    std::string opponent;
    int opp_id;
    std::string result;
    std::string score;
    std::string url;
};


struct Team {
    int team_id;
    long long elo = 1400;
};

// Splits one CSV line into fields, honoring double-quoted fields that may contain commas
// (schedule.csv dates look like "Mon, Nov 4").
static std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    for (char c : line) {
        if (inQuotes) {
            if (c == '"') inQuotes = false;
            else field += c;
        } else {
            if (c == '"') inQuotes = true;
            else if (c == ',') { fields.push_back(field); field.clear(); }
            else field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

// team_id,game_id,date,home_away,opponent,opp_id,result,score,game_url
static bool parseGame(const std::vector<std::string>& f, Game& g) {
    if (f.size() < 9) return false;
    try {
        g.team_id = std::stoi(f[0]);
        g.game_id = std::stoll(f[1]);
    } catch (...) {
        return false;
    }
    g.date = f[2];
    g.home_away = f[3];
    g.opponent = f[4];
    g.opp_id = f[5].empty() ? -1 : std::stoi(f[5]);
    g.result = f[6];
    g.score = f[7];
    g.url = f[8];
    return true;
}

// Every team's schedule.csv lives at data/espn/<team_id>/<year>/schedule.csv.
// Note: each game shows up twice in the raw data (once in each team's file),
// so games.size() here is ~2x the number of distinct games played.
std::vector<Game> loadSchedules(const std::string& dataRoot, const std::vector<int>& years) {
    std::vector<Game> games;
    fs::path espnRoot = fs::path(dataRoot) / "espn";

    for (const auto& teamDir : fs::directory_iterator(espnRoot)) {
        if (!teamDir.is_directory()) continue;
        for (int year : years) {
            fs::path schedulePath = teamDir.path() / std::to_string(year) / "schedule.csv";
            if (!fs::exists(schedulePath)) continue;

            std::ifstream file(schedulePath);
            std::string line;
            std::getline(file, line); // header
            while (std::getline(file, line)) {
                if (line.empty()) continue;
                Game g;
                if (parseGame(parseCsvLine(line), g)) {
                    games.push_back(g);
                }
            }
        }
    }
    return games;
}

bool comp (Game x, Game y){
    return x.game_id > y.game_id;
}

bool compEq(Game x, Game y){
    return x.game_id == y.game_id;
}

int play(Game g, std::unordered_map<int, int> &teams, int K_factor = 16){
    int BASE_RANK = 1400;
    teams.try_emplace(g.team_id, BASE_RANK);
    teams.try_emplace(g.opp_id, BASE_RANK);

    int Ra = teams[g.team_id];
    int Rb = teams[g.opp_id];
    float Ea = 1 / (1 + std::pow(10, ((Rb - Ra)/400.0)));
    float Eb = 1 / (1 + std::pow(10, ((Ra - Rb)/400.0)));

    int Sa = (g.result == "W") ? 1: 0;

    teams[g.team_id] = Ra + K_factor * (Sa - Ea);
    teams[g.opp_id] = Rb + K_factor * ((1-Sa) - Eb);

    return (Ra > Rb)? Sa : 1-Sa;
}


int main() {
    std::vector<int> years = {2024};
    std::vector<Game> games = loadSchedules("data", years);
    std::cout << "Loaded " << games.size() << " schedule rows\n";
    // sort the games
    sort(games.begin(), games.end(), comp);
    auto it = unique(games.begin(), games.end(), compEq);
    games.erase(it, games.end());
    
    std::unordered_map<int, int> teams;

    float wins = 0;
    int cnt = 0;
    for (const auto& g: games){
        int win = play(g, teams);
        wins += win;
        cnt += 1.0;
        if (cnt % 100 == 0) {
            std::cout << wins / cnt << "\n";
        }
    }
}
