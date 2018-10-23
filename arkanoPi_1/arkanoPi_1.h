#ifndef _ARKANOPI_H_
#define _ARKANOPI_H_

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <softPwm.h>


#include "kbhit.h" // para poder detectar teclas pulsadas sin bloqueo y leer las teclas pulsadas

#include "arkanoPiLib.h"

#include "tmr.h"
#include "fsm.h" // para poder crear y ejecutar la máquina de estados

#define CLK_MS 10 // PERIODO DE ACTUALIZACION DE LA MAQUINA ESTADOS
// FLAGS DEL SISTEMA
#define FLAG_TECLA 0X04
#define FLAG_PELOTA 0X10
#define FLAG_RAQUETA_DERECHA 0x01
#define FLAG_RAQUETA_IZQUIERDA 0X02
#define FLAG_FINAL_JUEGO 0X08
#define FLAG_VIDA 0X20


//Definimos los pines de las filas y las columnas
#define GPIO_COL_1 14
#define GPIO_COL_2 17
#define GPIO_COL_3 18
#define GPIO_COL_4 22

#define GPIO_ROW_1 0
#define GPIO_ROW_2 1
#define GPIO_ROW_3 2
#define GPIO_ROW_4 3
#define GPIO_ROW_5 4
#define GPIO_ROW_6 7
#define GPIO_ROW_7 23
 //Define los pines de los botones
#define GPIO_BOT_DCHA 5
#define GPIO_BOT_IZQ 6
//Define el pin del Buzzer
#define BUZZER 24
//Define los pines del display
#define DISPLAY_A 25 //1
#define DISPLAY_B 10 //2
#define DISPLAY_C 8 //4


//TODO:MIRAR LOS KEY A PONER
// A 'key' which we can lock and unlock - values are 0 through 3
//	This is interpreted internally as a pthread_mutex by wiringPi
//	which is hiding some of that to make life simple.
#define	FLAGS_KEY	1
#define	STD_IO_BUFFER_KEY	2

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

// Prototipos de funciones de entrada
int CompruebaTeclaPulsada (fsm_t* this);
int CompruebaTeclaPelota (fsm_t* this);
int CompruebaTeclaRaquetaIzquierda(fsm_t* this);
int CompruebaTeclaRaquetaDerecha (fsm_t* this);
int CompruebaFinalJuego (fsm_t* this);
int CompruebaTiempoVida(fsm_t* this);
//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
void InicializaJuego(fsm_t*fsm);
void FinalJuego(fsm_t*fsm);
void MovimientoPelota(fsm_t*fsm);
void MueveRaquetaDerecha(fsm_t*fsm);
void MueveRaquetaIzquierda(fsm_t*fsm);
void ReseteaJuego(fsm_t*fsm);
void RestaVidas(fsm_t*fsm);
void escribeVidas();


typedef enum {
	WAIT_START,
	WAIT_PUSH,
	WAIT_END} tipo_estados_juego;

typedef struct {
	tipo_arkanoPi arkanoPi;
	tipo_estados_juego estado;
	char teclaPulsada;
} tipo_juego;

//------------------------------------------------------
// FUNCIONES DE ACCION
//------------------------------------------------------



//------------------------------------------------------
// FUNCIONES DE INICIALIZACION
//------------------------------------------------------
int systemSetup (void);

//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------
PI_THREAD (thread_explora_teclado);

#endif /* ARKANOPI_H_ */
