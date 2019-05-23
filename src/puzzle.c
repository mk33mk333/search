#include "puzzle.h"
#include "common.h"

static int is_use_debug = 0;
static int is_use_random_pos = 1;

// int init_arr[30] = {
//         2,1,0,3,3,2,
//         1,5,4,2,5,1,
//         1,3,1,0,4,5,
//         3,1,2,5,2,1,
//         2,4,3,1,0,2,
// };

int init_state[PUZZLE_STATE_UNIT] = {-1};

int init_pos[2] = {0,2};

int* puzzle_state_cache;  // 状态储存

int* puzzle_pos_cache; // 位置储存

int puzzle_state_used_capacity = 0;

int puzzle_pos_used_capacity = 0;

int puzzle_score_max = 0;

int puzzle_score_pos = -1; // 最高得分的位置

int puzzle_transform_pos = 0; // 进行变换的游标  游标的单位都是 sizeof int * 1   pos 是 state 的 1/3

int puzzle_temp_state[6] = {0}; // 每次先克隆到这里，符合条件在写入到正式

static void error(char* msg){
    fprintf(stderr, "[error::]%s\n", msg);
    exit(1);
}

static void print_arr(int*arr,int length){
    printf("[");
    for(int i=0;i<length;i++){
        printf("%d,",*(arr+i));
    }
    printf("]\n");
}

void puzzle_set_debug() {
    is_use_debug = 1;
}

void puzzle_set_init_state(char* str){
    // 把字符串拆成6个数
    int index = 0;
    if(!str){
        error("参数错误");
    }else{
        char* ptr = strtok(str,",");
        while(ptr != NULL){
            init_state[index] = atoi(ptr);
            index++;
            ptr = strtok(NULL,",");
        }
        if(index == 6){
            printf(
                "[input::]%d,%d,%d,%d,%d,%d\n",
                init_state[0],init_state[1],init_state[2],
                init_state[3],init_state[4],init_state[5]
            );
        }else{
            error("参数错误");
        }

    }

}
// 设置初始位置，设置此位置时，不启用随机起点
void puzzle_set_init_pos(char* str){
    int index = 0;
    if(!str){
        error("参数错误");
    }else{
        char* ptr = strtok(str,",");
        while(ptr != NULL){
            init_pos[index] = atoi(ptr);
            index++;
            ptr = strtok(NULL,",");
        }
        if(index == 2){
            printf(
                "[pos::]%d,%d\n",
                init_pos[0],init_pos[1]
            );
            is_use_random_pos = 0;
        }else{
            error("参数错误");
        }
    }

}

static void set_random_pos () {
    int a,b;
    srand((unsigned)time(NULL));
    a = rand();
    srand((unsigned)time(NULL));
    b = rand();
    init_pos[0] = a%5;
    init_pos[1] = b%4;
    printf(
        "[pos::]%d,%d\n",
        init_pos[0],init_pos[1]
    );
}

static void mem_init () {
    puzzle_state_cache = (int*)malloc(sizeof(int) * PUZZLE_STATE_UNIT * MAX_CAP);
    puzzle_pos_cache = (int*)malloc(sizeof(int) * PUZZLE_POS_UNIT * MAX_CAP);
}

static void move_to_temp (int* arr){
    for(int i = 0; i < PUZZLE_STATE_UNIT; i++){
        *(puzzle_temp_state + i) = *(arr + i);
    }
}

// 从经过的节点倒推路径 参数为 pos_cache中X元组的起始偏移量
static void get_path(int offset){
    int pos = offset;
    int index = 0;
    int posXArr[200] = {-1};
    int posYArr[200] = {-1};
    int offsetArr[200] = {-1};
    while (pos != -1 && index < 10000){
        if(index > 200){
            error("倒推路径太长");
        }
        posXArr[index] = *(puzzle_pos_cache + pos);
        posYArr[index] = *(puzzle_pos_cache + pos + 1);
        offsetArr[index] = *(puzzle_pos_cache + pos + 3);
        index++; 
        pos = *(puzzle_pos_cache + pos + 3);        
    }
    if(is_use_debug){
        for(int i=0;i<index;i++){
            printf("偏移量:%d,坐标:x:%d,y:%d\n",offsetArr[i],posXArr[i],posYArr[i]);
        }
    }
    // 正式输出
    printf("[path::]");
    for(int i=0;i<index;i++){
        printf("[%d,%d]",posXArr[i],posYArr[i]);
        if(i!=index-1){
            printf("-");
        }else{
            printf("\n");
        }
    }
    printf("[score::]%d\n",puzzle_score_max);
    
}

