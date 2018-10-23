#include "arkanoPi_1.h"
#include "tmr.h"
#include <wiringPi/softTone.h>
#include <softPwm.h>
#include <wiringPi.h>


enum fsm_state { //Estados del juego
  wait_start,
  wait_push,
  wait_end,
  };
int velocidad =1200; //Velocidad de la bola y de las vidas (MEJORA)
int tiempoInicio; //Variable que controla la generaci髇 de vidas (MEJORA)
int tiempoVidas= 25000; //Tiempo de generacion de una vida
// Array que contiene las filas
int GPIO_ROW[7]={GPIO_ROW_1, GPIO_ROW_2, GPIO_ROW_3, GPIO_ROW_4, GPIO_ROW_5, GPIO_ROW_6, GPIO_ROW_7};

int flags = 0;	//Variable flags
int c=0;		//Variable de control de las columnas. La usa el isr

//Variables AntiRebotes
int DEBOUNCE_TIME = 50;
int debounceTime = 0;
//Temporizadores
static tmr_t* tmr; 		  	//Control refresco Leds
static tmr_t* tmr_pelota; 	//Movimiento autonomo pelota
static tmr_t* tmr_vida; 	//Movimiento autonomo de la vida



static volatile tipo_juego juego;

// Espera hasta la proxima activacion del reloj
//------------------------------------------------------
// FUNCIONES DE ACCION
//------------------------------------------------------


// void Pitido (void): funcion encargada de hacer sonar un pitido en le BUZZER
void Pitido(){
	softPwmWrite(BUZZER, 80);
	delay(200);
	softPwmWrite(BUZZER, 0);
}
// void PitidoInicial (void): funcion encargada de hacer sonar
//un pitido en el BUZZER que indica el principio del juego
void pitidoInicial(){
	softPwmWrite (BUZZER, 10);
	delay(100);
	softPwmWrite (BUZZER,100);
	delay(100);
		softPwmWrite (BUZZER,10);
		delay(100);
			softPwmWrite (BUZZER,100);
			delay(100);
				softPwmWrite (BUZZER,10);
				delay(100);
					softPwmWrite (BUZZER,100);
					delay(100);
					softPwmWrite (BUZZER,0);

}
//void display: Funci髇 para iluminar los LEDs del display de 7 segmentos (Mejora)
void display (int a, int b, int c){
	digitalWrite(DISPLAY_A, a);
  	digitalWrite(DISPLAY_B, b);
  	digitalWrite(DISPLAY_C, c);
  	}
//void delay_until (unsigned int next): Espera a la proxima activacion de reloj
void delay_until (unsigned int next) {
	unsigned int now = millis();

	if (next > now) {
		delay (next - now);
    }
}
// void InicializaJuego (void): funcion encargada de llevar a cabo
// la oportuna inicializaci贸n de toda variable o estructura de datos
// que resulte necesaria para el desarrollo del juego.
void InicializaJuego (fsm_t*fsm) {
	//Si hubiera un error poner todos los flags a 0 CON flags=0;
	piLock (FLAGS_KEY);
	flags = 0;
	piUnlock (FLAGS_KEY);
	InicializaArkanoPi((tipo_arkanoPi *)&(juego.arkanoPi));
	juego.arkanoPi.vidas.numeroVidas= 3;
	escribeVidas();
	tiempoInicio= millis();
	pitidoInicial();


}


// void MueveRaquetaIzquierda (void): funcion encargada de ejecutar
// el movimiento hacia la izquierda contemplado para la raqueta.
// Debe garantizar la viabilidad del mismo mediante la comprobacion
// de que la nueva posicion correspondiente a la raqueta no suponga
// que esta rebase o exceda los limites definidos para el Area de juego
// (i.e. al menos uno de los leds que componen la raqueta debe permanecer
// visible durante _todo el transcurso de la partida).
void MueveRaquetaIzquierda (fsm_t*fsm) {
	// A completar por el alumno...
	piLock (FLAGS_KEY);
	flags &= ~FLAG_RAQUETA_IZQUIERDA;
	flags &= ~FLAG_TECLA;
	piUnlock (FLAGS_KEY);

	if(juego.arkanoPi.raqueta.x > -2){
		(juego.arkanoPi.raqueta.x) -=1;

	}
	printf("\n Se ha movido la raqueta \n");
	ActualizaPantalla((tipo_arkanoPi*)&(juego.arkanoPi));
}

