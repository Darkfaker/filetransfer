#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#ifdef _REENTRANT
#include <pthread.h>
static pthread_mutex_t chain_mtx = PTHREAD_MUTEX_INITIALIZER;
#endif

#include "sockpair.h"

/*结构来存储双工管道特定的信息。*/
struct sockpair_chain {
	FILE *stream;               ///<指向双工管道流的指针
	pid_t pid;                  ///< 该命令的进程ID
	struct sockpair_chain *next;   ///< 指向链中的下一个指针
};

/**Typedef使结构更易于使用. */
typedef struct sockpair_chain sockpiar_t;

/* 打开的双重管道流的链头. */
static sockpiar_t *chain_hdr;

FILE *dpopen(const char *command){
	int fd[2], parent, child;
	pid_t   pid;
	FILE    *stream;
	sockpiar_t *chain;

	/* 使用BSD socketpair调用创建一个双工管道 */
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0)
		return NULL;
	parent = fd[0];
	child = fd[1];

	/*分叉过程并检查它是否成功 */
	if ((pid = fork()) < 0) {
		close(parent);
		close(child);
		return NULL;
	}

	if (pid == 0) {                         /* 孩子 */
		/* 关闭另一端 */
		close(parent);
		/* 复制到标准输入和标准输出 */
		if (child != STDIN_FILENO)
		if (dup2(child, STDIN_FILENO) < 0) {
			close(child);
			return NULL;
		}
		if (child != STDOUT_FILENO)
		if (dup2(child, STDOUT_FILENO) < 0) {
			close(child);
			return NULL;
		}
		/*在将其复制到标准I / O之后，也要关闭此端 */
		close(child);
		/* 像popen一样关闭所有以前打开的管道流 */
		for (chain = chain_hdr; chain != NULL; chain = chain->next)
			close(fileno(chain_hdr->stream));
		/* 通过sh执行命令 */
		char *myargv[10] = { 0 };

		printf("%s\n", command);
		execl("/bin/sh", "sh", "-c", command, NULL);
		printf("execv error\n");
		/* 如果execl失败，请退出子进程*/
		_exit(127);
	}
	else {                                /* 父 */
		/* 关闭另一端 */
		close(child);
		/* 用管道的文件描述符打开一个新的流 */
		stream = fdopen(parent, "r+");
		if (stream == NULL) {
			close(parent);
			return NULL;
		}
		/* 为dpipe_t结构分配内存 */
		chain = (sockpiar_t *)malloc(sizeof(sockpiar_t));
		if (chain == NULL) {
			fclose(stream);
			return NULL;
		}
		/*存储dpclose的必要信息，并调整链表头 */
		chain->stream = stream;
		chain->pid = pid;
#ifdef _REENTRANT
		pthread_mutex_lock(&chain_mtx);
#endif
		chain->next = chain_hdr;
		chain_hdr = chain;
#ifdef _REENTRANT
		pthread_mutex_unlock(&chain_mtx);
#endif
		return stream;
	}
}

int dpclose(FILE *stream){
	int status;
	pid_t pid, wait_res;
	sockpiar_t *cur;
	sockpiar_t **ptr;

	/* 搜索从链开始的流 */
#ifdef _REENTRANT
	pthread_mutex_lock(&chain_mtx);
#endif
	ptr = &chain_hdr;
	while ((cur = *ptr) != NULL) {         /* 不是链条的末端 */
		if (cur->stream == stream) {        /*流发现 */
			pid = cur->pid;
			*ptr = cur->next;
#ifdef _REENTRANT
			pthread_mutex_unlock(&chain_mtx);
#endif
			free(cur);
			if (fclose(stream) != 0)
				return -1;
			do {
				wait_res = waitpid(pid, &status, 0);
			} while (wait_res == -1 && errno == EINTR);
			if (wait_res == -1)
				return -1;
			return status;
		}
		ptr = &cur->next;                   /* Check next */
	}
#ifdef _REENTRANT
	pthread_mutex_unlock(&chain_mtx);
#endif
	errno = EBADF;              /*如果没有找到给定的流*/
	return -1;
}

int dphalfclose(FILE *stream){
	/*确保所有数据都被刷新 */
	if (fflush(stream) == EOF)
		return -1;
	/* 关闭管道进行书写 */
	return shutdown(fileno(stream), SHUT_WR);
}

