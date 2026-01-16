/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "pid.h"
#include <stdbool.h>

static float clamp(float val, float min, float max)
{
    if(val < min)
    {
        val = min;
    }
    else if(val > max)
    {
        val = max;
    }

    return val;    
}

float clampOutput(PID_S* pid, float val)
{
    return clamp(val, pid->outputMin, pid->outputMax);
}

float slewLimitOutput(PID_S* pid, float val)
{
    float delta = val - pid->previousOutput;

    float deltaMaxUp = pid->slewUp * pid->dt;
    float deltaMaxDown = pid->slewDown * pid->dt;

    return (clamp(delta, deltaMaxUp, -deltaMaxDown) + pid->previousOutput);
}

void updateI(PID_S* pid, float input)
{
    float error = pid->setPoint - input;

    if(error >= 0)
    {
        pid->integratorState += (pid->kiPos * error * pid->dt); 
    }
    else
    {
        pid->integratorState += (pid->kiNeg * error * pid->dt);
    }

    clamp(pid->integratorState, pid->integratorMin, pid->integratorMax);    
}

void updateAntiWindup(PID_S* pid, float commandedOutput)
{
    pid->integratorState += (pid->kaw * (commandedOutput - pid->previousOutput) * pid->dt);

    clamp(pid->integratorState, pid->integratorMin, pid->integratorMax);
}

float pidStep(PID_S* pid, float input, float ff)
{
    // Calculate error
    float error = pid->setPoint - input;

    // Update P
    float p = pid->kp * error;

    // Get I
    float i = pid->integratorState;

    // Update D
    float d = pid->kd * (pid->previousInput - input) / pid->dt;

    float output = ff + p + i + d;

    pid->previousInput = input;
    pid->previousOutput = output;

    return output;
}
