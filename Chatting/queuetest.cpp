#include "MTQueue.h"
#include <process.h>

Queue<int> queue = Queue<int>();

unsigned WINAPI threadFunc(void* args){
	int i, val;
	for (i = 0; i < 100; i++){
		//printf("\n\nThread%d : ", ((int*)args)[0]);
		val = queue.getLast();
		queue.add(i);
		//std::cout << i << std::endl;
	}
	return 0;
}
int queuetest(){
	HANDLE hThread[2];
	int args[2][1] = {
		{ 1 }, { 2 }
	};
	hThread[0] = (HANDLE)_beginthreadex(NULL, 0, threadFunc, (void*)args[0], 0, NULL);
	hThread[1] = (HANDLE)_beginthreadex(NULL, 0, threadFunc, (void*)args[1], 0, NULL);
	Sleep(1000);
	int i;
	for (i = 0; i < 200; i++){
		std::cout << queue.pop() << std::endl;
		Sleep(10);
	}
	
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	
	return 0;
}

#if 1
int main(){
	queuetest();
}
#endif
