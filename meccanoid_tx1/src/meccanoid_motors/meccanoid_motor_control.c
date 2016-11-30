/*
/ GPIO stuff adapted from Jetsonhacks.com
/ TODO: must be able to export GPIO to user space (need sudo, how to do in ROS?)
/ workaround: << echo "1" | sudo tee /sys/class/gpio/export >> prior to launching ros node?
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include "meccanoid_motors/meccanoid_motor_control.h"

/*
/ Creates a meccanoid motor controller struct
*/
struct MeccanoidMotorControl* createMeccanoidMotorControl (){
    struct MeccanoidMotorControl* controller = malloc(sizeof(struct MeccanoidMotorControl));
    if (controller){
        controller->active = -1;
        controller->numMotors = 2;
    }else{ 
        puts ("Not enough memory");
    }
    return controller;
}

/*
/ Destroys a meccanoid motor controller struct
*/ 
void destroyMeccanoidMotorControl(struct MeccanoidMotorControl* controller){
    free(controller);
}

/*
/ starts meccanoid motor control logic
*/
void activateMeccanoidMotorControl(struct MeccanoidMotorControl* controller){
    //TODO: set gpio pins in motor control mode
    //TODO: init control logic
    controller->active = 1;
}

/*
/ stops meccanoid motor control logic
*/
void deactivateMeccanoidMotorControl(struct MeccanoidMotorControl* controller){
    //TODO: set gpio pins in not motor control mode
    //TODO: deinit control logic
    controller->active = -1;
}

/*
/ << Can only be called as super user >>
/ Export the given gpio to userspace;
/ Return: Success = 0 ; otherwise open file error
*/
int gpioExport ( jetsonGPIO gpio ){
    int fileDescriptor, length;
    char commandBuffer[MAX_BUF];

    fileDescriptor = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
    if (fileDescriptor < 0) {
        char errorBuffer[128];
        snprintf(errorBuffer,sizeof(errorBuffer), "gpioExport unable to open gpio%d",gpio);
        perror(errorBuffer);
        return fileDescriptor;
    }

    length = snprintf(commandBuffer, sizeof(commandBuffer), "%d", gpio);
    if (write(fileDescriptor, commandBuffer, length) != length) {
        perror("gpioExport");
        return fileDescriptor;
    }
    close(fileDescriptor);

    return 0;
}

/*
/ << Can only be called as super user >>
/ Unexport the given gpio from userspace
/ Return: Success = 0 ; otherwise open file error
*/
int gpioUnexport ( jetsonGPIO gpio ){
    int fileDescriptor, length;
    char commandBuffer[MAX_BUF];

    fileDescriptor = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
    if (fileDescriptor < 0) {
        char errorBuffer[128];
        snprintf(errorBuffer,sizeof(errorBuffer), "gpioUnexport unable to open gpio%d",gpio);
        perror(errorBuffer);
        return fileDescriptor;
    }

    length = snprintf(commandBuffer, sizeof(commandBuffer), "%d", gpio);
    if (write(fileDescriptor, commandBuffer, length) != length) {
        perror("gpioUnexport");
        return fileDescriptor;
    }
    close(fileDescriptor);
    return 0;
}

