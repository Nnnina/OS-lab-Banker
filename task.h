//
// Created by Nina Lei on 3/22/18.
//

#ifndef OSLAB3_TASK_H
#define OSLAB3_TASK_H

#endif //OSLAB3_TASK_H
#include <iostream>
#include <string.h>
#include <queue>

using namespace std;
class task {
public:
    struct activity {
        string type;
        int delay;
        int resourceType;
        int numberReq;
        activity(string type, int delay, int resourceType, int numberReq)
        :type(type), delay(delay), resourceType(resourceType), numberReq(numberReq) {}
    };
    int timeTaken = 0;
    int waitingTime = 0;
    string status = "active";
    int curIndex = 0;
    vector<activity*> activities;
    map<int,int> ownResource;
    int taskNum;
    task(int taskNum):taskNum(taskNum){};
};