// void MueveRaquetaDerecha (void): funci贸n similar a la anterior
// encargada del movimiento hacia la derecha.
void MueveRaquetaDerecha (fsm_t*fsm) {
	piLock (FLAGS_KEY);
	flags &= ~FLAG_RAQUETA_DERECHA;
	flags &= ~FLAG_TECLA;
	piUnlock (FLAGS_KEY);
	if(juego.arkanoPi.raqueta.x < 9){
			(juego.arkanoPi.raqueta.x) +=1;
		}
	printf("\n Se ha movido la raqueta \n");
	ActualizaPantalla((tipo_arkanoPi*)&(juego.arkanoPi));
	// A completar por el alumno...
}

// void MovimientoPelota (void): funcion encargada de actualizar la
// posicion de la pelota conforme a la trayectoria definida para esta.
// Para ello debera identificar los posibles rebotes de la pelota para,
// en ese caso, modificar su correspondiente trayectoria (los rebotes
// detectados contra alguno de los ladrillos implicaran adicionalmente
// la eliminacion del ladrillo). Del mismo modo, debera tambien
// identificar las situaciones en las que se da por finalizada la partida:
// bien porque el jugador no consiga devolver la pelota, y por tanto 茅sta
// rebase el limite inferior del area de juego, bien porque se agoten
// los ladrillos visibles en el 谩rea de juego.
void MovimientoPelota (fsm_t*fsm) {

	piLock (FLAGS_KEY);
	flags &= ~FLAG_PELOTA;
	piUnlock (FLAGS_KEY);

	int xs= juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv;
	int ys= juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv;

	//CHOQUE CON PAREDES
	//Pared izquierda
	if (xs <= -1){
		juego.arkanoPi.pelota.xv = 1;
	}//Pared derecha
	else if (xs>= 10){
		juego.arkanoPi.pelota.xv = -1;
	}
	//Pared superior
	else if (ys <= -1){
		juego.arkanoPi.pelota.yv = 1;
	}
	//CHOQUE CON RAQUETA
	//Extremo izquierdo
	if ( (xs == juego.arkanoPi.raqueta.x) && (ys == juego.arkanoPi.raqueta.y) ){
		juego.arkanoPi.pelota.yv = -1;
		if(juego.arkanoPi.pelota.xv == -1){
			juego.arkanoPi.pelota.xv = -1;
			juego.arkanoPi.pelota.x -= 1;
		}
		else if (juego.arkanoPi.pelota.xv == 0 || juego.arkanoPi.pelota.xv == 1){
			juego.arkanoPi.pelota.xv = -1;
		}
	}
	//Centro
	else if ((xs == juego.arkanoPi.raqueta.x +1) && (ys == juego.arkanoPi.raqueta.y)){
		juego.arkanoPi.pelota.yv = -1;
		juego.arkanoPi.pelota.xv = 0;
	}
	//Extremo derecho
	else if ( (xs == juego.arkanoPi.raqueta.x+2) && (ys == juego.arkanoPi.raqueta.y) ){
		juego.arkanoPi.pelota.yv = -1;
		if(juego.arkanoPi.pelota.xv == 1){
			juego.arkanoPi.pelota.xv = 1;
			juego.arkanoPi.pelota.x += 1;

		}
		else if (juego.arkanoPi.pelota.xv == 0 || juego.arkanoPi.pelota.xv == -1){
			juego.arkanoPi.pelota.xv = 1;
		}
	}
	//Choque con ladrillos

	if (juego.arkanoPi.ladrillos.matriz[xs][ys]==1){
		juego.arkanoPi.pelota.yv = 1;
		juego.arkanoPi.ladrillos.matriz[xs][ys]=0;
		Pitido();

	}
	//Movimiento
	juego.arkanoPi.pelota.x += juego.arkanoPi.pelota.xv;
	juego.arkanoPi.pelota.y += juego.arkanoPi.pelota.yv;
	printf("\n Se ha movido la pelota \n");

	ActualizaPantalla((tipo_arkanoPi*)&(juego.arkanoPi));
}



