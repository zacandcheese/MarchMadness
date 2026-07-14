# MarchMadness

A prediction and betting-strategy engine for NCAA basketball, built to showcase quantitative/optimization skills. A backend engine ingests box scores, play-by-play, rosters, and schedules to produce calibrated win probabilities and betting decisions; a frontend lets you pick which model(s) and strategy parameters to run.

## Data Assets
- **Box scores** (basic + advanced, per team/season/game) — shooting splits, efficiency stats
- **Play-by-play** (per game, event-level) — possession-by-possession log
- **Roster** (per team/season) — player-level composition
- **Schedule** (per team/season) — game dates, opponents, home/away/neutral

## Prediction Engine — Models to Explore

### 1. Rating & Ranking Systems (the "quant" classics)
- Elo / Glicko-2, with home-court, margin-of-victory, and rest adjustments
- Log5 (Bill James) for converting two team ratings into a head-to-head win probability
- Massey & Colley matrix ratings — solved via least squares / linear algebra, a nice showcase of numerical methods
- Pythagorean expectation (Dean Oliver's basketball variant) from points for/against
- Adjusted efficiency margin (KenPom-style) — iteratively solve for offensive/defensive efficiency adjusted for opponent strength and pace; essentially a fixed-point/EM optimization problem
- RPI / strength-of-schedule as a supplementary feature

### 2. Statistical / ML Models on Engineered Features
- Logistic regression on efficiency differential, four factors, SOS — clean, interpretable baseline
- Regularized regression (ridge/lasso) for point-spread prediction
- Gradient-boosted trees / random forest on the full box-score feature set
- Feature importance analysis (what actually drives wins) — good explainability talking point
- Player-level plus-minus / RAPM if lineup data can be extracted from play-by-play

### 3. Simulation-Based Models
- Monte Carlo possession simulation: model each possession as a draw from team offensive/defensive tendencies (shot selection, turnover rate, rebound rate) derived from box score + PBP distributions; simulate full games thousands of times
- Markov chain game-state model built directly from PBP transitions (score margin, time remaining, possession) → in-game win probability
- Full bracket simulation: propagate simulated win probabilities through the 68-team bracket to get championship odds and seed-vs-seed upset rates

### 4. Bayesian Approaches
- Hierarchical Bayesian team-strength model with partial pooling across conferences — handles small early-season samples gracefully
- Bayesian in-season updating: prior from preseason/prior-year performance, posterior updated after each game
- Demonstrates uncertainty quantification, not just point estimates

### 5. Sequence Models (stretch)
- In-game win probability as a sequence model over PBP events — reacts to runs/momentum better than a static Markov chain
- Player trend modeling (form, fatigue) over the course of a season

### 6. Ensembling
- Stack rating-system output + ML model + simulation probabilities. Ensembles reliably beat any single model in sports prediction and are a natural place to show meta-modeling judgment.

## Betting / Decision Engine — Strategies to Explore
- **Edge detection** — compare model win probability to market-implied probability (from odds) to find +EV bets
- **Kelly Criterion staking** (full and fractional), sized by edge and model confidence
- **Bankroll simulation** — drawdown and variance analysis; classic position-sizing/risk-of-ruin territory
- **Closing line value (CLV)** tracking — the standard way professional bettors self-evaluate, even before outcomes are known
- **Live/in-play betting** — feed the Markov chain live win-probability model against real-time odds movement to find inefficiencies
- **Arbitrage / hedging** across books or bet types (moneyline vs. spread vs. live), if multiple odds sources are collected
- **Portfolio view** across simultaneous tournament games — correlated risk when overexposed to one team advancing multiple rounds

## Evaluation & Rigor
This is what actually reads as "quant" to reviewers — more than model sophistication:
- Strict walk-forward backtesting — never let a model see future games/seasons
- Score with Brier score & log-loss, not just accuracy — calibration matters more than hit rate when betting
- Calibration plots (predicted probability vs. observed frequency)
- Separate prediction-accuracy backtests from ROI/CLV backtests — a well-calibrated model can still lose money on bad staking
- Always compare against dumb baselines (seed-only, Vegas-implied) to prove genuine edge

## Feature Ideas by Data Source
- **Box scores**: four factors (eFG%, TOV%, ORB%, FT rate), pace, offensive/defensive rating, shooting splits
- **Play-by-play**: possession-level transitions, scoring-run detection, clutch-time performance (final 5 min, margin ≤5), foul-trouble effects
- **Roster**: experience/class mix, returning production, height/length proxies
- **Schedule**: rest days, travel, back-to-backs, conference-tournament fatigue, home/away/neutral

## Suggested Direction (light)
- Split this into two decoupled engines behind one API: a **Prediction Engine** (produces calibrated win probabilities / point-spread distributions) and a **Decision Engine** (converts probabilities + market odds into stake recommendations). Keep them composable so the frontend can mix parameters like "Elo + Kelly" vs. "Monte Carlo + fractional Kelly" as dropdowns.
- Store model outputs (full probabilities, not just picks) so new staking strategies can be backtested against old predictions without rerunning the whole pipeline.
- Start with the classics — Elo/Log5, four-factors logistic regression, Monte Carlo simulation — to get a credible baseline and backtest harness working end-to-end before investing in Bayesian/sequence models. The harness and evaluation discipline matter more, early on, than model sophistication.

## Run Me
g++ -std=c++17 -O2 -Wall -o /tmp/claude-1000/-home-znowak-MarchMadness/e21d0c38-c832-4dc5-b7ee-de4952276681/scratchpad/elo elo.cpp && /tmp/claude-1000/-home-znowak-MarchMadness/e21d0c38-c832-4dc5-b7ee-de4952276681/scratchpad/elo