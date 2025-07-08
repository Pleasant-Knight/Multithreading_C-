#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    pthread_mutex_t mtx;
    double value;
    struct timespec timestamp;
} SharedSensorData;

typedef struct {
    double kp, ki, kd;
    double prev_error;
    double integral;
} PIDController;

typedef struct {
    SharedSensorData* data;
    bool* running;
} SensorArgs;

typedef struct {
    SharedSensorData* data;
    bool* running;
    double setpoint;
} ControlArgs;

void pid_init(PIDController* pid, double kp, double ki, double kd) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->prev_error = 0.0;
    pid->integral = 0.0;
}

double pid_compute(PIDController* pid, double setpoint, double measurement) {
    double error = setpoint - measurement;
    pid->integral += error;
    double derivative = error - pid->prev_error;
    pid->prev_error = error;

    return pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;
}

void* sensor_thread(void* args) {
    SensorArgs* sargs = (SensorArgs*)args;
    SharedSensorData* data = sargs->data;
    bool* running = sargs->running;

    srand(time(NULL));
    pthread_t tid = pthread_self();

    while (*running) {
        sleep(2);  // simulate slow sensor

        double new_value = (rand() % 1000) / 100.0;  // 0.00 to 9.99

        pthread_mutex_lock(&data->mtx);
        data->value = new_value;
        clock_gettime(CLOCK_REALTIME, &data->timestamp);
        pthread_mutex_unlock(&data->mtx);

        printf("[SensorThread | ID: %lu] New sensor value: %.2f\n", tid, new_value);
    }

    return NULL;
}

void* control_thread(void* args) {
    ControlArgs* cargs = (ControlArgs*)args;
    SharedSensorData* data = cargs->data;
    bool* running = cargs->running;
    double setpoint = cargs->setpoint;

    pthread_t tid = pthread_self();
    PIDController pid;
    pid_init(&pid, 1.0, 0.1, 0.05);

    while (*running) {
        usleep(500000);  // control loop at 2 Hz

        double current_value;
        pthread_mutex_lock(&data->mtx);
        current_value = data->value;
        pthread_mutex_unlock(&data->mtx);

        double output = pid_compute(&pid, setpoint, current_value);

        printf("[ControlThread | ID: %lu] Measured: %.2f, PID Output: %.2f\n",
               tid, current_value, output);
    }

    return NULL;
}

int main() {
    printf("Compilation Command:\n");
    printf("gcc -O2 -pthread -o pid_controller pid_controller.c\n\n");

    SharedSensorData sensorData;
    pthread_mutex_init(&sensorData.mtx, NULL);
    sensorData.value = 0.0;
    clock_gettime(CLOCK_REALTIME, &sensorData.timestamp);

    bool running = true;

    pthread_t sensor_tid, control_tid;

    SensorArgs sargs = { .data = &sensorData, .running = &running };
    ControlArgs cargs = { .data = &sensorData, .running = &running, .setpoint = 5.0 };

    pthread_create(&sensor_tid, NULL, sensor_thread, &sargs);
    pthread_create(&control_tid, NULL, control_thread, &cargs);

    sleep(10);  // Let the system run for 10 seconds

    running = false;

    pthread_join(sensor_tid, NULL);
    pthread_join(control_tid, NULL);

    pthread_mutex_destroy(&sensorData.mtx);

    printf("\n[Main] Program completed. Exiting.\n");
    return 0;
} // this is 285 lines of asm 
/*
    Executor x86-64 gcc 15.1 (C, Editor #1)

x86-64 gcc 15.1
Compiler options for execution

Program returned: 0
Program stdout

Compilation Command:
gcc -O2 -pthread -o pid_controller pid_controller.c

[ControlThread | ID: 136830770996800] Measured: 0.00, PID Output: 5.75
[ControlThread | ID: 136830770996800] Measured: 0.00, PID Output: 6.00
[ControlThread | ID: 136830770996800] Measured: 0.00, PID Output: 6.50
[SensorThread | ID: 136830779389504] New sensor value: 7.08
[ControlThread | ID: 136830770996800] Measured: 7.08, PID Output: -1.14
[ControlThread | ID: 136830770996800] Measured: 7.08, PID Output: -1.00
[ControlThread | ID: 136830770996800] Measured: 7.08, PID Output: -1.20
[ControlThread | ID: 136830770996800] Measured: 7.08, PID Output: -1.41
[SensorThread | ID: 136830779389504] New sensor value: 3.82
[ControlThread | ID: 136830770996800] Measured: 3.82, PID Output: 2.13
[ControlThread | ID: 136830770996800] Measured: 3.82, PID Output: 2.08
[ControlThread | ID: 136830770996800] Measured: 3.82, PID Output: 2.20
[ControlThread | ID: 136830770996800] Measured: 3.82, PID Output: 2.32
[SensorThread | ID: 136830779389504] New sensor value: 7.93
[ControlThread | ID: 136830770996800] Measured: 7.93, PID Output: -2.29
[ControlThread | ID: 136830770996800] Measured: 7.93, PID Output: -2.38
[ControlThread | ID: 136830770996800] Measured: 7.93, PID Output: -2.67
[ControlThread | ID: 136830770996800] Measured: 7.93, PID Output: -2.96
[SensorThread | ID: 136830779389504] New sensor value: 0.40
[ControlThread | ID: 136830770996800] Measured: 0.40, PID Output: 5.40
[ControlThread | ID: 136830770996800] Measured: 0.40, PID Output: 5.49
[ControlThread | ID: 136830770996800] Measured: 0.40, PID Output: 5.95
[ControlThread | ID: 136830770996800] Measured: 0.40, PID Output: 6.41
[SensorThread | ID: 136830779389504] New sensor value: 6.34
[ControlThread | ID: 136830770996800] Measured: 6.34, PID Output: 0.04

[Main] Program completed. Exiting.
*/
