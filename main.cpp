#include <iostream>
#include <fstream>
#include <cmath>
#include <map>
#include <vector>
#include <iomanip>
#include <set>
#include <queue>
#include "task.h"
#include <string.h>
#include "tokenizer.h"

using namespace std;

//=========Global Variable=========
int taskCount;          // the number of tasks
int terminated = 0;     // the number of terminated tasks
int resourceCount;      // the number of resource
map<int,int> resource;   // the number of each resource available
vector<task*> tasks;    // tasks
vector<task*> blocked;   // tasks which are blocked
int cycle = 0;
bool deadlocked = true;

void printOutput() {
    int runTotal = 0, waitTotal = 0;
    for (int i = 0; i < tasks.size(); i++) {
        printf("Task %i:\t\t", (i+1));
        if (tasks[i]->status == "aborted") {
            cout << "aborted  " << endl;
        } else {
            runTotal += tasks[i]->timeTaken;
            waitTotal += tasks[i]->waitingTime;
            printf("%i %i %i%%\n", tasks[i]->timeTaken, tasks[i]->waitingTime, (int)round(tasks[i]->waitingTime / (double)tasks[i]->timeTaken * 100));
        }
    }
    printf("Total:\t\t%i %i %i%%\n\n", runTotal, waitTotal, (int)round(waitTotal / (double)runTotal * 100));
}

void calWaitingTime() {
    for (int i = 0; i < blocked.size(); i++) {
        blocked[i]->waitingTime++;
    }
}

void readTasks(string file) {
    tokenizer* parser = new tokenizer(file);
    taskCount = stoi(parser->getToken());
    resourceCount = stoi(parser->getToken());
    string type;
    int taskNum, delay, resourceType, resourceReq;
    for (int i = 0; i < resourceCount; i++) {
        resourceType = stoi(parser->getToken());
        resource[i+1] = resourceType;
    }
    for (int i = 0; i < taskCount; i++) {
        task* temp = new task(i+1);
        tasks.push_back(temp);
    }
    while (parser->nextToken()) {
        type = parser->getToken();
        taskNum = stoi(parser->getToken());
        delay = stoi(parser->getToken());
        resourceType = stoi(parser->getToken());
        resourceReq = stoi(parser->getToken());
        task::activity* temp = new task::activity(type, delay, resourceType, resourceReq);
        tasks[taskNum - 1]->activities.push_back(temp);
    }
}
void printTask() {
    cout<<"========="<<endl;
    for (vector<task*>::iterator iter = blocked.begin(); iter != blocked.end(); iter++) {
        cout << (*iter)->taskNum <<"    "<<(*iter)->status<<endl;
    }
    cout<<"========="<<endl;
}
void checkBlockedTasks() {
    if (blocked.size() > 0) {
        cout<<"First check blocked tasks." <<endl;
        for (vector<task*>::iterator iter = blocked.begin(); iter != blocked.end(); ) {
            task::activity* curAct = (*iter)->activities[(*iter)->curIndex];
            if (resource[curAct->resourceType] >= curAct->numberReq) {
                (*iter)->curIndex++;
                (*iter)->status = "abort to active";
                resource[curAct->resourceType] -= curAct->numberReq;
                cout<<"     Task"<<(*iter)->taskNum<<"'s pending request is granted"<<endl;
                blocked.erase(iter);
                deadlocked = false;
            } else {
                cout<<"     Task"<<(*iter)->taskNum<<"'s request cannot be granted"<<endl;
                iter++;
            }
        }
    }
    //printTask();
}

void printResourceStatus() {
    cout<<"Available resource: ";
    for (int i = 1; i <= resourceCount; i++) {
        cout<<resource[i]<<" ";
    }
    cout<<"At cycle "<<cycle + 1<<endl;
}


