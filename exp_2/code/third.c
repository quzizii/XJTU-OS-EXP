#include <stdio.h>
#include <stdlib.h>

#define PROCESS_NAME_LEN 32
#define MIN_SLICE 10
#define DEFAULT_MEM_SIZE 1024
#define DEFAULT_MEM_START 0

#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

int mem_size = DEFAULT_MEM_SIZE;
int ma_algorithm = MA_FF;
static int pid = 0;
int flag = 0;

struct free_block_type {
    int size;
    int start_addr;
    struct free_block_type *next;
};

struct allocated_block {
    int pid;
    int size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block *next;
};

struct free_block_type *free_block;
struct allocated_block *allocated_block_head;

/* 函数声明 */
struct free_block_type *init_free_block(int mem_size);
void display_menu();
void set_mem_size();
void set_algorithm();
void rearrange(int algorithm);
void rearrange_FF();
void rearrange_BF();
void rearrange_WF();
int allocate_mem(struct allocated_block *ab);
int compact_memory(int request_size);
int free_mem(struct allocated_block *ab);
int dispose(struct allocated_block *free_ab);
void new_process();
void kill_process();
struct allocated_block *find_process(int pid);
void display_mem_usage();
void do_exit();

int main() {
    char choice;
    pid = 0;
    free_block = init_free_block(mem_size);

    while (1) {
        //fflush(stdin);
        while ((getchar()) != '\n');

        display_menu();
        choice = getchar();

        switch (choice) {
            case '1':
                set_mem_size();
                break;
            case '2':
                set_algorithm();
                flag = 1;
                break;
            case '3':
                new_process();
                flag = 1;
                break;
            case '4':
                kill_process();
                flag = 1;
                break;
            case '5':
                display_mem_usage();
                flag = 1;
                break;
            case '0':
                do_exit();
                exit(0);
            default:
                break;
        }
    }

    return 0;
}

/* 初始化空闲块，默认为一块，可以指定大小及起始地址 */
struct free_block_type *init_free_block(int mem_size) {
    struct free_block_type *bound = (struct free_block_type *)malloc(sizeof(struct free_block_type));
    bound->size = 0;
    bound->start_addr = DEFAULT_MEM_SIZE;
    bound->next = NULL;

    struct free_block_type *fb;
    fb = (struct free_block_type *)malloc(sizeof(struct free_block_type));
    if (fb == NULL) {
        printf("No mem\n");
        return NULL;
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    // fb->next = bound;
    fb->next = NULL;

    return fb;
}

/* 显示菜单 */
void display_menu() {
    printf("\n\n");
    printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE);
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process\n");
    printf("4 - Terminate a process\n");
    printf("5 - Display memory usage\n");
    printf("0 - Exit\n");
    printf("please choice mode:");
}

/* 设置内存的大小 */
void set_mem_size() {
    int size;
    if (flag != 0) {
        printf("Cannot set memory size again\n");
        return;
    }
    printf("Total memory size =");
    scanf("%d", &size);
    if (size > 0) {
        mem_size = size;
        free_block->size = mem_size;
    }
    flag = 1;
}

/* 设置当前的分配算法 */
void set_algorithm() {
    int algorithm;
    printf("\n");
    printf("\t1 - First Fit\n");
    printf("\t2 - Best Fit\n");
    printf("\t3 - Worst Fit\n");
    printf("please choice mode:");
    scanf("%d", &algorithm);
    if (algorithm >= 1 && algorithm <= 3)
        ma_algorithm = algorithm;
    rearrange(ma_algorithm);
}

/* 按指定的算法整理内存空闲块链表 */
void rearrange(int algorithm) {
    switch (algorithm) {
        case MA_FF:
            rearrange_FF();
            break;
        case MA_BF:
            rearrange_BF();
            break;
        case MA_WF:
            rearrange_WF();
            break;
        default:
            break;
    }
}

