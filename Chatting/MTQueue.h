#include "stdafx.h"


template <typename T>
class node{ //���ø����� ���� ��� Ŭ����
public:
	T value;
	node<T>* next;
};

template <typename T>
class Queue{
private:
	int count;
	node<T>* front;
	HANDLE queueMutex;
public:
	Queue(){
		count = 0;
		front = NULL;
		queueMutex = CreateMutex(NULL, FALSE, NULL); //Mutex doesn't have its name, nor does it belong to the creator,
													 //which will be the main in this case.
	}
	~Queue(){
		DWORD mutex = WaitForSingleObject(queueMutex, INFINITE);
		node<T>* n = front; //�μ� ��� ����
		node<T>* b = front; //���� ��� ����
		switch (mutex){
		case WAIT_OBJECT_0:
			while (count-->0){
				b = n->next;
				free(n);
				n = b;
			}
			ReleaseMutex(queueMutex);
			break;
		case WAIT_ABANDONED: //ť�� �ı��Ǳ� ���� �̹� �������� ���
			return;
		default:
			TCHAR buffer[1024];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0x0800, buffer, 1024, NULL);
			std::cout << buffer << std::endl;
		}
		
	}
	HANDLE getMutex(){ // Mutex Getter.
		return queueMutex;
	}

	void add(T data){//ť�� ���� �߰��մϴ�.
		DWORD mutex = WaitForSingleObject(
			queueMutex,
			INFINITE);
		node<T>* n = new node<T>();
		switch (mutex){
		case WAIT_OBJECT_0: //Caller got ownership of the mutex
			n->value = data;
			n->next = NULL;
			if (count == 0){
				front = n;
			}
			else{
				node<T>* tmp = front;
				node<T>* rear = NULL;
				while (tmp != NULL){
					rear = tmp;
					tmp = tmp->next;
				}
				rear->next = n;
			}
			count++;
			ReleaseMutex(queueMutex);
			break;
		case WAIT_TIMEOUT: //something must be wrong
			printf("\n--- Thread failed to write an object to queue---\n");
			printf("Reason: Timeout\n");
			return; //Just ignores the message
			break;
		default:
			TCHAR buffer[1024];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0x0800, buffer, 1024, NULL);
			std::cout << buffer << std::endl;

		}
	}
	T pop(){ //ť���� ���� ��������, �� ��Ʈ���� �����մϴ�.
		DWORD result = WaitForSingleObject(queueMutex, INFINITE);
		T val;
		node<T>* f = front;
		switch (result){
		case WAIT_OBJECT_0:
			if (front == NULL){
				//fprintf(stderr, "queue is empty!");
				ReleaseMutex(queueMutex);
				return NULL;
			}
			front = f->next;
			val = f->value;
			count--;
			delete f;
			ReleaseMutex(queueMutex);
			return val;
			break;
		default:
			TCHAR buffer[1024];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0x0800, buffer, 1024, NULL);
			std::cout << buffer << std::endl;
			break;
		}
	}

	T getFirst(){ //pop()�� ������, ť���� ��Ʈ���� �������� �ʽ��ϴ�.
		if (front == NULL){
			fprintf(stderr, "queue is empty!");
			return NULL; //-1
		}
		return front->value;
	}
	T getLast(){ //�������� �߰��� ���� �����ɴϴ�. �׽�Ʈ��.
		if (front == NULL){
			fprintf(stderr, "queue is empty!");
			return NULL; //-1
		}
		node<T>* tmp = front;
		node<T>* rear = NULL;
		while (tmp != NULL){
			rear = tmp;
			tmp = tmp->next;
		}
		return rear->value;
	}
	bool isEmpty(){
		if (count) return false;
		return true;
	}
};