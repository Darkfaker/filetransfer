#ifndef PROCESSPOOL_H
#define PROCESSPOOL_H
#define _CRT_SECURE_NO_WARNINGS
#include "STL"
#include "LinuxFun"
#include "MD5.h"
#include "sockpair.h"
using namespace std;

/*
����һ���ӽ��̵���
m_pid ��Ŀ���ӽ��̵�pid    m_pipeid �Ǹ��ӽ��̵Ĺܵ�
*/
class process
{
public:
	process() : m_pid(-1){}
public:
	pid_t m_pid;                 //Ŀ���ӽ��̵�PID
	int m_pipefd[2];          //���ӽ��̼�ͨ���õĹܵ�
};

/*���̳��� */
template< typename T >
class processpool
{
public:
	//����ģʽ���ⲿ�ӿ�
	static processpool<T>* create(int listenfd, int process_number = 8)
	{
		if (!m_instance)
		{
			m_instance = new processpool< T >(listenfd, process_number);
		}
		return m_instance;
	}
	~processpool()
	{
		delete[] m_sub_process;
	}
	//�������̳�
	void run();

private:
	//����ģʽcreate ����Ϊ˽��
	processpool(int listenfd, int process_number = 8);

	void setup_sig_pipe();
	void run_parent();
	void run_child();

	//���̵����������ӽ�������
	static const int MAX_PROCESS_NUMBER = 16;
	//ÿ���ӽ�������ܴ���Ŀͻ�����
	static const int USER_PER_PROCESS = 65536;
	//epoll ����ܴ�����¼���
	static const int MAX_EVENT_NUMBER = 10000;
	//���̳��еĽ�������
	int m_process_number;
	//�ӽ����ڳ��е����
	int m_idx;
	//ÿ�����̶���һ�� epoll �ں��¼���
	int m_epollfd;
	//����sockfd
	int m_listenfd;
	//�ӽ���ͨ�� m_stop �������Ƿ�ֹͣ����
	int m_stop;
	//���������ӽ��̵�������Ϣ
	process* m_sub_process;
	//���̳صľ�̬ʵ��
	static processpool< T >* m_instance;
};







//��̬ʵ������������
template< typename T >
processpool< T >* processpool< T >::m_instance = NULL;



//���ڴ����źŵĹܵ�
static int sig_pipefd[2];


