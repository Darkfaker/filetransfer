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

/*�ṹ���洢˫���ܵ��ض�����Ϣ��*/
struct sockpair_chain {
	FILE *stream;               ///<ָ��˫���ܵ�����ָ��
	pid_t pid;                  ///< ������Ľ���ID
	struct sockpair_chain *next;   ///< ָ�����е���һ��ָ��
};

/**Typedefʹ�ṹ������ʹ��. */
typedef struct sockpair_chain sockpiar_t;

/* �򿪵�˫�عܵ�������ͷ. */
static sockpiar_t *chain_hdr;

FILE *dpopen(const char *command)
{
	int fd[2], parent, child;
	pid_t   pid;
	FILE    *stream;
	sockpiar_t *chain;

	/* ʹ��BSD socketpair���ô���һ��˫���ܵ� */
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0)
		return NULL;
	parent = fd[0];
	child = fd[1];

	/*�ֲ���̲�������Ƿ�ɹ� */
	if ((pid = fork()) < 0) {
		close(parent);
		close(child);
		return NULL;
	}

	if (pid == 0) {                         /* ���� */
		/* �ر���һ�� */
		close(parent);
		/* ���Ƶ���׼����ͱ�׼��� */
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
		/*�ڽ��临�Ƶ���׼I / O֮��ҲҪ�رմ˶� */
		close(child);
		/* ��popenһ���ر�������ǰ�򿪵Ĺܵ��� */
		for (chain = chain_hdr; chain != NULL; chain = chain->next)
			close(fileno(chain_hdr->stream));
		/* ͨ��shִ������ */
		char *myargv[10] = { 0 };

		printf("%s\n", command);
		execl("/bin/sh", "sh", "-c", command, NULL);
		printf("execv error\n");
		/* ���execlʧ�ܣ����˳��ӽ���*/
		_exit(127);
	}
	else {                                /* �� */
		/* �ر���һ�� */
		close(child);
		/* �ùܵ����ļ���������һ���µ��� */
		stream = fdopen(parent, "r+");
		if (stream == NULL) {
			close(parent);
			return NULL;
		}
		/* Ϊdpipe_t�ṹ�����ڴ� */
		chain = (sockpiar_t *)malloc(sizeof(sockpiar_t));
		if (chain == NULL) {
			fclose(stream);
			return NULL;
		}
		/*�洢dpclose�ı�Ҫ��Ϣ������������ͷ */
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

int dpclose(FILE *stream)
{
	int status;
	pid_t pid, wait_res;
	sockpiar_t *cur;
	sockpiar_t **ptr;

	/* ����������ʼ���� */
#ifdef _REENTRANT
	pthread_mutex_lock(&chain_mtx);
#endif
	ptr = &chain_hdr;
	while ((cur = *ptr) != NULL) {         /* ����������ĩ�� */
		if (cur->stream == stream) {        /*������ */
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
	errno = EBADF;              /*���û���ҵ���������*/
	return -1;
}

int dphalfclose(FILE *stream)
{
	/*ȷ���������ݶ���ˢ�� */
	if (fflush(stream) == EOF)
		return -1;
	/* �رչܵ�������д */
	return shutdown(fileno(stream), SHUT_WR);
}

