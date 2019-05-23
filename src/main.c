#include "common.h"
#include "commander.h"
#include "puzzle.h"
// 命令行参数处理

static void debug (command_t *self) {
    printf("开启debug模式\n");
    puzzle_set_debug();
}

static void array (command_t *self) {
    // printf("设置array,%s\n",self->arg);
    char* array_str = malloc((strlen(self->arg)+1)*sizeof(char));
    strcpy(array_str,self->arg);
    puzzle_set_init_state(array_str);
}

static void pos (command_t *self) {
    printf("设置pos,%s\n",self->arg);
    // 如果设置了pos,就不再随机
    char* pos_str = malloc((strlen(self->arg)+1)*sizeof(char));
    strcpy(pos_str,self->arg);
    puzzle_set_init_pos(pos_str);
}


int main(int argc,char** argv){
    command_t cmd; // commander主结构
    command_init(&cmd,argv[0],"0.0.1");
    command_option(&cmd, "-d", "--debug", "开启debug模式", debug);
    command_option(&cmd, "-a", "--array <arg>", "初始状态", array);
    command_option(&cmd, "-p", "--pos <arg>", "初始位置", pos);
    command_parse(&cmd, argc, argv);
    puzzle_run();
    command_free(&cmd);
    return 0;
}