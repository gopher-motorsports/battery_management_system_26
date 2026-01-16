#ifndef INC_PID_H_
#define INC_PID_H_

typedef struct {
    // PID constants
    float kp;
    float kiPos;
    float kiNeg;
    float kd;

    // Anti-windup constant
    float kaw;

    // Output clamp
    float outputMin;
    float outputMax;

    // Integrater clamp
    float integratorMin;
    float integratorMax;

    // Slew limits
    float slewUp;
    float slewDown;

    // Update period
    float dt;

    // State
    float setPoint;
    float integratorState;

    float previousInput;
    float previousOutput;
}PID_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

float clampOutput(PID_S* pid, float val);

float slewLimitOutput(PID_S* pid, float val);

void updateI(PID_S* pid, float input);

void updateAntiWindup(PID_S* pid, float commandedOutput);

float pidStep(PID_S* pid, float input, float ff);

#endif /* INC_PID_H_ */