//void MovimientoVida(fsm_t*fsm): Metodo que se encarga del movimieto de la vida
//cuando la vida se sale del tablero la aparta y cambia su existecia a 0
void MovimientoVida(fsm_t*fsm) {
	piLock (FLAGS_KEY);
	flags &= ~FLAG_VIDA;
	piUnlock (FLAGS_KEY);
	juego.arkanoPi.vidas.x += juego.arkanoPi.vidas.xv;
	juego.arkanoPi.vidas.y += juego.arkanoPi.vidas.yv;
	if (juego.arkanoPi.vidas.y == 6){
		if((juego.arkanoPi.raqueta.x == juego.arkanoPi.vidas.x) || (juego.arkanoPi.raqueta.x +1==juego.arkanoPi.vidas.x) || (juego.arkanoPi.raqueta.x +2 == juego.arkanoPi.vidas.x)){
			juego.arkanoPi.vidas.numeroVidas += 1;
			escribeVidas();
		}
		juego.arkanoPi.vidas.yv =0;
		juego.arkanoPi.vidas.y =2;
		juego.arkanoPi.vidas.existe=0;
	}
}

//void AumentaVelocidad():Metodo que aumenta la velocidad de la pelota
//y lo muestra por el led
void AumentaVelocidad(){
	piLock (FLAGS_KEY);
	flags &= ~FLAG_RAQUETA_DERECHA;
	piUnlock (FLAGS_KEY);
	if (velocidad > 400){
		velocidad = velocidad -200;
	}else{
		velocidad = 1200;
	}
	switch(velocidad) {
    case 1200:
		display(0,0,0);
		break;

	case 1000:
		display(0,0,1);
		break;

	case 800:
		display(0,1,0);
		break;

	case 600:
		display(0,1,1);
		break;

	case 400:
		display(1,0,0);
		break;
	}
}

//void NuevaVida(fsm_t*fsm):Metodo que genera una vida cada tiempoVida
//Tambien gestiona su movimiento
void NuevaVida(fsm_t*fsm){
	if(juego.arkanoPi.vidas.numeroVidas <7){
		ReseteaVida((tipo_vida*)(&(juego.arkanoPi.vidas)));
		piLock (FLAGS_KEY);
			flags |= FLAG_VIDA;
			piUnlock (FLAGS_KEY);
		tmr_startms(tmr_vida,velocidad);

	}


}

// void FinalJuego (void): funci贸n encargada de mostrar en la ventana de
// terminal los mensajes necesarios para informar acerca del resultado del juego.
void FinalJuego (fsm_t*fsm) {
	piLock (FLAGS_KEY);
	flags = 0;

	piUnlock (FLAGS_KEY);
	int pantalla_final[MATRIZ_ANCHO][MATRIZ_ALTO] = {
			{0,1,1,1,1,1,0},
			{0,1,0,1,0,0,0},
			{0,1,0,1,0,0,0},
			{0,0,0,0,0,0,0},
			{0,1,1,1,1,1,0},
			{0,0,0,0,0,0,0},
			{0,1,1,1,1,1,0},
			{0,0,1,0,0,0,0},
			{0,0,0,0,1,0,0},
			{0,1,1,1,1,1,0},
	};
	int i, j = 0;

	for(i=0;i<MATRIZ_ANCHO;i++) {
		for(j=0;j<MATRIZ_ALTO;j++) {
			juego.arkanoPi.pantalla.matriz[i][j] = pantalla_final[i][j];
		}
	}
	printf("\n Final del Juego \n");
	PintaPantallaPorTerminal((tipo_pantalla *)&(juego.arkanoPi.pantalla));
}

//void ReseteaJuego (void): funci贸n encargada de llevar a cabo la
// reinicializaci贸n de cuantas variables o estructuras resulten
// necesarias para dar comienzo a una nueva partida.
void ReseteaJuego (fsm_t*fsm) {
	ReseteaMatriz((tipo_pantalla*)(&(juego.arkanoPi.pantalla)));
	ReseteaLadrillos((tipo_pantalla*)(&(juego.arkanoPi.ladrillos)));
	ReseteaPelota((tipo_pelota*)(&(juego.arkanoPi.pelota)));
	ReseteaVida((tipo_vida*)(&(juego.arkanoPi.vidas)));
	ReseteaRaqueta((tipo_raqueta*)(&(juego.arkanoPi.raqueta)));
}

//------------------------------------------------------
// FUNCIONES DE INICIALIZACION
//------------------------------------------------------


