//多任务管理

#include	"bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

//申请内存空间
struct TASK *task_init(struct MEMMAN *memman){
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	taskctl = (struct TASKCTL *)memman_alloc_4k(memman, sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++ ){
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
	}
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].running = 0;
		taskctl->level[i].now = 0;
	}
	task = task_alloc();
	//活动中的标识
	task->flags = 2;
	//0.02秒
	task->priority = 2;
	//最高的level
	task->level = 0;
	task_add(task);
	//设置level
	task_switchsub();
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);
	return task;
}

struct TASK *task_alloc(void){
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++){
		if (taskctl->tasks0[i].flags == 0){
			task = &taskctl->tasks0[i];
			//正在使用中的标识
			task->flags = 1;
			//IF = 1
			task->tss.eflags = 0x00000202;
			//先设置为0
			task->tss.eax = 0;
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	//无可用
	return 0;
}

void task_run(struct TASK *task, int level, int priority){
	if (level < 0){
		level = task->level;
	}
	//优先级
	if (priority > 0){
		task->priority = priority;
	}
	//改变活动中的level
	if (task->flags == 2 && task->level != level){
		task_remove(task);
	}
	//从休眠中唤醒
	if (task->flags != 2){
		task->level = level;
		task_add(task);
	}
	//下次任务切换时检查level
	taskctl->lv_change = 1;
	return;
}

void task_switch(void){
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];
	tl->now++;
	if (tl->now == tl->running){
		tl->now = 0;
	}
	if (taskctl->lv_change != 0){
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	new_task = tl->tasks[tl->now];
	timer_settime(task_timer, new_task->priority);
	if (new_task != now_task){
		farjmp(0, new_task->sel);
	}
	return;
}

//任务休眠
void task_sleep(struct TASK *task){
	struct TASK *now_task;
	//如果处于活动状态
	if (task->flags == 2){
		now_task = task_now();
		task_remove(task);
		//如果休眠的说自己， 则需要进行任务切换
		if (task == now_task){
			task_switchsub();
			now_task = task_now();
			farjmp(0, now_task->sel);
		}
	}
	return;
}

struct TASK *task_now(void){
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}

void task_add(struct TASK *task){
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->running] = task;
	tl->running++;
	//活动中
	task->flags = 2;
	return;
}

void task_remove(struct TASK *task){
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];

	//寻找task所在的位置
	for (i = 0; i < tl->running; i++){
		if (tl->tasks[i] == task){
			break;
		}
	}

	tl->running--;
	if (i < tl->now){
		//需要移动成员，要相应处理
		tl->now--;
	}
	if (tl->now >= tl->running){
		//如果now的值出现异常，则需要修正
		tl->now = 0;
	}
	//休眠
	task->flags = 1;
	//移动
	for (; i < tl->running; i++){
		tl->tasks[i] = tl->tasks[i + 1];
	}
	return;
}

void task_switchsub(void){
	int i;
	//寻找最上层的level
	for (i = 0; i < MAX_TASKLEVELS; i++){
		if (taskctl->level[i].running > 0){
			break;
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}
