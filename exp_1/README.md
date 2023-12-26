# 1.1 进程相关
## 1.1.1 完成图 1-1 程序的运行验证
根据示例，编写程序为：
```
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
int main()
{
        pid_t pid, pid1;
        pid = fork();
        if(pid<0){
                fprintf(stderr,"Fork Failed");
                return 1;
        }
        else if (pid == 0){
                pid1 = getpid();
                printf("child:pid = %d\n",pid);
                printf("child:pid1 = %d\n",pid1);
        }
        else{
                pid1 = getpid();
                printf("parent:pid = %d\n",pid);
                printf("parent:pid1 = %d\n",pid1);
                wait(NULL);
        }
        return 0;
}
```

多次运行程序，输出结果为：
```
[root@ecs-osexp firstexp]# ./exp11
parent:pid = 9487
child:pid = 0
parent:pid1 = 9486
child:pid1 = 9487

[root@ecs-osexp firstexp]# ./exp11
parent:pid = 9489
child:pid = 0
parent:pid1 = 9488
child:pid1 = 9489

[root@ecs-osexp firstexp]# ./exp11
parent:pid = 9491
child:pid = 0
parent:pid1 = 9490
child:pid1 = 9491

[root@ecs-osexp firstexp]# ./exp11
child:pid = 0
parent:pid = 9493
child:pid1 = 9493
parent:pid1 = 9492
```
可以看到，子进程pid返回一直为0，子进程pid1为子进程实际pid 9487；父进程pid返回为子进程pid 9847，父进程实际pid为9846，比子进程pid小1。多次运行程序发现，每次pid序号在不断递增。
## 1.1.2 添加一个全局变量并操作
对1.1.1中程序进行如下修改：定义全局变量value，在子进程中令其加1，在父进程中令其减2，最终return前为其加10，得到程序为：
```
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
int value=0;
int main()
{
        pid_t pid, pid1;
        pid = fork();
        if(pid<0){
                fprintf(stderr,"Fork Failed");
                return 1;
        }
        else if (pid == 0){
                value++;
                printf("child value = %d\n",value);
        }
        else{
                value-=2;
                printf("parent value = %d\n", value);
                wait(NULL);
        }
        value += 10;
        printf("real value = %d\n",value);
        return 0;
}
```
输出为
```
[root@ecs-osexp firstexp]# ./exp12
parent value = -2
child value = 1
real value = 11
real value = 8
```
从输出可以看出，子进程拷贝了自己单独的value值，因此相当于子进程、父进程在分别操作自己的value值，同时可以看到子进程与父进程的执行顺序是随机的。
## 1.1.3 子进程中调用 system 和 exec 
单独定义system_call.c用于获取件进程号 PID，定义如下：
```
#include <stdio.h>
#include <unistd.h>
int main()
{
        pid_t pid = getpid();
        printf("system_call PID: %d\n",pid);
        return 0;
}
```
此处system与exec函数改变的地方差别不大，system函数需额外引入头文件（询问ChatGPT得知）代码见下：
 ```
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
        pid_t pid, pid1;
        pid = fork();
        if(pid<0){
                fprintf(stderr,"Fork Failed");
                return 1;
        }
        else if (pid == 0){
                pid1 = getpid();
                printf("child process PID: %d\n",pid1);
                system("./system_call");
        }
        else{
                pid1 = getpid();
                printf("parent procss PID: %d\n",pid1);
                wait(NULL);
        }
        return 0;
}
 ```
 ```
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
        pid_t pid, pid1;
        pid = fork();
        if(pid<0){
                fprintf(stderr,"Fork Failed");
                return 1;
        }
        else if (pid == 0){
                pid1 = getpid();
                printf("child process PID: %d\n",pid1);
                execl("./system_call", "system_call", (char *)NULL);
        }
        else{
                pid1 = getpid();
                printf("parent procss PID: %d\n",pid1);
                wait(NULL);
        }
        return 0;
}
 ```