void abortTask() {
    while (deadlocked) {
            for (vector<task*>::iterator iter = tasks.begin(); iter != tasks.end() && deadlocked == true; iter++) {
                if ((*iter)->status == "blocked") {
                    cout << "According to the spec task " << (*iter)->taskNum
                         << " is aborted now and its resources are available next cycle" << endl;
                    (*iter)->status = "aborted";
                    terminated++;
                    map<int, int> temp = (*iter)->ownResource;
                    for (map<int, int>::iterator iter2 = temp.begin(); iter2 != temp.end(); iter2++) {
                        resource[iter2->first] += iter2->second;
                    }
                    break;
                }
            }

            for (vector<task*>::iterator iter1 = blocked.begin(); iter1 != blocked.end() && deadlocked == true; ) {
                task::activity* curAct = (*iter1)->activities[(*iter1)->curIndex];
                if ((*iter1)->status == "aborted") {
                    iter1 = blocked.erase(iter1);
                } else {
                    if (resource[curAct->resourceType] >= curAct->numberReq) {
                        deadlocked = false;
                        break;
                    }
                    iter1++;
                }
            }
    }

}



void FIFO() {
    while (terminated != taskCount) {
        cout<<"During "<<cycle<<"-"<<cycle+1<<endl;
        checkBlockedTasks();
        map<int, int> released;
        deadlocked = true;
        for (int i = 0; i < tasks.size(); i++) {
            if (tasks[i]->status == "active") {
                task::activity* curAct = tasks[i]->activities[tasks[i]->curIndex];
                if (curAct->delay-- > 0) {
                    deadlocked = false;
                    cout<<" Task"<<i+1<<" computes: remaining delay time "<<curAct->delay<<endl;
                }else if (curAct->type == "initiate") {
                    deadlocked = false;
                    tasks[i]->curIndex++;
                    cout<<" Task"<<i+1<<" completes its initiate"<<endl;
                } else if (curAct->type == "request") {
                    if (resource[curAct->resourceType] >= curAct->numberReq) {
                        deadlocked = false;
                        tasks[i]->curIndex++;
                        tasks[i]->ownResource[curAct->resourceType] += curAct->numberReq;
                        resource[curAct->resourceType] -= curAct->numberReq;
                        cout<<" Task"<<tasks[i]->taskNum<<" completes its request"<<endl;
                    } else {
                        tasks[i]->status = "blocked";
                        blocked.push_back(tasks[i]);
                        cout<<" Task"<<tasks[i]->taskNum<<"'s request cannot be granted"<<endl;
                    }
                } else if (curAct->type == "release") {
                    deadlocked = false;
                    int resourceType = curAct->resourceType;
                    released[resourceType] += curAct->numberReq;
                    //resource[resourceType] += curAct->numberReq;
                    tasks[i]->ownResource[resourceType] -= curAct->numberReq;
                    tasks[i]->curIndex++;
                    cout<<" Task"<<i+1<<" release "<<curAct->numberReq<<" "<<"units of "<<resourceType<<" available at "<<cycle + 1<<endl;
                }
                task::activity* nextAct = tasks[i]->activities[tasks[i]->curIndex];
                if (nextAct->type == "terminate" && nextAct->delay == 0) {
                    tasks[i]->timeTaken = cycle + 1;
                    tasks[i]->status="terminated";
                    terminated++;
                    cout<<" Task"<<i+1<<" is finished at "<<cycle+1<<endl;
                }
            } else if (tasks[i]->status == "abort to active") {
                deadlocked = false;
                tasks[i]->status = "active";
            }
        }
        for (int i = 1; i <= resourceCount; i++) {
            resource[i] += released[i];
        }
        printResourceStatus();

        abortTask();
        //printTask();
        calWaitingTime();
        if(cycle++ > 20) break;
    }
}

int main(int argc, char* argv[]) {
    readTasks("example10.txt");
    FIFO();
    std::cout << "FIFO" << std::endl;
    printOutput();
    return 0;
}