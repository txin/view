/*
 * main program 
 * calibration, stereoView for depth, eyeTracking, change 3D view
 *
 */
#include <stdio.h>
#include <pthread.h>

#include "EyeTracking.h"
#include "Cube.h"
#include "Global.h"

#define THREAD_NUM 2

using namespace std;

// task type for different thread
enum Task {EYETRACKING, CUBE};

// thread id same to camera id
void *doTask(void *t) {

    long tid = (long)t;
    Task taskid = (Task)tid;

    switch (taskid) {

    case EYETRACKING: {
        EyeTracking eyeTracking;
        eyeTracking.run();
    }
        break;
    case CUBE: {
        Cube cube;
        cube.run();
    }
        break;
    default: {
        cerr << "Error: invalid task id" << endl;
    }
        break;
    }

    pthread_exit(NULL);
}

// 2 threads for eyeTracking and 3D view of a cube separately
int main() {

    pthread_t threads[THREAD_NUM];
    pthread_attr_t attr;
    void *status;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    // test with setting global's position
    Global global = Global::getInstance();
    global.setPosition(2, 2);

    // create threads
    int rc;
    for (int i = 0; i < THREAD_NUM; i++) {
        cout << "main(): creating thread, " << i << endl;
        long index = (long)i;
        rc = pthread_create(&threads[i], NULL, doTask, (void *)index);
    }
    
    pthread_attr_destroy(&attr);
    // join threads
    for (int i = 0; i < THREAD_NUM; i++) {
        rc = pthread_join(threads[i], &status);
        if (rc) {
            cout << "Error: unable to join, " << rc << endl;
        }
        cout << "Main: completed thread id: " << i;
        cout << " exiting with status: " << status << endl;
    }
    cout << "Main: program exiting." << endl;
    return 0;
}