//void activaFlagDcha (void): funcion encargada de poner a 1
// el flag correspondiente a tecla pulsada y raqueta derecha
// Gestiona mediante las variables debounceTime los rebotes.
void activaFlagDcha(void){
	if (millis () < debounceTime)
	{
	debounceTime = millis () + DEBOUNCE_TIME ;
	return;
	}
	piLock (FLAGS_KEY);
	flags |= FLAG_RAQUETA_DERECHA;
	flags |= FLAG_TECLA;
	piUnlock (FLAGS_KEY);
	debounceTime = millis () + DEBOUNCE_TIME ;

	while (digitalRead(GPIO_BOT_DCHA)==HIGH){
		delay(1);
	}
}
//void activaFlagIzq (void): funcion encargada de poner a 1
// el flag correspondiente a tecla pulsada y raqueta izquierda
// Gestiona mediante las variables debounceTime los rebotes.
void activaFlagIzq(void){
	if (millis () < debounceTime)
	{
	debounceTime = millis () + DEBOUNCE_TIME ;
	return;
	}
	piLock (FLAGS_KEY);
	flags |= FLAG_RAQUETA_IZQUIERDA;
	flags |= FLAG_TECLA;
	piUnlock (FLAGS_KEY);
	debounceTime = millis () + DEBOUNCE_TIME ;

	while (digitalRead(GPIO_BOT_IZQ)==HIGH){
		delay(1);
	}
}
// int systemSetup (void): procedimiento de configuracion del sistema.
// Realizara, entra otras, todas las operaciones necesarias para:
// configurar el uso de posibles librerias (e.g. Wiring Pi),
// configurar las interrupciones externas asociadas a los pines GPIO,
// configurar las interrupciones periodicas y sus correspondientes temporizadores,
// crear, si fuese necesario, los threads adicionales que pueda requerir el sistema
int systemSetup (void) {
//	int x = 0;
	piLock (STD_IO_BUFFER_KEY);


	// sets up the wiringPi library
		if (wiringPiSetupGpio () < 0) {
			printf ("Unable to setup wiringPi\n");
			piUnlock (STD_IO_BUFFER_KEY);
			return -1;
		}
		//Columnas
		pinMode(14,OUTPUT);
		pinMode(17,OUTPUT);
		pinMode(18,OUTPUT);
		pinMode(22,OUTPUT);
		//Filas
		pinMode(0,OUTPUT);
		pinMode(1,OUTPUT);
		pinMode(2,OUTPUT);
		pinMode(3,OUTPUT);
		pinMode(4,OUTPUT);
		pinMode(7,OUTPUT);
		pinMode(23,OUTPUT);
		//Pulsadores
		pinMode(5, INPUT);
		pinMode(6, INPUT);
		//BUZZER
		pinMode(24, OUTPUT);
		//Display
		pinMode(25, OUTPUT);
		pinMode(10, OUTPUT);
		pinMode(8, OUTPUT);

		softPwmCreate(BUZZER, 0, 100);
		softPwmWrite(BUZZER, 0);

//Detecta una caida en los correspondientes pines y ejecuta la funcion del tercer parametro
	wiringPiISR(GPIO_BOT_DCHA,INT_EDGE_FALLING, activaFlagDcha);
	wiringPiISR(GPIO_BOT_IZQ, INT_EDGE_FALLING, activaFlagIzq);
// Lanzamos thread para exploracion del teclado convencional del PC
//	x = piThreadCreate (thread_explora_teclado);
//	if (x != 0) {
//		printf ("it didn't start!!!\n");
//		piUnlock (STD_IO_BUFFER_KEY);
//		return -1;
//	}
	PintaMensajeInicialPantalla((tipo_pantalla*)(&(juego.arkanoPi.pantalla)),(tipo_pantalla *)&(juego.arkanoPi.pantalla) );
	display(0,0,0);
	piUnlock (STD_IO_BUFFER_KEY);
	return 1;
}

//------------------------------------------------------
// FUNCIONES DE COMPROBACION
//------------------------------------------------------
// int CompruebaTiempo (fsm_t* this):procedimiento de comprobacion del tiempo.
// Devuelve un 1 si se tiene que generar una Vida
int CompruebaTiempoVida(fsm_t* this){
	int tiempoActual= millis();
	if ((tiempoActual - tiempoInicio) > tiempoVidas){
		tiempoInicio =millis();
		return 1;
	}
	return 0;
}

