#include <array>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <vector>


namespace {


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
// TODO(stefan.mesken): define game
// TODO(stefan.mesken): logging
// TODO(stefan.mesken): simulate EV

namespace strategy {
// TODO(stefan.mesken): define strategy
/*
A strategy is a function
(bets, position?, card) -> Action
*/

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
};  // namespace strategy

};  // namespace


int main() {
  int stack{0};

  for (size_t hand{0}; hand < 10; hand++) {
    int wins{0};
    if (hand % 2 == 0) {
      wins = std::get<0>(play_round(Card::queen,
                                    Card::jack,
                                    &strategy::naive,
                                    &strategy::naive,
                                    true));
    } else {
      wins = std::get<1>(play_round(Card::king,
                                    Card::queen,
                                    &strategy::naive,
                                    &strategy::naive,
                                    true));
    }
    stack += wins;
    std::cout << "Hand: " << std::setw(5) << hand
              << "  Wins: " << std::setw(5) << wins
              << "  Stack: " << std::setw(5) << stack << std::endl;
  }
}
