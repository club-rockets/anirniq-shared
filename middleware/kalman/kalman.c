#include "kalman.h"

//maps X (state variable) to Z (sensor data)
static float H[3] = { BAROMETER_H_GAIN, 0, 0 };
static const arm_matrix_instance_f32 H_f32 = { .numCols = 3, .numRows = 1, .pData = H };

static float Ht[3] = { BAROMETER_H_GAIN, 0, 0 };
static const arm_matrix_instance_f32 H_t_f32 = { .numCols = 1, .numRows = 3, .pData = Ht };

//measurement noise covariance
static float R[1] = { ALTITUDE_SIGMA * ALTITUDE_SIGMA };
static const arm_matrix_instance_f32 R_f32 = { .numCols = 1, .numRows = 1, .pData = R };

//process noise covariance matrix
static float Q[9] = { VAR_MODEL_S, 0, 0, 0, VAR_MODEL_V, 0, 0, 0, VAR_MODEL_A };
static const arm_matrix_instance_f32 Q_f32 = { .numCols = 3, .numRows = 3, .pData = Q };

//maps previous state to next state (throughout motion equation)
static float A[9] = { 1, KALMAN_TIME_STEP, KALMAN_TIME_STEP * KALMAN_TIME_STEP * 0.5, 0, 1, KALMAN_TIME_STEP, 0, 0, 1 };
static arm_matrix_instance_f32 A_f32 = { .numCols = 3, .numRows = 3, .pData = A };

static float At[9] = { 0, 0, 0, KALMAN_TIME_STEP, 1, 0, KALMAN_TIME_STEP * KALMAN_TIME_STEP * 0.5, KALMAN_TIME_STEP, 1 };
static arm_matrix_instance_f32 A_t_f32 = { .numCols = 3, .numRows = 3, .pData = At };

//error covariance  matrix, first guess is identity
static float P0[9] = { VAR_MODEL_S, 0, 0, 0, VAR_MODEL_V, 0, 0, 0, VAR_MODEL_A };
static arm_matrix_instance_f32 P0_f32 = { .numCols = 3, .numRows = 3, .pData = P0 };

static float P1[9] = { VAR_MODEL_S, 0, 0, 0, VAR_MODEL_V, 0, 0, 0, VAR_MODEL_A };
static arm_matrix_instance_f32 P1_f32 = { .numCols = 3, .numRows = 3, .pData = P1 };

static float P_past[9] = { VAR_MODEL_S, 0, 0, 0, VAR_MODEL_V, 0, 0, 0, VAR_MODEL_A };
static arm_matrix_instance_f32 P_past_f32 = { .numCols = 3, .numRows = 3, .pData = P_past };

static float K[3] = { 0.72, 0.39, 0.71 };
static arm_matrix_instance_f32 K_f32 = { .numCols = 1, .numRows = 3, .pData = K };

static float K_last[3] = { 0.5, 0.5, 0.5 };
static arm_matrix_instance_f32 K_last_f32 = { .numCols = 1, .numRows = 3, .pData = K_last };

//state matrix
static float X0[3] = { 0.1 };
static arm_matrix_instance_f32 X0_f32 = { .numCols = 1, .numRows = 3, .pData = X0 };

static float X1[3] = { 0.1 };
arm_matrix_instance_f32 X1_f32 = { .numCols = 1, .numRows = 3, .pData = X1 };

static float X_past[3] = { 0.1 };
static arm_matrix_instance_f32 X_past_f32 = { .numCols = 1, .numRows = 3, .pData = X_past };

static float Z[1] = { 0 };
static arm_matrix_instance_f32 Z_f32 = { .numCols = 1, .numRows = 1, .pData = Z };

static float Y[1] = { 0 };
static arm_matrix_instance_f32 Y_f32 = { .numCols = 1, .numRows = 1, .pData = Y };