system函数调用，程序输出为：
```
[root@ecs-osexp firstexp]# ./exp13
child process PID: 9616
parent procss PID: 9615
system_call PID: 9617
```
exec函数调用，程序输出为：
 ```
 [root@ecs-osexp firstexp]# ./exp14
parent procss PID: 9618
child process PID: 9619
system_call PID: 9619
 ```
 分析上述结果，子进程与父进程的pid与前文规律一致，并不奇怪。
 system调用中system_call得到的pid为9617，比子进程大1，表明创建了一个新的进程用于执行system_call。
而exec调用中system_call得到的pid为9619，与子进程一致，表明没有创建新的进程 
# 1.2 线程相关
## 1.2.1 线程创建与变量操作
代码为：
```
#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 2
#define NUM_OPERATIONS 100000

int sharedVariable = 0;

void *incrementThread(void *arg) {
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        sharedVariable += 100;
    }
    pthread_exit(NULL);
}

void *decrementThread(void *arg) {
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        sharedVariable -= 100;
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Create the increment and decrement threads
    pthread_create(&threads[0], NULL, incrementThread, NULL);
    pthread_create(&threads[1], NULL, decrementThread, NULL);

    // Wait for both threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Output the final value of the shared variable
    printf("Final sharedVariable value: %d\n", sharedVariable);

    return 0;
}
```
需注意编译时命令为：`gcc -o exp21 exp21.c -lpthread`.输出结果为：
```
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: 107300
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: 246400
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: -749700
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: -135400
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: 215300
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: -603300
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: 451400
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: -705400
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: -569700
[root@ecs-osexp second]# ./exp21
Final sharedVariable value: 401800
```
## 1.2.2 引入PV操作加锁
改写1.2.1中代码为：
```
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 2
#define NUM_OPERATIONS 100000

int sharedVariable = 0;
sem_t semaphore;

void *incrementThread(void *arg) {
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        sem_wait(&semaphore);
        sharedVariable += 100;
        sem_post(&semaphore);
    }
    pthread_exit(NULL);
}
void *decrementThread(void *arg) {
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        sem_wait(&semaphore);
        sharedVariable -= 100;
        sem_post(&semaphore);
    }
    pthread_exit(NULL);
}
int main() {
    sem_init(&semaphore, 0, 1);
    pthread_t threads[NUM_THREADS];

    // Create the increment and decrement threads
    pthread_create(&threads[0], NULL, incrementThread, NULL);
    pthread_create(&threads[1], NULL, decrementThread, NULL);
    
    // Wait for both threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Output the final value of the shared variable
    printf("Final sharedVariable value: %d\n", sharedVariable);

    return 0;
}
```
运行结果为：
```
[root@ecs-osexp second]# gcc -o exp22 exp22.c -lpthread
[root@ecs-osexp second]# ./exp22
Final sharedVariable value: 0
[root@ecs-osexp second]# ./exp22
Final sharedVariable value: 0
[root@ecs-osexp second]# ./exp22
Final sharedVariable value: 0
```
可以看到此时最终输出结果均为0，表明两线程运行次数一样。
## 1.2.3 调用 system 和 exec 
拷贝1.1中的system_call代码到1.2中，编写system与exec函数调用程序为：
```
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 2

void *incrementThread(void *arg) {
    pid_t pid = getpid();
    pthread_t tid = pthread_self();
    printf("thread1 tid = %lu, pid = %d\n", tid, pid);
    system("./system_call");
    pthread_exit(NULL);
}
void *decrementThread(void *arg) {
    pid_t pid = getpid();
    pthread_t tid = pthread_self();
    printf("thread1 tid = %lu, pid = %d\n", tid, pid);
    system("./system_call");
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Create the increment and decrement threads
    pthread_create(&threads[0], NULL, incrementThread, NULL);
    pthread_create(&threads[1], NULL, decrementThread, NULL);

    // Wait for both threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
```
```
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 2

void *incrementThread(void *arg) {
    pid_t pid = getpid();
    pthread_t tid = pthread_self();
    printf("thread1 tid = %lu, pid = %d\n", tid, pid);
    execl("./system_call", "system_call", (char *)NULL);
    pthread_exit(NULL);
}
void *decrementThread(void *arg) {
    pid_t pid = getpid();
    pthread_t tid = pthread_self();
    printf("thread1 tid = %lu, pid = %d\n", tid, pid);
    execl("./system_call", "system_call", (char *)NULL);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Create the increment and decrement threads
    pthread_create(&threads[0], NULL, incrementThread, NULL);
    pthread_create(&threads[1], NULL, decrementThread, NULL);

    // Wait for both threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
```
程序输出结果为：
```
[root@ecs-osexp second]# gcc -o exp23 exp23.c -lpthread
[root@ecs-osexp second]# ./exp23
thread1 tid = 281464707281376, pid = 10166
thread1 tid = 281464698827232, pid = 10166
system_call PID: 10169
system_call PID: 10170
[root@ecs-osexp second]# ./exp23
thread1 tid = 281461195993568, pid = 10171
thread1 tid = 281461187539424, pid = 10171
system_call PID: 10174
system_call PID: 10175
[root@ecs-osexp second]# ./exp23
thread1 tid = 281470409306592, pid = 10176
thread1 tid = 281470400852448, pid = 10176
system_call PID: 10180
system_call PID: 10179
[root@ecs-osexp second]# ./exp23
thread1 tid = 281467119727072, pid = 10181
thread1 tid = 281467111272928, pid = 10181
system_call PID: 10184
system_call PID: 10185
```
```
[root@ecs-osexp second]# gcc -o exp24 exp24.c -lpthread
[root@ecs-osexp second]# ./exp24
thread1 tid = 281461569810912, pid = 10191
system_call PID: 10191
[root@ecs-osexp second]# ./exp24
thread1 tid = 281467991814624, pid = 10194
system_call PID: 10194
[root@ecs-osexp second]# ./exp24
thread1 tid = 281458231669216, pid = 10197
thread1 tid = 281458223215072, pid = 10197
system_call PID: 10197
[root@ecs-osexp second]# ./exp24
thread1 tid = 281458988478944, pid = 10200
system_call PID: 10200
```
# 1.3 自旋锁实验
## 1.3.1 使用自旋锁实现互斥与同步

