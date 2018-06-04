#include <windows.h>
#include <iostream>
#include <time.h>
#include <string>
#include <fstream>
#include <vector>


using namespace std;



class MonitorStack {
public:
	std::vector<int> stack;
	unsigned int count;
	unsigned int size;
public:
	MonitorStack(int);
	MonitorStack(const MonitorStack &obj);
	~MonitorStack();
	void push(int);
	void pop();
	bool empty() const;
	bool full() const;
};


MonitorStack::MonitorStack(int _size) {
	count = 0;
	size = _size;
	stack.resize(_size);
}

MonitorStack::MonitorStack(const MonitorStack &obj) {
	count = obj.count;
	size = obj.size;
	stack.resize(size);
	for (size_t i = 0; i < count; ++i) {
		stack[i] = obj.stack[i];
	}
}

MonitorStack::~MonitorStack() {
	stack.~vector();
}

void MonitorStack::push(int _element) {
	if (count < size) {
		printf("%s%d\n", "Произведено ", _element);
		stack[count] = _element;
		++count;
	}
}

void MonitorStack::pop() {
	if (count > 0) {
		printf("%s%d\n", "Употреблено ", stack[count - 1]);
		--count;
	}
}

bool MonitorStack::empty() const {
	return count == 0;
}

bool MonitorStack::full() const {
	return count == size;
}

struct Data {
	MonitorStack *ms;
	int size;
	int i;
	Data(MonitorStack& _ms, int _size, int _i) {
		ms = &_ms;
		size = _size;
		i = _i;
	}
};


CRITICAL_SECTION cs;


DWORD WINAPI producer(LPVOID lpParam) {
	srand((unsigned int)time(NULL));
	Data* data = (Data*)lpParam;
	int i = 0;
	int n;
	while (i < data->size) {
		if (!data->ms->full()) {
			EnterCriticalSection(&cs);
			if (!data->ms->full()) {
				n = rand() % data->size + 1;
				data->ms->push(n);
				i++;
			}
			LeaveCriticalSection(&cs);
		}
	}
	return 0;
}
DWORD WINAPI consumer(LPVOID lpParam) {
	srand((unsigned int)time(NULL));
	Data* data = (Data*)lpParam;
	int i = 0;
	while (i < data->size) {
		if (!data->ms->empty()) {
			EnterCriticalSection(&cs);
			if (!data->ms->empty()) {
				data->ms->pop();
				i++;
			}
			LeaveCriticalSection(&cs);
		}
	}
	return 0;
}


int main() {
	setlocale(0, "");
	srand((unsigned int)time(NULL));


	ifstream fin("input.txt");
	ofstream fout("output.txt");


	InitializeCriticalSection(&cs);


	try {
		printf("%s", "Введите размер stack: ");
		int sizeStack;
		cin >> sizeStack;
		if (sizeStack <= 0) {
			throw string("Error! Неверный размер stack!");
		}
		MonitorStack ms(sizeStack);


		printf("%s", "Введите количество producer: ");
		int countProducers;
		cin >> countProducers;
		if (countProducers <= 0) {
			throw string("Error! Неверное количество producer!");
		}
		HANDLE* producers = new HANDLE[countProducers];
		DWORD* idProducers = new DWORD[countProducers];
		vector<int> countNumProducers(countProducers);
		printf("%s", "Введите количество чисел каждого producer: ");
		for (auto i = 0; i < countProducers; ++i) {
			cin >> countNumProducers[i];
		}


		printf("%s", "Введите размер consumer: ");
		int countConsumers;
		cin >> countConsumers;
		if (countConsumers <= 0) {
			throw string("Error! Неверное количество consumer!");
		}
		HANDLE* consumers = new HANDLE[countConsumers];
		DWORD* idConsumers = new DWORD[countConsumers];
		vector<int> countNumConsumers(countConsumers);
		printf("%s", "Введите количество чисел каждого consumer: ");
		for (auto i = 0; i < countConsumers; ++i) {
			cin >> countNumConsumers[i];
		}


		Data* data;
		for (int i = 0; i < countProducers; ++i) {
			data = new Data(ms, countNumProducers[i], i);
			producers[i] = CreateThread(NULL, 0, producer, data, 0, &idProducers[i]);
		}


		for (int i = 0; i < countConsumers; ++i) {
			data = new Data(ms, countNumConsumers[i], i);
			consumers[i] = CreateThread(NULL, 0, consumer, data, 0, &idConsumers[i]);
		}


		WaitForMultipleObjects(countProducers, producers, TRUE, INFINITE);
		WaitForMultipleObjects(countConsumers, consumers, TRUE, INFINITE);


		for (int i = 0; i < countProducers; ++i) {
			CloseHandle(producers[i]);
		}
		for (int i = 0; i < countConsumers; ++i) {
			CloseHandle(consumers[i]);
		}
	}
	catch (string e) {
		cout << e << endl;
	}


	DeleteCriticalSection(&cs);


	system("pause");
	return 0;
}