static float I_3x3[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
static arm_matrix_instance_f32 I_3x3_f32 = { .numCols = 3, .numRows = 3, .pData = I_3x3 };

/** Initialize Kalman structure 
 * 
 * @param  kalmann  pointer to allocated kalman struct
 */
void kalman_init(kalman_t *kalman) {
    kalman->acceleration = 0;
    kalman->altitude = 0;
    kalman->velocity = 0;
    kalman->raw_acceleration = 0;
    kalman->raw_altitude = 0;

    kalman->initialized = 1;
}

/**
 */
void kalman_update(kalman_t *kalman, float altitude, float acceleration)
{
    uint32_t loop_counter = 0;
    uint32_t convergence = 1;

    float temp1_3x1[2] = { 0 };
    arm_matrix_instance_f32 temp1_3x1_f32 = { .numCols = 1, .numRows = 3, .pData = temp1_3x1 };

    float temp1_3x3[9] = { 0 };
    arm_matrix_instance_f32 temp1_3x3_f32 = { .numCols = 3, .numRows = 3, .pData = temp1_3x3 };

    float temp2_3x3[9] = { 0 };
    arm_matrix_instance_f32 temp2_3x3_f32 = { .numCols = 3, .numRows = 3, .pData = temp2_3x3 };

    float temp1_1x3[3] = { 0 };
    arm_matrix_instance_f32 temp1_1x3_f32 = { .numCols = 3, .numRows = 1, .pData = temp1_1x3 };

    float temp1_1x1[1] = { 0 };
    arm_matrix_instance_f32 temp1_1x1_f32 = { .numCols = 1, .numRows = 1, .pData = temp1_1x1 };

    float temp2_1x1[1] = { 0 };
    arm_matrix_instance_f32 temp2_1x1_f32 = { .numCols = 1, .numRows = 1, .pData = temp2_1x1 };

    for (int i = X1_f32.numRows * X1_f32.numCols - 1; i >= 0; i--) {
        X_past[i] = X1[i];
    }

    for (int i = P1_f32.numRows * P1_f32.numCols - 1; i >= 0; i--) {
        P_past[i] = P1[i];
    }

    Z[0] = altitude;

    do {
        // Predicted state, a priori estimate
        // x(k|k-1) = A * x(k-1|k-1)
        arm_mat_mult_f32(&A_f32, &X_past_f32, &X0_f32);

        // Predicted covariance, a priori estimate
        // P(k|k-1) = A * P(k-1|k-1) * A' + Q
        arm_mat_mult_f32(&A_f32, &P_past_f32, &temp1_3x3_f32);
        arm_mat_mult_f32(&temp1_3x3_f32, &A_t_f32, &temp2_3x3_f32);
        arm_mat_add_f32(&temp2_3x3_f32, &Q_f32, &P0_f32);

        // Innovation (or measurement residual)
        // y = Z - H * x(k|k-1)
        arm_mat_mult_f32(&H_f32, &X0_f32, &temp1_1x1_f32);
        arm_mat_sub_f32(&Z_f32, &temp1_1x1_f32, &Y_f32);

        // Innovation (or residual) covariance
        // S = H * P(k|k-1) * H' + R
        arm_mat_mult_f32(&H_f32, &P0_f32, &temp1_1x3_f32);
        arm_mat_mult_f32(&temp1_1x3_f32, &H_t_f32, &temp1_1x1_f32);
        arm_mat_add_f32(&temp1_1x1_f32, &R_f32, &temp2_1x1_f32);

        // Optimal Kalman gain
        // K = P(k|k-1) * H' * S^-1
        arm_mat_inverse_f32(&temp2_1x1_f32, &temp1_1x1_f32);
        arm_mat_mult_f32(&P0_f32, &H_t_f32, &temp1_3x1_f32);
        arm_mat_mult_f32(&temp1_3x1_f32, &temp1_1x1_f32, &K_f32);

        // Updated (a posteriori) state estimate
        // X(k|k) = x(k|k-1) + K * y
        arm_mat_mult_f32(&K_f32, &Y_f32, &temp1_3x1_f32);
        arm_mat_add_f32(&X0_f32, &temp1_3x1_f32, &X1_f32);

        // Updated (a posteriori) estimate covariance
        // P(k|k) = (I - K * H) * P(k|k-1)
        arm_mat_mult_f32(&K_f32, &H_f32, &temp1_3x3_f32);
        arm_mat_sub_f32(&I_3x3_f32, &temp1_3x3_f32, &temp2_3x3_f32);
        arm_mat_mult_f32(&temp2_3x3_f32, &P0_f32, &P1_f32);

        for (int i = K_last_f32.numRows * K_last_f32.numCols - 1; i >= 0; i--) {
            if ((K[i] - K_last[i]) / K_last[i] < CONVERGENCE_PRECISION) {
                convergence &= 1;
            } else {
                convergence = 0;
            }
            K_last[i] = K[i];
        }

        loop_counter++;

    } while (convergence == 0 && loop_counter < CONVERGENCE_TIMEOUT);

    //update handle values
    kalman->altitude = X1[0];
    kalman->velocity = X1[1];
    kalman->acceleration = X1[2];
}