/* 按 FF 算法重新整理内存空闲块链表 */
void rearrange_FF() {
    // 按起始地址升序排列
    struct free_block_type *current = free_block;
    struct free_block_type *next = NULL;

    while (current != NULL && current->next != NULL) {
        next = current->next;
        if (current->start_addr > next->start_addr) {
            // 交换节点
            int temp_size = current->size;
            int temp_start = current->start_addr;
            current->size = next->size;
            current->start_addr = next->start_addr;
            next->size = temp_size;
            next->start_addr = temp_start;
            current = free_block;
        } else {
            current = current->next;
        }
    }
}

/* 按 BF 算法重新整理内存空闲块链表 */
void rearrange_BF() {
    // 按大小升序排列
    struct free_block_type *current = free_block;
    struct free_block_type *next = NULL;

    while (current != NULL && current->next != NULL) {
        next = current->next;
        if (current->size > next->size) {
            // 交换节点
            int temp_size = current->size;
            int temp_start = current->start_addr;
            current->size = next->size;
            current->start_addr = next->start_addr;
            next->size = temp_size;
            next->start_addr = temp_start;
            current = free_block;
        } else {
            current = current->next;
        }
    }
}

/* 按 WF 算法重新整理内存空闲块链表 */
void rearrange_WF() {
    // 按大小降序排列
    struct free_block_type *current = free_block;
    struct free_block_type *next = NULL;

    while (current != NULL && current->next != NULL) {
        next = current->next;
        if (current->size < next->size) {
            // 交换节点
            int temp_size = current->size;
            int temp_start = current->start_addr;
            current->size = next->size;
            current->start_addr = next->start_addr;
            next->size = temp_size;
            next->start_addr = temp_start;
            current = free_block;
        } else {
            current = current->next;
        }
    }
}


/* 创建新的进程，主要是获取内存的申请数量 */
void new_process() {
    struct allocated_block *ab;
    int size;
    ab = (struct allocated_block *)malloc(sizeof(struct allocated_block));
    if (!ab)
        exit(-5);
    ab->next = NULL;
    pid++;
    sprintf(ab->process_name, "PROCESS-%02d", pid);
    ab->pid = pid;
    printf("Memory for %s:", ab->process_name);
    scanf("%d", &size);
    if (size > 0)
        ab->size = size;
    int ret = allocate_mem(ab);
    if ((ret == 1) && (allocated_block_head == NULL)) {
        allocated_block_head = ab;
    } else if (ret == 1) {
        ab->next = allocated_block_head;
        allocated_block_head = ab;
    } else if (ret == -1) {
        printf("Allocation fail\n");
        free(ab);
    }
}


/* 删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点 */
void kill_process() {
    int pid;
    printf("Kill Process, pid=");
    scanf("%d", &pid);
    struct allocated_block *ab = find_process(pid);
    if (ab) {
        free_mem(ab);
        dispose(ab);
    }
}
/* 在已分配块链表中查找具有给定 PID 的进程 */
struct allocated_block *find_process(int pid) {
    struct allocated_block *current = allocated_block_head;
    while (current != NULL) {
        if (current->pid == pid) {
            return current;  // 找到匹配的进程
        }
        current = current->next;
    }
    return NULL;  // 未找到匹配的进程
}


/* 分配内存模块 */
int allocate_mem(struct allocated_block *ab) {
    int request_size = ab->size;
    struct free_block_type *current = free_block;
    struct free_block_type *previous = NULL;

    while (current != NULL) {
        if (current->size >= request_size) {
            // 找到合适的空闲块
            ab->start_addr = current->start_addr;
            
            if (current->size == request_size) {
                // 如果空闲块大小正好等于请求大小，直接分配
                if (previous == NULL) {
                    // 分配的是第一个空闲块
                    free_block = current->next;
                } else {
                    // 分配的是中间或最后一个空闲块
                    previous->next = current->next;
                }
                free(current);
            } else {
                // 空闲块大小大于请求大小，需要拆分，并将多余的部分保留在空闲块中
                current->start_addr += request_size;
                current->size -= request_size;
            }

            return 1; // 分配成功
        } else {
            previous = current;
            current = current->next;
        }
    }

    // 找不到合适的空闲块，采用内存紧缩技术
    if (compact_memory(request_size) == 1) {
        // 重新调用自身，进行分配
        return allocate_mem(ab);
    }

    return -1; // 分配失败
}

