#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "comunicacion.h"
#include "log_config.h"
#include "sockets.h"
#include "manejodetabla.h"
#include "swap.h"


// STRUCTS/ENUMS

 typedef struct  // archivo de configuracion memoria
 {
 	char* ip_memoria;
 	char* puerto_escucha;
 	int tam_memoria;
 	int tam_pagina;
 	int entradas_por_tabla;
 	int retardo_memoria;
 	char* algoritmo_reemplazo;
 	int marcos_por_proceso;
 	int retardo_swap;
 	char* path_swap;
 } arch_config;


t_list* tabla_de_pagina_1_nivel;


arch_config config_valores_memoria;

//VARIABLES
int socket_memoria;

void* memoria_usuario;
int cantidad_de_marcos;
char* pathSwap;
algoritmo algoritmo_memoria;
t_list* marcos;

//SEMAFOROS

pthread_mutex_t mutex_comparador_pid;
pthread_mutex_t mutex_comparador;
pthread_mutex_t mutex_marcos;

//FUNCIONES

void cargar_configuracion();
t_paquete* preparar_paquete_para_handshake();
void manejo_instrucciones(t_list*,int);
void traducir_operandos(void*,uint32_t*,uint32_t*);
void inicializar_memoria();
void manejo_conexiones(int);
int atender_clientes_memoria(int);
void suspender_proceso(int);


void liberarMarco(uint32_t);
void liberarPag(uint32_t );
void liberarTodosLosMarcos(uint32_t);
int ocuparMarcolibre(uint32_t pid);
bool estaLibre(marquito* );
bool igualPid(marquito*);
int cantidadUsadaMarcos(uint32_t pid);
void inicializar_marcos();
double marcosTotales();
t_list* marcosPid(uint32_t );

int pags_proceso(uint32_t ,int );
int tp2_proceso(int ,int );

uint32_t leer_de_memoria(uint32_t);
void escribirEn(uint32_t , uint32_t);
bool tiene_mismo_indice(tabla_de_segundo_nivel*);
int get_marco_offset(uint32_t indice);



#endif