//���� sockfd Ϊ������
static int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
	return fd;
}
//epoll_ctl ��� fd ������Ϊ������
static void addfd(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}
//epoll_ctl �Ƴ� fd �ϵ�����ע���¼�
static void removefd(int epollfd, int fd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	close(fd);
}
//���ӽ��̼�Ĺܵ����� ˫��ͨ��
static void sig_handler(int sig)
{
	int save_errno = errno;
	int msg = sig;
	send(sig_pipefd[1], (char*)&msg, 1, 0);
	errno = save_errno;
}
//����źŴ�����
static void addsig(int sig, void(handler)(int), bool restart = true)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;//ָ���źŴ�����
	if (restart)
	{
		sa.sa_flags |= SA_RESTART;//���ó�����յ��źź����µ��ñ����ź���ֹ��ϵͳ����
	}
	sigfillset(&sa.sa_mask);//sa_maskΪ���̵��źż����룬sigfillset���źż������������ź�
	assert(sigaction(sig, &sa, NULL) != -1);
}
/*
���̳صĹ��캯����listenfd �Ǽ��� sockfd �������ڴ������̳�֮ǰ�����������ӽ����޷�
ֱ������
*/
template< typename T >
processpool< T >::processpool(int listenfd, int process_number)
: m_listenfd(listenfd), m_process_number(process_number), m_idx(-1), m_stop(false)
{
	assert((process_number > 0) && (process_number <= MAX_PROCESS_NUMBER));
	//����Ҫ�����Ľ�������new �ӽ��̹������
	m_sub_process = new process[process_number];
	assert(m_sub_process);

	for (int i = 0; i < process_number; ++i)
	{
		//�������Ӽ��ȫ˫���ܵ�
		int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_sub_process[i].m_pipefd);
		assert(ret == 0);
		//fork �ӽ���
		m_sub_process[i].m_pid = fork();
		assert(m_sub_process[i].m_pid >= 0);
		if (m_sub_process[i].m_pid > 0)//����Ǹ�������ر� ����
		{
			close(m_sub_process[i].m_pipefd[1]);
			continue;
		}
		else
		{
			close(m_sub_process[i].m_pipefd[0]);//������ӽ�����ر� д��
			m_idx = i;
			break;
		}
	}
}
//ͳһʱ��Դ
template< typename T >
void processpool< T >::setup_sig_pipe()
{
	//���� epoll �ں��¼��� �� �źŹܵ�
	m_epollfd = epoll_create(5);
	assert(m_epollfd != -1);
	//��������֮���ȫ˫�����źŹܵ�
	int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
	assert(ret != -1);

	//����Ϊ������
	setnonblocking(sig_pipefd[1]);
	addfd(m_epollfd, sig_pipefd[0]);
	//�����źŴ�����
	addsig(SIGCHLD, sig_handler);
	addsig(SIGTERM, sig_handler);
	addsig(SIGINT, sig_handler);
	addsig(SIGPIPE, SIG_IGN);
}
//�������� m_idx ֵΪ -1 ���ӽ����� >= 0 �ݴ��жϸ��ӽ��̵ĵ���
template< typename T >
void processpool< T >::run()
{
	if (m_idx != -1)
	{
		run_child();
		return;
	}
	run_parent();
}
//�ӽ�����������
template< typename T >
void processpool< T >::run_child()
{
	setup_sig_pipe();
	//�ӽ��̸����Լ��� chain�ṹ�е� id ���ҵ������븸����ͨ�ŵĹܵ�
	int pipefd = m_sub_process[m_idx].m_pipefd[1];
	//�ӽ�����Ҫ�����ܵ��ļ������� pipieid ����Ϊ�����̽�ͨ������֪ͨ�ӽ��� accept ������
	addfd(m_epollfd, pipefd);

	//����epoll�����¼���
	epoll_event events[MAX_EVENT_NUMBER];
	//����65535 ���¼����������
	T* users = new T[USER_PER_PROCESS];
	assert(users);
	int number = 0;
	int ret = -1;
	//�ȴ��˳����� m_stop
	while (!m_stop)
	{
		number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR))
		{
			printf("epoll failure\n");
			break;
		}
		//number Ϊepoll_wait ���صľ����������ĸ���
		for (int i = 0; i < number; i++)
		{
			int sockfd = events[i].data.fd;
			if ((sockfd == pipefd) && (events[i].events & EPOLLIN))
			{
				int client = 0;
				//�Ӹ������ݹܵ��ж�ȡ���ݣ������������client������
				ret = recv(sockfd, (char*)&client, sizeof(client), 0);
				if (((ret < 0) && (errno != EAGAIN)) || ret == 0)
				{
					continue;
				}
				else//��ȡ�ɹ����ʾ�����û�����
				{
					struct sockaddr_in client_address;
					socklen_t client_addrlength = sizeof(client_address);
					//Ϊ���û� accept����
					int connfd = accept(m_listenfd, (struct sockaddr*)&client_address, &client_addrlength);
					if (connfd < 0)
					{
						printf("errno is: %d\n", errno);
						continue;
					}
					//����ܵ�����
					addfd(m_epollfd, connfd);
					//ģ�� T ������ṩinit��ʼ���������Գ�ʼ��һ�����ӣ�ʹ��connfd�������Ч��
					users[connfd].init(m_epollfd, connfd, client_address);
				}
			}
			//���Ϊ�źŹܵ������¼�
			else if ((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN))
			{
				int sig;
				char signals[1024];
				// 0 �Ŷ� ���ź�
				ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
				if (ret <= 0)//ret Ϊ��������������
				{
					continue;
				}
				else
				{
					//���ճɹ������ѭ������
					//��Ϊÿ���ź�ֵռһ���ֽ����԰��ֽ��������
					for (int i = 0; i < ret; ++i)
					{
						switch (signals[i])//�ж��ź�����
						{
						case SIGCHLD://�ӽ���״̬�����仯(�˳�����ͣ)
						{
										 pid_t pid;
										 int stat;
										 //waitpid����û�����˳����ӽ����򷵻� 0
										 while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
										 {
											 continue;
										 }
										 break;
						}
							//��ֹ����
						case SIGTERM://��ֹ���̣�kill����Ĭ�ϵ��ź�
						case SIGINT://ctrl + C �ж��˳�����
						{
										m_stop = true;
										break;
						}
						default:
						{
								   break;
						}
						}
					}
				}
			}
			//����������������ݣ����Ȼ���Ͽͻ������󣬵�����Ӧ���� �ӽ��̴���
			else if (events[i].events & EPOLLIN)
			{
				users[sockfd].process();
			}
			else
			{
				continue;
			}
		}
	}

	delete[] users;
	users = NULL;
	close(pipefd);
	//close( m_listenfd );
	//
	close(m_epollfd);
}
//��������������
template< typename T >
void processpool< T >::run_parent()
{
	//��ʼ���źŹܵ�
	setup_sig_pipe();
	//�����̼��� listenfd
	addfd(m_epollfd, m_listenfd);
	//�����ں��¼���
	epoll_event events[MAX_EVENT_NUMBER];
	int sub_process_counter = 0;
	int new_conn = 1;
	int number = 0;
	int ret = -1;

	//�ȴ� m_stop ������־
	while (!m_stop)
	{
		//epoll_wait ����
		number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR))
		{
			printf("epoll failure\n");
			break;
		}
		//number Ϊepoll_wait ���صľ����������ĸ���
		for (int i = 0; i < number; i++)
		{
			int sockfd = events[i].data.fd;
			if (sockfd == m_listenfd)//�����������ӵ���
			{
				//�����ӻ�ʹ�� ROUND ROBIN �ķ����������ӷ����һ���ӽ���
				int i = sub_process_counter;
				do
				{
					if (m_sub_process[i].m_pid != -1)
					{
						break;
					}
					i = (i + 1) % m_process_number;
				} while (i != sub_process_counter);

				if (m_sub_process[i].m_pid == -1)
				{
					m_stop = true;
					break;
				}
				sub_process_counter = (i + 1) % m_process_number;
				//send( m_sub_process[sub_process_counter++].m_pipefd[0], ( char* )&new_conn, sizeof( new_conn ), 0 );
				send(m_sub_process[i].m_pipefd[0], (char*)&new_conn, sizeof(new_conn), 0);
				printf("send request to child %d\n", i);
				//sub_process_counter %= m_process_number;
			}
			//�����̴�������յ��ź�
			else if ((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN))
			{
				int sig;
				char signals[1024];
				//���źŹܵ���ȡ�ź�
				ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
				if (ret <= 0)
				{
					continue;
				}
				else
				{
					//������ź���ѭ������
					for (int i = 0; i < ret; ++i)
					{
						//�ж��ź�����
						switch (signals[i])
						{
						case SIGCHLD:
						{
										pid_t pid;
										int stat;
										while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
										{
											for (int i = 0; i < m_process_number; ++i)
											{
												if (m_sub_process[i].m_pid == pid)
												{
													printf("child %d join\n", i);
													close(m_sub_process[i].m_pipefd[0]);
													m_sub_process[i].m_pid = -1;
												}
											}
										}
										m_stop = true;
										for (int i = 0; i < m_process_number; ++i)
										{
											if (m_sub_process[i].m_pid != -1)
											{
												m_stop = false;
											}
										}
										break;
						}
						case SIGTERM:
						case SIGINT:
						{
									   printf("kill all the clild now\n");
									   for (int i = 0; i < m_process_number; ++i)
									   {
										   int pid = m_sub_process[i].m_pid;
										   if (pid != -1)
										   {
											   kill(pid, SIGTERM);
										   }
									   }
									   break;
						}
						default:
						{
								   break;
						}
						}
					}
				}
			}
			else
			{
				continue;
			}
		}
	}

	//close( m_listenfd );
	close(m_epollfd);
}

#endif