static int* write_state_cache (int* arr){
    // printf("STATE分配内存%d\n",puzzle_state_used_capacity);
    if(puzzle_state_used_capacity + PUZZLE_STATE_UNIT >= MAX_CAP * PUZZLE_STATE_UNIT){
        if(is_use_debug)printf("STATE内存用完退出\n");
        printf(
            "[result::]%d,%d,%d,%d,%d,%d\n",
            puzzle_state_cache[puzzle_score_pos],
            puzzle_state_cache[puzzle_score_pos+1],
            puzzle_state_cache[puzzle_score_pos+2],
            puzzle_state_cache[puzzle_score_pos+3],
            puzzle_state_cache[puzzle_score_pos+4],
            puzzle_state_cache[puzzle_score_pos+5]
        );
        get_path(puzzle_score_pos/PUZZLE_STATE_UNIT*PUZZLE_POS_UNIT);
        exit(0);
    }
    int* curPos = puzzle_state_cache + puzzle_state_used_capacity;
    for(int i = 0; i < PUZZLE_STATE_UNIT; i++){
        *(curPos + i) = *(arr + i);
    }
    puzzle_state_used_capacity += PUZZLE_STATE_UNIT;//更新用量
    return curPos;

}

static void write_pos_cache (int x,int y,int from,int prev_cache_offset){
    // printf("POS分配内存%d\n",puzzle_pos_used_capacity);
    if(puzzle_pos_used_capacity + PUZZLE_POS_UNIT >= MAX_CAP * PUZZLE_POS_UNIT){
        printf("POS内存用完退出\n");
        exit(0);
    }
    int* curPos = puzzle_pos_cache + puzzle_pos_used_capacity;
    *curPos = x;
    *(curPos + 1) = y;
    *(curPos + 2) = from; // 来的方向
    *(curPos + 3) = prev_cache_offset; // 来源在cache中的偏移量
    puzzle_pos_used_capacity += PUZZLE_POS_UNIT;//更新用量
    return ;
}


// 从6数初始化
static void init_from_bits(){
    for(int i = 0;i < PUZZLE_STATE_UNIT;i++){
        *(puzzle_state_cache + i) += init_state[i];
    }
    puzzle_state_used_capacity += PUZZLE_STATE_UNIT;
    *puzzle_pos_cache = *init_pos;
    *(puzzle_pos_cache + 1) = *(init_pos + 1);
    *(puzzle_pos_cache + 2) = -1; // 没有来的方向，定为-1
    *(puzzle_pos_cache + 3) = -1; // 没有来源位置，定为-1
    puzzle_pos_used_capacity += PUZZLE_POS_UNIT;
}



// void puzzle_init(){
//     // 从数组中检测 0 - 6 的位置
//     // int 最大 32 位，正好不会溢出，按位存放
//     for(int i=0;i<L_LEN;i++){
//         int val = *(init_arr + i);
//         int b = 1 << i;
//         *(puzzle_state_cache + val) += b;  // 不是加1，而是加 i 位 为 1 的二进制数 
//     }
//     puzzle_state_used_capacity += PUZZLE_STATE_UNIT;
//     *puzzle_pos_cache = *init_pos;
//     *(puzzle_pos_cache + 1) = *(init_pos + 1);
//     *(puzzle_pos_cache + 2) = -1; // 没有来的方向，定为-1
//     *(puzzle_pos_cache + 3) = -1; // 没有来源位置，定为-1
//     puzzle_pos_used_capacity += PUZZLE_POS_UNIT;
// }

static int check_sum (int* arr){
    int i = *arr | *(arr+1) | *(arr+2) | *(arr+3) | *(arr+4) | *(arr+5);
    if(i != 1073741823){
        printf("[error::]完整性校验未通过%d\n",i);
        exit(1);
    }
    
}

