#include <algorithm>
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <queue>

using namespace std;

typedef void (*TaskFunc)(LPVOID);

typedef struct Task {
    TaskFunc function;
    LPVOID lpParam;
} Task;

typedef struct TaskQueue {
    CRITICAL_SECTION critSection;
    queue<Task> tasks;
} TaskQueue;

typedef struct ThreadPool {
    vector<HANDLE> threads;
    vector<vector<Task>> threadTasks;
    int size;
} ThreadPool;

bool getNextTask(TaskQueue* queue, Task* task) {
    EnterCriticalSection(&queue->critSection);
    if (queue->tasks.empty()) {
        LeaveCriticalSection(&queue->critSection);
        return false; // No task available
    }
    *task = queue->tasks.front();
    queue->tasks.pop();
    LeaveCriticalSection(&queue->critSection);
    return true; // Task found
}

DWORD WINAPI threadProc(void* lpParam) {
    vector<Task>* tasks = (vector<Task>*)lpParam;
    for (size_t i = 0; i < tasks->size(); ++i) {
        tasks->at(i).function(tasks->at(i).lpParam);
    }
    return 0;
}

void sortStrings(void* lpParam) {
    vector<string>* strings = (vector<string>*)lpParam;
    sort(strings->begin(), strings->end());
}

int main() {
    int threadCount;
    cout << "Enter number of threads: ";
    cin >> threadCount;

    ifstream inputFile;
    inputFile.open("C:/Users/madam/CLionProjects/sys-prog/lapa5/input.txt");
    if (!inputFile.is_open()) {
        cerr << "Error opening input file!" << endl;
        return -1;
    }

    vector<string> strings;
    string line;
    while (getline(inputFile, line)) {
        strings.push_back(line);
    }
    inputFile.close();

    TaskQueue queue;
    InitializeCriticalSection(&queue.critSection);

    ThreadPool* pool = new ThreadPool();
    pool->size = threadCount;
    pool->threads.resize(threadCount);
    pool->threadTasks.resize(threadCount);

    vector<vector<string> > parts(threadCount);
    int partSize = strings.size() / threadCount;
    int remaining = strings.size() % threadCount;

    int start = 0;
    for (int i = 0; i < threadCount; ++i) {
        int currentSize = partSize + (i < remaining ? 1 : 0);
        parts[i] = vector(strings.begin() + start, strings.begin() + start + currentSize);
        start += currentSize;

        EnterCriticalSection(&queue.critSection);

        Task task;
        task.function = sortStrings;
        task.lpParam = &parts[i];

        queue.tasks.push(task);

        LeaveCriticalSection(&queue.critSection);
    }

    Task task;
    int threadIndex = 0;
    while (getNextTask(&queue, &task)) {
        pool->threadTasks[threadIndex].push_back(task);
        threadIndex = (threadIndex + 1) % threadCount;
    }

    for (int i = 0; i < pool->size; ++i) {
        HANDLE threadHandle = CreateThread(
            NULL,  //SECURITY_ATTRIBUTES, которая определяет, может ли возвращаемый дескриптор наследоваться дочерними процессами
            0, //Начальный размер стека в байтах
            threadProc, //Указатель на определяемую приложением функцию, выполняемую потоком.
            &pool->threadTasks[i], //Указатель на переменную, передаваемую потоку.
            0, //Флаги, управляющие созданием потока.
            NULL //идентификатор потока
        );
        pool->threads.push_back(threadHandle);
    }

    WaitForMultipleObjects(
        pool->threads.size(), //The number of object handles in the array pointed to by lpHandles.
        pool->threads.data(), //An array of object handles.
        TRUE, //TRUE, the function returns when the state of all objects in the lpHandles array is signaled.
        INFINITE); //function will return only when the specified objects are signaled.

    vector<string> result;
    for (const auto& part : parts) {
        result.insert(result.end(), part.begin(), part.end());
    }
    sort(result.begin(), result.end());

    ofstream outputFile;
    outputFile.open("C:/Users/madam/CLionProjects/sys-prog/lapa5/output.txt");
    if (!outputFile.is_open()) {
        cerr << "Error opening output file!" << endl;
        return -1;
    }

    outputFile << "Sorted strings:\n";
    for (const auto& str : result) {
        outputFile << str << endl;
    }
    outputFile.close();

    for (int i = 0; i < pool->size; ++i) {
        if (pool->threads[i]) {
            CloseHandle(pool->threads[i]);
        }
    }

    free(pool->threads.data());
    free(pool);

    DeleteCriticalSection(&queue.critSection);

    return 0;
}