// int CompruebaTeclaPulsada (fsm_t* this):procedimiento de comprobacion de la variable flags.
// Si el flag correspondiente a tecla pulsada esta activo devuelve un uno, en caso contrario 0.
int CompruebaTeclaPulsada (fsm_t* this) {
	int result;
	piLock (FLAGS_KEY);
	result = (flags & FLAG_TECLA);
	piUnlock (FLAGS_KEY);
	return result;
}
// int CompruebaTeclaPelota (fsm_t* this):procedimiento de comprobacion de la variable flags.
// Si el flag correspondiente a pelota esta activo devuelve un uno, en caso contrario 0;
int CompruebaTeclaPelota(fsm_t* this) {
	int result;
	piLock (FLAGS_KEY);
	result = (flags & FLAG_PELOTA);
	piUnlock (FLAGS_KEY);

	return result;
}
// int CompruebaTeclaRaquetaDerecha (fsm_t* this):procedimiento de comprobacion de la variable flags.
// Si el flag correspondiente a raqueta derecha esta activo devuelve un uno, en caso contrario 0;
int CompruebaTeclaRaquetaDerecha (fsm_t* this) {
	int result;
	piLock (FLAGS_KEY);
	result = (flags & FLAG_RAQUETA_DERECHA);
	piUnlock (FLAGS_KEY);
	return result;
}

// int CompruebaFlagVida (fsm_t* this):procedimiento de comprobacion de la variable flags.
// Si el flag correspondiente a vida esta activo devuelve un uno, en caso contrario 0;
int CompruebaFlagVida (fsm_t* this) {
	int result;
	piLock (FLAGS_KEY);
	result = (flags & FLAG_VIDA);
	piUnlock (FLAGS_KEY);
	return result;
}
// int CompruebaTeclaRaquetaIzquierda (fsm_t* this):procedimiento de comprobacion de la variable flags.
// Si el flag correspondiente a raqueta izquierda esta activo devuelve un uno, en caso contrario 0;
int CompruebaTeclaRaquetaIzquierda (fsm_t* this) {
	int result;
	piLock (FLAGS_KEY);
	result = (flags & FLAG_RAQUETA_IZQUIERDA);
	piUnlock (FLAGS_KEY);
	return result;
}
// int CompruebaTeclaRaquetaDerecha (fsm_t* this):procedimiento de comprobacion de la posicion de la pelota.
// Si la pelota esta fuera del tablero devuelve un uno, en caso contrario 0;
int CompruebaPelota(fsm_t* this){
	if (juego.arkanoPi.pelota.y >= 6){
		return 1;
	}
	return 0;
}
// int CompruebaFinalJuego (fsm_t* this):procedimiento de comprobacion del estado del juego.
// Si no hay ladrillos o no quedan vidas se activa el flag final juego.
//Si esta activo devuelve un 1 si no un 0.
int CompruebaFinalJuego (fsm_t* this) {

	int result;
	//Si se cumplen las condiciones de final juego se activa el flag
	if( CalculaLadrillosRestantes((tipo_pantalla *)&(juego.arkanoPi.ladrillos)) == 0 || juego.arkanoPi.vidas.numeroVidas <=0){
		piLock (FLAGS_KEY);
		flags |= FLAG_FINAL_JUEGO;
		piUnlock (FLAGS_KEY);
	}

	piLock (FLAGS_KEY);
	result = (flags & FLAG_FINAL_JUEGO);
	piUnlock (FLAGS_KEY);
	return result;
}