static int check_score_bit(int* arr){
    // 计算一个状态的评分
    int score = 0;
    for(int i=0;i<6;i++){
        // 获取一个二进制数的评分
        int bitNum = *(arr+i);
        for(int y = 0;y < 5;y++){
            for(int x = 0;x < 6;x++){
                int pos = y*6 + x; //搜索位
                int testNum = 1 << pos;
                if((bitNum & testNum) == testNum){ //搜索位为1，可以继续查找
                    // 在 4x5 的区域内都要做横向搜索
                    // 在 6x3 的区域内都要做纵向搜索
                    if(x<4){
                        // 横向搜索
                        int testNum2 = 1 << (y*6+x+1);
                        int testNum3 = 1 << (y*6+x+2);
                        if( ((bitNum & testNum2) == testNum2) && ((bitNum & testNum3) == testNum3) ){
                            // printf("横向三连 x:%d,y:%d,val:%d\n",x,y,i);
                            score ++ ;
                        }
                    }
                    if(y<3){
                        int testNum2 = 1 << ((y+1)*6+x);
                        int testNum3 = 1 << ((y+2)*6+x);
                        if( ((bitNum & testNum2) == testNum2) && ((bitNum & testNum3) == testNum3) ){
                            // printf("纵向三连 x:%d,y:%d,val:%d\n",x,y,i);
                            score ++ ;
                        }

                    }

                }
                // printf("%d-%d-%d\n",bitNum,testNum,bitNum & testNum);
            }

        }

    }

    if(score > puzzle_score_max){
        
        puzzle_score_max = score;
        puzzle_score_pos = arr - puzzle_state_cache;
        if(is_use_debug)printf("更新最高分%d,偏移量:%d\n",score,puzzle_score_pos);
    }

    return score;
}

// 转移 克隆一个矩阵 在x,y处 向1,2,3,4四个方向转移 会改变原数组，所以传入前自行克隆
// 转移成功返回1 失败返回-1
static int transform_bit (int* arr,int x,int y,int nextPos){
    // 能不能只通过6个二进制数完成评分和转移 否则就要存储矩阵数组
    // 评分 对一个二进制数评分  12次按位与 从 1 << 31 到 1 依次按位与 获取到位置P为1 查找另外5个位是否为1
    // 12 * 4 * 6  360次按位与
    // 转移 转移需要在6个二进制数中找到对应的位 ，然后直接更新两个位  6x2
    // printf("输入:");
    // print_arr(arr,6);
    int pos1 = y*6+x;
    int pos2 = nextPos;
    // printf("pos:%d,%d,x:%d,y:%d\n",pos1,pos2,x,y);
    // 在6个数中找到pos1 和 pos2
    int flag = 0; // 检验是否能够正确替换两个数
    int posNum1 = 1 << pos1;
    int posNum2 = 1 << pos2;
    // printf("x:%d,y:%d,pos1:%d,pos2:%d\n",x,y,pos1,pos2);
    for(int i = 0;i < 6;i++){
        int f1 = 0;
        int f2 = 0;
        if( (*(arr + i) & posNum1) == posNum1){
            // printf("在数字%d上找到pos1\n",i);
            f1 = 1;
        }
        if( (*(arr + i) & posNum2) == posNum2){
            // printf("在数字%d找到pos2\n",i);
            f2 = 1;
        }

        if(f1+f2 == 2){
            // 两个位置都在同一个颜色上，调换和不调换一样

        }else if(f1+f2 == 0){
            // 都不在此颜色上，无需改动

        }else if(f1 == 1){
            // 匹配到位置1 拿掉位置1的1，在位置2上放置1
            int temp = *(arr + i);
            // printf("1-%d",temp);
            temp = temp ^ posNum1; // 拿掉位置1的1
            // printf("2-%d",temp);
            temp = temp | posNum2; // 在位置2上加1
            // printf("3-%d",temp);
            *(arr + i) = temp;
            // printf("F1修改%d\n",i);
        }else if(f2 == 1){
            // 匹配到位置2 拿掉位置2的1，在位置1上放置1
            int temp = *(arr + i);
            temp = temp ^ posNum2; // 拿掉位置2的1
            temp = temp | posNum1; // 在位置1上加1
            *(arr + i) = temp;
            // printf("F2修改%d\n",i);
        }else{
            printf("[error::]意外错误\n");
            return -1;
        }

    }
    check_sum(arr);
    // printf("输出:");
    // print_arr(arr,6);
    
    return 1;
}



