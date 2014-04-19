enum color_t {BLACK, RED, BLUE};
enum bumper_t {NONE, FRONT, RIGHT, LEFT, BACK};
enum state_t {
  START_STATE       = (1 << 0),
  LINE_FOLLOW_STATE = (1 << 1),
  LINE_SEARCH_STATE = (1 << 2),
  FIRST_BUMP_STATE  = (1 << 3),
  SECOND_BUMP_STATE = (1 << 4),
  END_OF_LINE_STATE = (1 << 5)
};
enum search_state_t {START, PIVOT_LEFT, PIVOT_RIGHT, PIVOT_TO_ORIG_POS, REVERSE, DONE};

