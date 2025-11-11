#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>

typedef enum {
    PRIO_IDLE = 0,
    PRIO_LOW = 1,
    PRIO_NORMAL = 2,
    PRIO_HIGH = 3,
    PRIO_CRITICAL = 4
} TaskPriority;

typedef enum {
    TASK_READY = 0,
    TASK_BLOCKED,
    TASK_SUSPENDED
} TaskState;

void task_create(void (*func)(void), TaskPriority priority);
void sched_start(void);
void task_delay(uint32_t ms);
void task_yield(void);
void task_set_priority(uint8_t task_id, TaskPriority priority);
uint8_t get_current_task_id(void);
TaskPriority get_task_priority(uint8_t task_id);
uint32_t millis(void);
uint32_t task_get_overruns(uint8_t task_id);
void task_exit(void);
uint8_t task_get_count(void);
void sched_kill_all_tasks(void);

typedef struct {
    uint8_t id;
    TaskState state;
    TaskPriority priority;
    uint32_t cpu_time;
} TaskInfo;

int get_task_list(TaskInfo* list, int max_tasks);

#define sleep_ms(x) task_delay(x)

#endif // SCHED_H