static int check_repeat (int* arr) {
    int pos = 0;
    int bingo = -1;
    // printf("%d,%d,%d,%d,%d,%d\n",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]);
    // printf("%d,%d,%d,%d,%d,%d\n",puzzle_state_cache[0],puzzle_state_cache[1],puzzle_state_cache[2],puzzle_state_cache[3],puzzle_state_cache[4],puzzle_state_cache[5]);
    while(bingo != 1 && pos < puzzle_state_used_capacity){ // 不能跟自己比
        // printf("%d,%d\n",pos,puzzle_state_used_capacity);
        if(
            (*(arr) == *(puzzle_state_cache + pos) )
            && (*(arr + 1) == *(puzzle_state_cache + pos + 1))
            && (*(arr + 2) == *(puzzle_state_cache + pos + 2))
            && (*(arr + 3) == *(puzzle_state_cache + pos + 3))
            && (*(arr + 4) == *(puzzle_state_cache + pos + 4))
            && (*(arr + 5) == *(puzzle_state_cache + pos + 5))   
        ){
            bingo = 1;
        }
        pos += 6;
    }

    return bingo;
}
// 检查是否可以寻路 和来源方向相反的话就退出
// 0 和 2 相反  1 和 3 相反
static int check_search (int x,int y, int direction,int from) {
    int pos1 = y*6+x;
    int pos2 = -1;
    if(
        (direction == 0 && from == 2)
        || (direction == 2 && from == 0)
        || (direction == 1 && from == 3)
        || (direction == 3 && from == 1)
    ){
        // printf("来源:%d,目标%d,相反跳过\n",from,direction);
        return pos2;
    }

    if(direction == 0 && y > 0){ // 向上
        pos2 = pos1 - 6;
    }else if(direction == 1 && x < 5){ // 向右
        pos2 = pos1 + 1;
    }else if(direction == 2 && y < 4){ //向下
        pos2 = pos1 + 6;
    }else if(direction == 3 && x > 0){ // 向左
        pos2 = pos1 - 1;
    }
    // if(pos2 == -1){
    //     // printf("无法到达，跳过。x:%d,y:%d,d:%d\n",x,y,direction);
    //     return -1;
    // } // 无法到达
    return pos2;

}

static void search(){
    // 循环起来 克隆 转移 评分
    // 根据游标在cache中循环前进，直到游标位置大于等于已用容量位置
    while(puzzle_transform_pos < puzzle_state_used_capacity){
        
        for(int i = 0;i < 4;i++){
            int d = puzzle_transform_pos / PUZZLE_STATE_UNIT * PUZZLE_POS_UNIT;
            int x = *(puzzle_pos_cache + d);  
            int y = *(puzzle_pos_cache + d + 1); 
            int from = *(puzzle_pos_cache + d + 2); // 来源方向
            int prev_cache_offset = d;

            int nextPos = check_search(x,y,i,from);
            if(nextPos != -1){
                move_to_temp(puzzle_state_cache + puzzle_transform_pos); // 将要转移的矩阵移动到临时内存中
                int result = transform_bit(puzzle_temp_state,x,y,nextPos); // 路径需要记录
                if(result == 1){
                    // int repeate = check_repeat(puzzle_temp_state);
                    int repeate = -1;
                    if(repeate == -1){
                        // 不重复的时候写入到cache中 
                        // 写入的时候需要记录一个当前状态对应的位置
                        int* _state = write_state_cache(puzzle_temp_state);
                        check_score_bit(_state);
                        // printf("来源坐标:%d,%d\n",x,y);
                        if(i == 0){
                            y = y - 1;
                        }else if(i == 1){
                            x = x + 1;
                        }else if(i == 2){
                            y = y + 1;
                        }else if(i == 3){
                            x = x - 1;
                        }
                        // printf("目标坐标:%d,%d,目标方向:%d,来源在cache中的偏移量:%d\n",x,y,i,prev_cache_offset);
                        write_pos_cache(x,y,i,prev_cache_offset);
                        
                    }
                }
            }
        }
        puzzle_transform_pos += PUZZLE_STATE_UNIT; // 转换游标前进
    }

}

void puzzle_run(){
    if(is_use_random_pos)set_random_pos(); // 使用随机起点
    mem_init(); // 初始化内存
    init_from_bits(); // 初始化首个状态
    check_score_bit(puzzle_state_cache); // 对初始态评分
    search();   
}



// 状态转移
// 怎么记状态效率最高  每个矩阵记成6个二进制数 分别为 1-6在30个位置等于1形成的二进制数
// 移动 -> 交换位置  其中两个数的某一位变换
// 对比是否一样 比较六个数
// 评价 怎么能够高效的评价 一个二进制数中1的个数是否大于3 哪些位为1 或者已知某一位为1，如何判断上下左右四个方向是否为1
// 记录路径 怎么记录路径效率最高 并且计算路径效率最高
// 如何防止无效的环路(什么是无效的环路)
// 怎么做路径记录的hash和状态的hash
// 找到评分大于N时结束 路径大于X时不再向更深的方向前进

