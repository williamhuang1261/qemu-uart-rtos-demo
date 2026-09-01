#ifndef TASKS_APP_H
#define TASKS_APP_H

/* Creates the two producer tasks, one consumer task, and the queue they
 * share. Must be called before vTaskStartScheduler(). */
void app_tasks_create(void);

#endif /* TASKS_APP_H */
