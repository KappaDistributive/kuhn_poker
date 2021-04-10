#include <array>
#include <cassert>
#include <iostream>
#include <vector>


namespace {


enum class Action {
  fold,
  check,
  call,
  bet,
};


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


int
calculate_pot(const std::vector<Action>& actions,
              bool verify_actions = true) {
  if (verify_actions && !actions_are_legal(actions)) {
    throw std::runtime_error("Encountered illegal sequence of actions!");
  }
  int pot{2};  // antees
  for (auto action : actions) {
    if (action == Action::bet || action == Action::call) {
      pot++;
    }
  }
  return pot;
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


int
play_round(const Card& alice_card,
           const Card& bob_card,
           Action(*alice_strategy)
             (const std::vector<Action>& moves, const Card& card),
           Action(*bob_strategy)
             (const std::vector<Action>& moves, const Card& card)) {
  std::vector<Action> actions;

  for (size_t move{0}; move < 4; move++) {
    Action action;
    if (move % 2 == 0) {
      action = alice_strategy(actions, alice_card);
    } else {
      action = bob_strategy(actions, bob_card);
    }
    actions.push_back(action);
    if (action == Action::fold) {
      break;
    }
  }
  return 0;
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

// Action naive(const std::vector<int>& bets, )
};  // namespace strategy

};  // namespace


int main() {
  std::cout << (Card::queen < Card::king) << std::endl;
}
