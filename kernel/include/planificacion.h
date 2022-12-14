#ifndef PLANIFICACION_H
#define PLANIFICACION_H


#include "kernel.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "comunicacion.h"
#include "log_config.h"
#include "sockets.h"
#include "planificacionUtils.h"

//VARIABLES
proceso* procesoAux;
int proceso_ejecutando;
int socket_proceso_exec ;
int tiempo_inicio_bloqueo;
bool interrupcion;

// FUNCIONES


//Largo Plazo

pcb *crear_estructura_pcb(t_consola*);
 void iniciar_planificador_largo_plazo(void);
 void agregarANewPcb(proceso* );
 void transicion_admitir_por_prioridad(void) ;
 void finalizarPcb();


//Mediano Plazo

 void iniciar_planificador_mediano_plazo(void);
 void transicion_suspender(proceso *);
 void estado_suspended_ready(void );

// Corto Plazo

 void iniciar_planificador_corto_plazo(void);
void estadoReady(void);
void estadoExec(void);
void estadoBlockeado(void);

int socket_interrupt ;



#endif
