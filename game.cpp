#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <random>


namespace  {


double random() {
  static std::default_random_engine random_engine(
  std::chrono::system_clock::now().time_since_epoch().count());
  static std::uniform_real_distribution<double> distribution(0., 1.);

  return distribution(random_engine);
}


enum class Action {
  fold,
  check,
  call,
  bet,
};

std::ostream& operator<<(std::ostream& os, const Action& action) {
  switch (action) {
    case Action::fold: os << "Fold"; break;
    case Action::check: os << "Check"; break;
    case Action::call: os << "Call"; break;
    case Action::bet: os << "Bet"; break;
    default: throw std::runtime_error("This should never happen!"); break;
  }

  return os;
}


enum class Card : int {
  jack,
  queen,
  king,
};


std::ostream& operator<<(std::ostream& os, const Card& card) {
  switch (card) {
    case Card::jack: os << "Jack"; break;
    case Card::queen: os << "Queen"; break;
    case Card::king: os << "King"; break;
    default: throw std::runtime_error("This should never happen!");
  }

  return os;
}


std::vector<Card> shuffle() {
  std::vector<Card> cards = {Card::jack, Card::queen, Card::king};
  static std::default_random_engine random_engine(
    std::chrono::system_clock::now().time_since_epoch().count());
  std::shuffle(cards.begin(), cards.end(), random_engine);

  return cards;
}


bool
actions_are_legal(const std::vector<Action>& actions) {
  const std::vector<std::vector<Action>> all_legal_actions = {
    {Action::fold},
    {Action::check, Action::fold},
    {Action::check, Action::check},
    {Action::check, Action::bet, Action::fold},
    {Action::check, Action::bet, Action::call},
    {Action::bet, Action::fold},
    {Action::bet, Action::call},
  };
  bool is_legal_action{false};
  for (auto legal_actions : all_legal_actions) {
    if (is_legal_action) {
      break;
    }
    if (actions.size() <= legal_actions.size()) {
      is_legal_action = true;
      for (size_t index{0}; index < actions.size(); index++) {
        if (actions[index] != legal_actions[index]) {
          is_legal_action = false;
          break;
        }
      }
    }
  }

  return is_legal_action;
}


bool
actions_are_complete(const std::vector<Action>& actions) {
  const std::vector<std::vector<Action>> all_legal_actions = {
    {Action::fold},
    {Action::check, Action::fold},
    {Action::check, Action::check},
    {Action::check, Action::bet, Action::fold},
    {Action::check, Action::bet, Action::call},
    {Action::bet, Action::fold},
    {Action::bet, Action::call},
  };
  for (auto legal_actions : all_legal_actions) {
    if (actions == legal_actions) {
      return true;
    }
  }

  return false;
}


std::pair<int, int>
calculate_bets(const std::vector<Action>& actions,
              bool verify_actions = true) {
  if (verify_actions && !actions_are_legal(actions)) {
    throw std::runtime_error("Encountered illegal sequence of actions!");
  }
  std::pair<int, int> bets = {1, 1};  // antees
  bool alice_turn{true};
  for (auto action : actions) {
    if (action == Action::bet || action == Action::call) {
      if (alice_turn) {
        std::get<0>(bets)++;
      } else {
        std::get<1>(bets)++;
      }
    }
    alice_turn = !alice_turn;
  }
  return bets;
}


bool alice_won(const std::vector<Action>& actions,
               const Card& alice_card,
               const Card& bob_card,
               bool verify_action = true) {
  assert(alice_card != bob_card);
  if (verify_action && !actions_are_legal(actions)) {
    throw std::runtime_error("Encountered an illegal sequence of actions!");
  } else if (verify_action && !actions_are_complete(actions)) {
    throw std::runtime_error("Encountered an incomplete sequence of actions!");
  }

  switch (actions.size()) {
    case 1:
      assert(actions[0] == Action::fold);  // Alice folds
      return false;
      break;
    case 2:
      if (actions[1] == Action::fold) {  // Bob folds
        return true;
      } else {  // showdown
        assert(actions[1] == Action::check || actions[1] == Action::call);
        return alice_card > bob_card;
      }
      break;
    case 3:
      if (actions[2] == Action::fold) {  // Alice folds
        return false;
      } else {  // showdown
        assert(actions[2] == Action::check || actions[2] == Action::call);
        return alice_card > bob_card;
      }
    default:
      throw std::runtime_error("This should never happen!");
      break;
  }
}


std::pair<int, int>
play_round(const Card& alice_card,
           const Card& bob_card,
           Action(*alice_strategy)
             (const std::vector<Action>& actions, const Card& card),
           Action(*bob_strategy)
             (const std::vector<Action>& actions, const Card& card),
           bool verbose = false) {
  std::vector<Action> actions;

  for (size_t move{0}; move < 4; move++) {
    Action action;
    if (move % 2 == 0) {
      action = alice_strategy(actions, alice_card);
    } else {
      action = bob_strategy(actions, bob_card);
    }
    actions.push_back(action);
    if (actions_are_complete(actions)) {
      break;
    }
  }
  assert(actions_are_legal(actions));
  assert(actions_are_complete(actions));
  auto [alice_bets, bob_bets] = calculate_bets(actions, false);
  auto alice_has_won = alice_won(actions, alice_card, bob_card, false);

  if (verbose) {
    std::cout << "Actions: [";
    for (auto it = actions.begin(); it != actions.end(); it++) {
      std::cout << *it;
      if (std::next(it) != actions.end()) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;
  }
  if (alice_has_won) {
    return {bob_bets, -bob_bets};
  } else {
    return {-alice_bets, alice_bets};
  }
}


namespace strategy {

Action randomized(const std::vector<Action>& actions, const Card& card) {
  bool is_alice = actions.size() % 2 == 0;
  bool is_facing_bet{false};

  if (is_alice) {
    is_facing_bet = (actions.size() == 2) && actions[1] == Action::bet;
  } else {
    is_facing_bet = (actions.size() == 1) && actions[0] == Action::bet;
  }

  if (is_facing_bet) {
    return random() < 0.5 ? Action::call : Action::fold;
  } else {
    return random() < 0.5 ? Action::bet : Action::check;
  }

  throw std::runtime_error("This should never happen!");
}

Action naive(const std::vector<Action>& actions, const Card& card) {
  bool is_alice = actions.size() % 2 == 0;
  bool is_facing_bet{false};

  if (is_alice) {
    is_facing_bet = (actions.size() == 2) && actions[1] == Action::bet;
  } else {
    is_facing_bet = (actions.size() == 1) && actions[0] == Action::bet;
  }

  switch (card) {
    case Card::jack:
      return is_facing_bet ? Action::fold : Action::check;
      break;
    case Card::queen:
      return is_facing_bet ? Action::call : Action::check;
      break;
    case Card::king:
      return is_facing_bet ? Action::call : Action::bet;
      break;
    default:
      throw std::runtime_error("This should never happen!");
      break;
  }
}


Action exploit_naive(const std::vector<Action>& actions, const Card& card) {
  bool is_alice = actions.size() % 2 == 0;
  bool is_facing_bet{false};

  if (is_alice) {
    is_facing_bet = (actions.size() == 2) && actions[1] == Action::bet;
  } else {
    is_facing_bet = (actions.size() == 1) && actions[0] == Action::bet;
  }

  if (is_alice) {
    is_facing_bet = (actions.size() == 2) && actions[1] == Action::bet;
    switch (card) {
      case Card::jack:
        if (is_facing_bet) {
          return Action::fold;
        } else {
          return Action::check;
        }
        break;
      case Card::queen:
        return is_facing_bet ? Action::fold : Action::check;
        break;
      case Card::king:
        return is_facing_bet ? Action::call : Action::bet;
        break;
      default:
        throw std::runtime_error("This should never happen!");
        break;
    }
  } else {
    is_facing_bet = (actions.size() == 1) && actions[0] == Action::bet;
    switch (card) {
      case Card::jack:
        if (is_facing_bet) {
          return Action::fold;
        } else {
          return Action::check;
        }
        break;
      case Card::queen:
        if (is_facing_bet) {
          return Action::fold;
        } else {
          return Action::check;
        }
        break;
      case Card::king:
        return is_facing_bet ? Action::call : Action::bet;
        break;
      default:
        throw std::runtime_error("This should never happen!");
        break;
    }
  }

  throw std::runtime_error("This should never happen!");
}

Action counter_exploit(const std::vector<Action>& actions, const Card& card) {
  bool is_alice = actions.size() % 2 == 0;
  bool is_facing_bet{false};

  if (is_alice) {
    is_facing_bet = (actions.size() == 2) && actions[1] == Action::bet;
  } else {
    is_facing_bet = (actions.size() == 1) && actions[0] == Action::bet;
  }

  if (is_alice) {
    is_facing_bet = (actions.size() == 2) && actions[1] == Action::bet;
    switch (card) {
      case Card::jack:
        if (is_facing_bet) {
          return Action::fold;
        } else {
          return Action::bet;
        }
        break;
      case Card::queen:
        return is_facing_bet ? Action::call: Action::check;
        break;
      case Card::king:
        return is_facing_bet ? Action::call : Action::check;
        break;
      default:
        throw std::runtime_error("This should never happen!");
        break;
    }
  } else {
    is_facing_bet = (actions.size() == 1) && actions[0] == Action::bet;
    switch (card) {
      case Card::jack:
        if (is_facing_bet) {
          return Action::fold;
        } else {
          return Action::bet;
        }
        break;
      case Card::queen:
        if (is_facing_bet) {
          return Action::call;
        } else {
          return Action::check;
        }
        break;
      case Card::king:
        return is_facing_bet ? Action::call : Action::check;
        break;
      default:
        throw std::runtime_error("This should never happen!");
        break;
    }
  }

  throw std::runtime_error("This should never happen!");
}

template<uint enumerator, uint denominator>
Action kuhn(const std::vector<Action>& actions, const Card& card) {
  assert(denominator > 0);
  double alpha = static_cast<double>(enumerator) /
                 static_cast<double>(denominator);
  bool is_alice = actions.size() % 2 == 0;
  bool is_facing_bet{false};

  if (is_alice) {
    is_facing_bet = (actions.size() == 2) && actions[1] == Action::bet;
    switch (card) {
      case Card::jack:
        if (is_facing_bet) {
          return Action::fold;
        } else {
          return random() < alpha ?
                 Action::bet : Action::check;
        }
        break;
      case Card::queen:
        if (is_facing_bet) {
          return random() < alpha ?
                 Action::call : Action::fold;
          return Action::call;
        } else {
          return Action::check;
        }
        return is_facing_bet ? Action::call : Action::check;
        break;
      case Card::king:
        if (is_facing_bet) {
          return Action::call;
        } else {
          return random() < 3 * alpha ?
                 Action::bet : Action::check;
        }
        break;
      default:
        throw std::runtime_error("This should never happen!");
        break;
    }
  } else {
    is_facing_bet = (actions.size() == 1) && actions[0] == Action::bet;
    switch (card) {
      case Card::jack:
        if (is_facing_bet) {
          return Action::fold;
        } else {
          return random() < 1./3. ?
                 Action::bet : Action::check;
        }
        break;
      case Card::queen:
        if (is_facing_bet) {
          return random() < 1./3. ?
                 Action::call : Action::fold;
        } else {
          return Action::check;
        }
        break;
      case Card::king:
        return is_facing_bet ? Action::call : Action::bet;
        break;
      default:
        throw std::runtime_error("This should never happen!");
        break;
    }
  }

  throw std::runtime_error("This should never happen!");
}

};  // namespace strategy

};  // namespace


int main() {
  const bool verbose{false};
  size_t wins{0}, losses{0};
  size_t total_hands{0};
  std::vector<std::vector<int>> history;

  for (size_t round{0}; round < 10; round++) {
    history.push_back({0});
    int stack{0};
    size_t hand{0};
    auto hero_strategy = &strategy::kuhn<1, 3>;
    auto villain_strategy = &strategy::counter_exploit;
    for ( ; hand < 200; hand++) {
      total_hands++;
      auto cards = shuffle();
      int wins{0};
      if (hand % 2 == 0) {  // we are Alice
        wins = std::get<0>(play_round(cards[0],
                                      cards[1],
                                      hero_strategy,
                                      villain_strategy,
                                      verbose));
      } else {  // we are Bob
        wins = std::get<1>(play_round(cards[0],
                                      cards[1],
                                      villain_strategy,
                                      hero_strategy,
                                      verbose));
      }
      stack += wins;
      history.back().push_back(stack);
      if (verbose) {
        std::cout << "Hand: " << std::setw(5) << hand
                  << "  Wins: " << std::setw(5) << wins
                  << "  Stack: " << std::setw(5) << stack
                  << std::endl;
      }
    }

    if (stack > 0) {
      wins++;
    } else {
      losses++;
    }

    std::cout << "Round: " << std::setw(5) << round
              << "  Wins: " << std::setw(5) << wins
              << "  Stack: " << std::setw(5) << stack
              << "  Hands: " << std::setw(5) << hand
              << std::endl;
  }
  auto total_stack = std::accumulate(history.back().begin(),
                                     history.back().end(),
                                     0);
  std::cout << "Wins: " << std::setw(5) << wins
            << "\nLosses: " << std::setw(5) << losses
            << "\nTotal Wins: " << std::setw(5) << total_stack
            << "\nWins Per hand: " << std::setw(5)
            << static_cast<double>(total_stack) /
               static_cast<double>(total_hands)
            << std::endl;

  std::ofstream output_file;
  output_file.open("../data/kuhn__vs__counter_exploit.txt");
  for (auto game : history) {
    for (auto it = game.begin(); it != game.end(); it++) {
      output_file << *it;
      if (std::next(it) != game.end()) {
        output_file << ",";
      }
    }
    output_file << "\n";
  }
  output_file.close();
}