//void IluminaColumnas: funci髇 encargada de iluminar las columnas
void IluminaColumnas (int i){
	int fila;
//Recorre el array de filas
	for(fila=0; fila<7; fila++){
		digitalWrite(GPIO_ROW[fila],HIGH);
	}

	switch(i) {
	case 0:
		digitalWrite(GPIO_COL_1, LOW);
		digitalWrite(GPIO_COL_2, LOW);
		digitalWrite(GPIO_COL_3, LOW);
		digitalWrite(GPIO_COL_4, LOW);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;

	case 1:
		digitalWrite(GPIO_COL_1, HIGH);
		digitalWrite(GPIO_COL_2, LOW);
		digitalWrite(GPIO_COL_3, LOW);
		digitalWrite(GPIO_COL_4, LOW);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;
	case 2:
		digitalWrite(GPIO_COL_1, LOW);
		digitalWrite(GPIO_COL_2, HIGH);
		digitalWrite(GPIO_COL_3, LOW);
		digitalWrite(GPIO_COL_4, LOW);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;

	case 3:
		digitalWrite(GPIO_COL_1, HIGH);
		digitalWrite(GPIO_COL_2, HIGH);
		digitalWrite(GPIO_COL_3, LOW);
		digitalWrite(GPIO_COL_4, LOW);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;
	case 4:
		digitalWrite(GPIO_COL_1, LOW);
		digitalWrite(GPIO_COL_2, LOW);
		digitalWrite(GPIO_COL_3, HIGH);
		digitalWrite(GPIO_COL_4, LOW);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;

	case 5:
		digitalWrite(GPIO_COL_1, HIGH);
		digitalWrite(GPIO_COL_2, LOW);
		digitalWrite(GPIO_COL_3, HIGH);
		digitalWrite(GPIO_COL_4, LOW);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;

	case 6:
		digitalWrite(GPIO_COL_1, LOW);
		digitalWrite(GPIO_COL_2, HIGH);
		digitalWrite(GPIO_COL_3, HIGH);
		digitalWrite(GPIO_COL_4, LOW);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;

	case 7:
		digitalWrite(GPIO_COL_1, HIGH);
		digitalWrite(GPIO_COL_2, HIGH);
		digitalWrite(GPIO_COL_3, HIGH);
		digitalWrite(GPIO_COL_4, LOW);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;

	case 8:
		digitalWrite(GPIO_COL_1, LOW);
		digitalWrite(GPIO_COL_2, LOW);
		digitalWrite(GPIO_COL_3, LOW);
		digitalWrite(GPIO_COL_4, HIGH);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;

	case 9:
		digitalWrite(GPIO_COL_1, HIGH);
		digitalWrite(GPIO_COL_2, LOW);
		digitalWrite(GPIO_COL_3, LOW);
		digitalWrite(GPIO_COL_4, HIGH);
		for(fila=0; fila<7; fila++){
			if(juego.arkanoPi.pantalla.matriz[i][fila]==1){
				digitalWrite(GPIO_ROW[fila], LOW);
			}}

		break;

	default:
		break;
	}
}



//void escribeVidas(): Funci髇 para sacar el numero de vidas actual en el display de 7 segmentos
// teniendo en cuenta que el maximo numero de vidas es 7
void escribeVidas() {
	if (juego.arkanoPi.vidas.numeroVidas == 0) {
		display(0,0,0);
	}
	else if (juego.arkanoPi.vidas.numeroVidas == 1) {
		display(0,0,1);
	}
	else if (juego.arkanoPi.vidas.numeroVidas == 2) {
		display(0,1,0);
	}
	else if (juego.arkanoPi.vidas.numeroVidas == 3) {
		display(0,1,1);
	}
	else if (juego.arkanoPi.vidas.numeroVidas == 4) {
			display(1,0,0);
		}
	else if (juego.arkanoPi.vidas.numeroVidas == 5) {
			display(1,0,1);
		}
	else if (juego.arkanoPi.vidas.numeroVidas == 6) {
			display(1,1,0);
		}
	else if (juego.arkanoPi.vidas.numeroVidas == 7) {
			display(1,1,1);
		}
}


// int RestaVidas (fsm_t* this): Actualiza la variable vidas
//poniendo la pelota en la posicion de inicio;
void RestaVidas(fsm_t*fsm){
	//Si caes pierdes vidas
	juego.arkanoPi.vidas.numeroVidas= juego.arkanoPi.vidas.numeroVidas- 1;
		juego.arkanoPi.pelota.x = 4;
		juego.arkanoPi.pelota.y = 1;
		juego.arkanoPi.pelota.xv = 0;
		juego.arkanoPi.pelota.yv = 1;
		escribeVidas();
}


