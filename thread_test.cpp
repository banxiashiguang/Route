#include <iostream>
#include <pthread.h>
using namespace std;

void *test(void *arg)
{
	while(1)
	{
		cout<<"I am the pthread test"<<endl;
	}
}

int main(int argc, char const *argv[])
{
	pthread_t pid;
	int rv = pthread_create(&pid, NULL, test, 0);
	cout<<"after pthread_create"<<endl;
	pthread_join(pid, NULL);//代码中如果没有pthread_join主线程会很快结束从而使整个进程结束，从而使创建的线程没有机会开始执行就结束了
	return 0;
}