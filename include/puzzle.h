#define L_LEN 30
#define PUZZLE_STATE_UNIT 6
#define PUZZLE_POS_UNIT 4 
#define MAX_CAP 1000000
// 设置debug模式
void puzzle_set_debug();
// 设置初始态
void puzzle_set_init_state(char* str);
// 设置初始位置
void puzzle_set_init_pos(char* str);
// 执行
void puzzle_run();