/* 内存紧缩技术 */
int compact_memory(int request_size) {
    struct allocated_block *current2 = allocated_block_head;
    int current_addr = 0;
    // 合并所有已分配块到低位
    while (current2 != NULL) {
        current2->start_addr = current_addr;
        current_addr += current2->size; 
        current2 = current2->next;
    }


    struct free_block_type *current1 = free_block;
    // 合并所有空闲块到高位
    while (current1 != NULL) {
        current1->start_addr = current_addr;
        current_addr += current1->size; 
        current1 = current1->next;
    }
    // 合并相邻空闲块
    struct free_block_type *current = free_block;
    struct free_block_type *previous = NULL;
    while (current != NULL && current->next != NULL) {
        struct free_block_type *next = current->next;
        if (current->start_addr + current->size == next->start_addr) {
            current->size += next->size;
            current->next = next->next;
            free(next);
            return 1;
        } else {
            current = current->next;
        }
    }
    return -1; // 无法进行内存紧缩
}


/* 释放 ab 所表示的已分配区，并进行可能的合并 */
int free_mem(struct allocated_block *ab) {
    struct free_block_type *new_free_block = (struct free_block_type *)malloc(sizeof(struct free_block_type));
    if (!new_free_block)
        return -1;

    new_free_block->size = ab->size;
    new_free_block->start_addr = ab->start_addr;
    new_free_block->next = NULL;

    // 在空闲区链表中找到合适的位置插入新的空闲块
    struct free_block_type *current = free_block;
    struct free_block_type *previous = NULL;

    while (current != NULL && current->start_addr < new_free_block->start_addr) {
        previous = current;
        current = current->next;
    }

    if (previous == NULL) {
        // 新的空闲块插入到链表头部
        new_free_block->next = free_block;
        free_block = new_free_block;
    } else {
        // 新的空闲块插入到链表中间或尾部
        new_free_block->next = current;
        previous->next = new_free_block;
    }

    // 合并相邻的空闲块
    current = free_block;
    while (current != NULL && current->next != NULL) {
        struct free_block_type *next = current->next;
        if (current->start_addr + current->size == next->start_addr) {
            // 合并相邻空闲块
            current->size += next->size;
            current->next = next->next;
            free(next);
            return 1;
        } else {
            current = current->next;
        }
    }
    // 重新排序空闲链表
    rearrange(ma_algorithm);

    return 0;
}

/* 释放 ab 数据结构节点 */
int dispose(struct allocated_block *free_ab) {
    struct allocated_block *current = allocated_block_head;
    struct allocated_block *previous = NULL;

    while (current != NULL && current != free_ab) {
        previous = current;
        current = current->next;
    }

    if (previous == NULL) {
        // 要释放的节点是链表头部
        allocated_block_head = allocated_block_head->next;
        free(free_ab);
    } else {
        // 要释放的节点在链表中间或尾部
        previous->next = free_ab->next;
        free(free_ab);
    }

    return 0;
}



/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
void display_mem_usage() {
    struct free_block_type *fbt = free_block;
    struct allocated_block *ab = allocated_block_head;
    if (fbt == NULL)
        return;

    printf("----------------------------------------------------------\n");

    /* 显示空闲区 */
    printf("Free Memory:\n");
    printf("%20s %20s\n", " start_addr", " size");
    while (fbt != NULL) {
        printf("%20d %20d\n", fbt->start_addr, fbt->size);
        fbt = fbt->next;
    }

    /* 显示已分配区 */
    printf("\nUsed Memory:\n");
    printf("%10s %20s %10s %10s\n", "PID", "ProcessName", "start_addr", " size");
    while (ab) {
        printf("%10d %20s %10d %10d\n", ab->pid, ab->process_name,
               ab->start_addr, ab->size);
        ab = ab->next;
    }

    printf("----------------------------------------------------------\n");
}

/* 退出程序，释放内存 */
void do_exit() {
    struct free_block_type *current = free_block;
    struct free_block_type *next = NULL;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    struct allocated_block *ab = allocated_block_head;
    struct allocated_block *next_ab = NULL;

    while (ab != NULL) {
        next_ab = ab->next;
        free(ab);
        ab = next_ab;
    }
}