```
#include <stdio.h>
#include <pthread.h>

// 定义自旋锁结构体
typedef struct {
    int flag;
} spinlock_t;

// 初始化自旋锁
void spinlock_init(spinlock_t *lock) {
    lock->flag = 0;
}

// 获取自旋锁
void spinlock_lock(spinlock_t *lock) {
    while (__sync_lock_test_and_set(&lock->flag, 1)) {
        // 自旋等待，直到成功获取锁
    }
}

// 释放自旋锁
void spinlock_unlock(spinlock_t *lock) {
    __sync_lock_release(&lock->flag);
}

// 共享变量
int shared_value = 0;

// 线程函数
void *thread_function(void *arg) {
    spinlock_t *lock = (spinlock_t *)arg;

    for (int i = 0; i < 5000; ++i) {
        spinlock_lock(lock); // 获取自旋锁

        shared_value++; // 操作共享变量

        spinlock_unlock(lock); // 释放自旋锁
    }

    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    spinlock_t lock;

    // 初始化自旋锁
    spinlock_init(&lock);
    
    // 输出共享变量的值
    printf("Shared Value: %d\n", shared_value);

    // 创建两个线程
    pthread_create(&thread1, NULL, thread_function, &lock);
    pthread_create(&thread2, NULL, thread_function, &lock);

    // 等待线程结束
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // 输出共享变量的值
    printf("Shared Value: %d\n", shared_value);

    return 0;
}

```
程序输出结果为：
```
[root@ecs-osexp second]# gcc -o exp3 exp3.c -lpthread
[root@ecs-osexp second]# ./exp3
Shared Value:0
Shared Value: 10000
[root@ecs-osexp second]# ./exp3
Shared Value:0
Shared Value: 10000
[root@ecs-osexp second]# ./exp3
Shared Value:0
Shared Value: 10000
[root@ecs-osexp second]# ./exp3
Shared Value:0
Shared Value: 10000
```