//Antiguamente regia la deteccion por teclado para el manejo del juego
PI_THREAD (thread_explora_teclado) {
	char teclaPulsada;

	while(1) {
		delay(10); // Wiring Pi function: pauses program execution for at least 10 ms

		piLock (STD_IO_BUFFER_KEY);

		if(kbhit()) {
			teclaPulsada = kbread();
			piLock (FLAGS_KEY);
			flags |= FLAG_TECLA;
			piUnlock (FLAGS_KEY);
			//printf("Tecla %c\n", teclaPulsada);

			switch(teclaPulsada) {
			case 'i':
				piLock (FLAGS_KEY);
				flags |= FLAG_RAQUETA_IZQUIERDA;
				piUnlock (FLAGS_KEY);
				break;

			case 'o':
				piLock (FLAGS_KEY);
				flags |= FLAG_RAQUETA_DERECHA;
				piUnlock (FLAGS_KEY);
				break;

			case 'p':
				piLock (FLAGS_KEY);
				flags |= FLAG_PELOTA;
				piUnlock (FLAGS_KEY);
				break;
			case 'q':
				piLock (FLAGS_KEY);
				flags |= FLAG_FINAL_JUEGO;
				piUnlock (FLAGS_KEY);
				break;

			default:
				printf("INVALID KEY!!!\n");
				break;
			}
		}

		piUnlock (STD_IO_BUFFER_KEY);
	}
}

//void fsm_setup(fsm_t* luz_fsm): Pone a 0 la variable flags
void fsm_setup(fsm_t* luz_fsm) {
	piLock (FLAGS_KEY);
	flags = 0;
	piUnlock (FLAGS_KEY);
}

//void timer_isr_vida(union sigval value):
//Rutina de atenci髇 al timer de refresco de columnas
void timer_isr(union sigval value){

	IluminaColumnas(c);
	c++;

	if(c>=10){
		c=0;
	}
	tmr_startms(tmr, 1);
}
//void timer_isr_vida(union sigval value):
//Rutina de atenci髇 al timer de movimiento de pelota
void timer_isr_pelota(union sigval value){
	piLock (FLAGS_KEY);
	flags |= FLAG_PELOTA;
	piUnlock (FLAGS_KEY);

	tmr_startms(tmr_pelota, velocidad);
}

//void timer_isr_vida(union sigval value):
//Rutina de atenci髇 al timer de movimiento de vida
void timer_isr_vida(union sigval value){
	piLock (FLAGS_KEY);
	flags |= FLAG_VIDA;
	piUnlock (FLAGS_KEY);

	tmr_startms(tmr_vida, velocidad);
}


int main (){

	//Temporizadores
	tmr = tmr_new (timer_isr);
	tmr_startms(tmr ,1);
	tmr_pelota = tmr_new (timer_isr_pelota);
	tmr_startms(tmr_pelota,velocidad);
	tmr_vida = tmr_new (timer_isr_vida);
	tmr_startms(tmr_vida,velocidad);
//	tmr_pelota = tmr_new (timer_isr_vida);

	// Maquina de estados: lista de transiciones
	// {EstadoOrigen,FuncionDeEntrada,EstadoDestino,FuncionDeSalida}
	fsm_trans_t estados_tabla[] = {
			{ wait_start, CompruebaTeclaRaquetaIzquierda, wait_push, InicializaJuego },
			{ wait_start, CompruebaTeclaRaquetaDerecha, wait_start, AumentaVelocidad },
			{ wait_push, CompruebaTeclaPelota, wait_push, MovimientoPelota},
			{ wait_push, CompruebaFlagVida, wait_push, MovimientoVida},
			{ wait_push, CompruebaPelota, wait_push, RestaVidas},
			{ wait_push, CompruebaTiempoVida, wait_push, NuevaVida},
			{ wait_push, CompruebaTeclaRaquetaDerecha, wait_push, MueveRaquetaDerecha},
			{ wait_push, CompruebaTeclaRaquetaIzquierda, wait_push, MueveRaquetaIzquierda},
			{ wait_push, CompruebaFinalJuego, wait_end, FinalJuego},
			{wait_end, CompruebaTeclaPulsada, wait_start, ReseteaJuego},
			{ -1, NULL, -1, NULL },
	};

	fsm_t* juego_fsm = fsm_new (wait_start, estados_tabla, NULL);

	// Configuracion e inicializacion del sistema
	systemSetup();
	fsm_setup (juego_fsm);
	unsigned int next;
	next = millis();
	while (1) {
		fsm_fire (juego_fsm);
		next += CLK_MS;
		delay_until (next);
	}

	fsm_destroy (juego_fsm);

}