/*
/ Set the direction of the GPIO pin 
/ Return: Success = 0 ; otherwise open file error
*/
int gpioSetDirection ( jetsonGPIO gpio, unsigned int out_flag ){
    int fileDescriptor;
    char commandBuffer[MAX_BUF];

    snprintf(commandBuffer, sizeof(commandBuffer), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

    fileDescriptor = open(commandBuffer, O_WRONLY);
    if (fileDescriptor < 0) {
        char errorBuffer[128];
        snprintf(errorBuffer,sizeof(errorBuffer), "gpioSetDirection unable to open gpio%d",gpio);
        perror(errorBuffer);
        return fileDescriptor;
    }

    if (out_flag) {
        if (write(fileDescriptor, "out", 4) != 4) {
            perror("gpioSetDirection") ;
            return fileDescriptor;
        }
    }
    else {
        if (write(fileDescriptor, "in", 3) != 3) {
            perror("gpioSetDirection") ;
            return fileDescriptor;
        }
    }
    close(fileDescriptor);
    return 0;
}

/*
/ Set the value of the GPIO pin to 1 or 0
/ Return: Success = 0 ; otherwise open file error
*/
int gpioSetValue ( jetsonGPIO gpio, unsigned int value ){
    int fileDescriptor;
    char commandBuffer[MAX_BUF];

    snprintf(commandBuffer, sizeof(commandBuffer), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fileDescriptor = open(commandBuffer, O_WRONLY);
    if (fileDescriptor < 0) {
        char errorBuffer[128] ;
        snprintf(errorBuffer,sizeof(errorBuffer), "gpioSetValue unable to open gpio%d",gpio);
        perror(errorBuffer);
        return fileDescriptor;
    }

    if (value) {
        if (write(fileDescriptor, "1", 2) != 2) {
            perror("gpioSetValue");
            return fileDescriptor;
        }
    }
    else {
        if (write(fileDescriptor, "0", 2) != 2) {
            perror("gpioSetValue");
            return fileDescriptor;
        }
    }
    close(fileDescriptor);
    return 0;
}

/*
/ Get the value of the requested GPIO pin ; value return is 0 or 1
/ Return: Success = 0 ; otherwise open file error
*/
int gpioGetValue ( jetsonGPIO gpio, unsigned int *value){
    int fileDescriptor;
    char commandBuffer[MAX_BUF];
    char ch;

    snprintf(commandBuffer, sizeof(commandBuffer), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fileDescriptor = open(commandBuffer, O_RDONLY);
    if (fileDescriptor < 0) {
        char errorBuffer[128] ;
        snprintf(errorBuffer,sizeof(errorBuffer), "gpioGetValue unable to open gpio%d",gpio);
        perror(errorBuffer);
        return fileDescriptor;
    }

    if (read(fileDescriptor, &ch, 1) != 1) {
        perror("gpioGetValue");
        return fileDescriptor;
     }

    if (ch != '0') {
        *value = 1;
    } else {
        *value = 0;
    }

    close(fileDescriptor);
    return 0;
}

/*
/ Set the edge of the GPIO pin
/ Valid edges: 'none' 'rising' 'falling' 'both'
/ Return: Success = 0 ; otherwise open file error
*/
int gpioSetEdge ( jetsonGPIO gpio, char *edge ){
    int fileDescriptor;
    char commandBuffer[MAX_BUF];

    snprintf(commandBuffer, sizeof(commandBuffer), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

    fileDescriptor = open(commandBuffer, O_WRONLY);
    if (fileDescriptor < 0) {
        char errorBuffer[128];
        snprintf(errorBuffer,sizeof(errorBuffer), "gpioSetEdge unable to open gpio%d",gpio);
        perror(errorBuffer);
        return fileDescriptor;
    }

    if (write(fileDescriptor, edge, strlen(edge) + 1) != ((int)(strlen(edge) + 1))) {
        perror("gpioSetEdge");
        return fileDescriptor;
    }
    close(fileDescriptor);
    return 0;
}

/*
/ Open the given pin for reading
/ Returns the file descriptor of the named pin
*/
int gpioOpen( jetsonGPIO gpio ){
    int fileDescriptor;
    char commandBuffer[MAX_BUF];

    snprintf(commandBuffer, sizeof(commandBuffer), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fileDescriptor = open(commandBuffer, O_RDONLY | O_NONBLOCK );
    if (fileDescriptor < 0) {
        char errorBuffer[128];
        snprintf(errorBuffer,sizeof(errorBuffer), "gpioOpen unable to open gpio%d",gpio);
        perror(errorBuffer);
    }
    return fileDescriptor;
}


/*
/ Close the given file descriptor 
*/
int gpioClose ( int fileDescriptor )
{
    return close(fileDescriptor);
}

/*
/ Set the active_low attribute of the GPIO pin to 1 or 0
/ Return: Success = 0 ; otherwise open file error
*/
int gpioActiveLow ( jetsonGPIO gpio, unsigned int value ){
    int fileDescriptor;
    char commandBuffer[MAX_BUF];

    snprintf(commandBuffer, sizeof(commandBuffer), SYSFS_GPIO_DIR "/gpio%d/active_low", gpio);

    fileDescriptor = open(commandBuffer, O_WRONLY);
    if (fileDescriptor < 0) {
        char errorBuffer[128];
        snprintf(errorBuffer,sizeof(errorBuffer), "gpioActiveLow unable to open gpio%d",gpio);
        perror(errorBuffer);
        return fileDescriptor;
    }

    if (value) {
        if (write(fileDescriptor, "1", 2) != 2) {
            perror("gpioActiveLow");
            return fileDescriptor;
        }
    }
    else {
        if (write(fileDescriptor, "0", 2) != 2) {
            perror("gpioActiveLow");
            return fileDescriptor;
        }
    }
    close(fileDescriptor);
    return 0;
}