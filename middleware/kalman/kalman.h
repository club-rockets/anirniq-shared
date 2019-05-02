#ifndef KALMAN_H_
#define KALMAN_H_

#include "stm32f4xx_hal.h"
#include "main.h"
#include "arm_matrix/arm_matrix.h"

/******************************************************************************/
/*                                Define                                      */
/******************************************************************************/
#define ALTITUDE_SIGMA 		  6	 // ecart type de la mesure d'altitude barometrique
#define ACCELERATION_SIGMA 	0  // ecart type de la mesure d'acceleration

#define BAROMETER_H_GAIN		  1	//1 ou 0, maps le capteur dans le filtre
#define ACCELEROMETER_H_GAIN	0 //1 ou 0, maps le capteur dans le filtre

#define KALMAN_TIME_STEP		0.05        // pas de calcul en seconde
#define CONVERGENCE_TIMEOUT		100        // nombre d'essai pour trouver les gains de kalman optimals
#define CONVERGENCE_PRECISION	0.000001  //critere de convergence

#define VAR_MODEL_S 25  // variance de l'altitude dans le modele
#define VAR_MODEL_V 5 // variance de la vitesse dans le modele
#define VAR_MODEL_A 1 // variance de l'acceleration dans le modele

/******************************************************************************/
/*                              Type  Prototype                               */
/******************************************************************************/
typedef struct {
  float raw_acceleration;
  float raw_altitude;

  float altitude;
  float velocity;
  float acceleration;

  uint32_t initialized;
} kalman_t;

/******************************************************************************/
/*                             Global variable                                */
/******************************************************************************/

/******************************************************************************/
/*                             Function prototype                             */
/******************************************************************************/
void kalman_init(kalman_t *kalman);
void kalman_update(kalman_t *kalman, float altitude, float acceleration, float dt);

#endif //KALMAN_H_
