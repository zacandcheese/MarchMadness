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
    int opp_id;
    std::string result;
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
    int idx = 0;
    int team_id;
    std::errc ec = std::from_chars(f[idx].data(), f[idx].data() + f[idx].size(), team_id).ec;
    if (ec != std::errc()) {
        return false; // malformed field — bail like your current catch block does
    }
    g.team_id = team_id;

    idx = 1;
    int game_id;
    ec = std::from_chars(f[idx].data(), f[idx].data() + f[idx].size(), game_id).ec;
    if (ec != std::errc()) {
        return false; // malformed field — bail like your current catch block does
    }
    g.game_id = game_id;

    g.opp_id = f[5].empty() ? -1 : std::stoi(f[5]);

    g.result = f[6];
    return true;
}

// Every team's schedule.csv lives at data/espn/<team_id>/<year>/schedule.csv.
// Note: each game shows up twice in the raw data (once in each team's file),
// so games.size() here is ~2x the number of distinct games played.
std::vector<Game> loadSchedules(const std::string& dataRoot, const std::vector<int>& years) {
    std::vector<Game> games;
    games.reserve(20000); //headroom
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

bool comp (const Game& x, const Game& y){
    return x.game_id > y.game_id;
}

bool compEq(const Game& x,const Game& y){
    return x.game_id == y.game_id;
}

template<int K_FACTOR>
int play(const Game& g, std::unordered_map<int, int> &teams){
    int BASE_RANK = 1400;
    int Ra = (teams.try_emplace(g.team_id, BASE_RANK)).first->second;
    int Rb = (teams.try_emplace(g.opp_id, BASE_RANK)).first->second;

    float Ea = 1 / (1 + std::pow(10, ((Rb - Ra)/400.0)));
    float Eb = 1  - Ea;

    int Sa = (g.result == "W") ? 1: 0;

    teams[g.team_id] = Ra + K_FACTOR * (Sa - Ea);
    teams[g.opp_id] = Rb + K_FACTOR * ((1-Sa) - Eb);

    return (Ra > Rb)? Sa : 1-Sa;
}

template<int K_FACTOR>
void runBacktest(const std::vector<Game>& games) {
    std::unordered_map<int, int> teams;
    float wins = 0;
    int cnt = 0;
    for (const auto& g: games){
        int win = play<K_FACTOR>(g, teams);
        wins += win;
        cnt += 1.0;
    }
    std::cout << wins / cnt << "\n";
}


int main() {
    std::vector<int> years = {2024};
    auto t0 = std::chrono::steady_clock::now();
    std::vector<Game> games = loadSchedules("data", years);
    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> ms = t1 - t0;
    std::cout << "Loaded " << games.size() << " schedule rows\n";
    std::cout << "Load Time " << ms.count() << "\n";
    // sort the games
    sort(games.begin(), games.end(), comp);
    auto it = unique(games.begin(), games.end(), compEq);
    games.erase(it, games.end());
    
    runBacktest<16>(games);
    auto t2 = std::chrono::steady_clock::now();
    ms = t2 - t1;
    std::cout << "Run Time " << ms.count() << "\n";
}
