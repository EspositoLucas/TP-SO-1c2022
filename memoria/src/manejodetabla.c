#include "manejodetabla.h"

bool indicador ;

//------------------------ACCESO A PRIMERA TABLA------------------------
uint32_t devolver_entrada_a_segunda_tabla(uint32_t tabla,uint32_t entrada){
	t_list* tabla_primer_nivel_proceso = (t_list*)list_get(lista_tabla_de_pagina_1_nivel,tabla);
	t_p_1* tabla_1 = (t_p_1*)list_get(tabla_primer_nivel_proceso,entrada);
	return tabla_1->numero_de_tabla2;
}


//------------------------ACCESO A SEGUNDA TABLA----------------------------
uint32_t devolver_marco(uint32_t tabla,uint32_t entrada){
	tabla_de_segundo_nivel*tabla_elegida=(tabla_de_segundo_nivel*)list_get(lista_tablas_segundo_nivel,tabla);
	t_p_2* pagina=(t_p_2*)list_get(tabla_elegida->lista_paginas,entrada);

	log_info(memoria_logger,"pagina a usar para reemplazo  %d \n",pagina->indice);
	uint32_t indice_tabla_en_swap = devolverNroTablaEnSwap(tabla_elegida->p_id,tabla_elegida->id_tabla);;
	if(pagina->p){
		log_info(memoria_logger,"Pagina presente en memoria, no hay page fault \n");
		usleep(config_valores_memoria.retardo_memoria); // retardo memoria por el page fault y hay que ir al archivo a buscar la pagina y cargarla en memoria
		pagina->u = 1;
		return pagina->marco;
	}
	else{ // page fault
		log_info(memoria_logger,"Page Fault, buscar marco libre \n");
		pthread_mutex_lock(&mutex_contador_pid);
		contador_por_pid* contador  = (contador_por_pid*)list_get(contador_pid,tabla_elegida->p_id);
		contador->contadorPF +=1 ;
		pthread_mutex_unlock(&mutex_contador_pid);
		usleep(config_valores_memoria.retardo_swap); //retardo swap
		if(cantidadUsadaMarcos(tabla_elegida->p_id)<config_valores_memoria.marcos_por_proceso){//si el proceso todavía no uso la cantidad máxima de marcos por proceso
			pagina->marco=ocuparMarcoLibre(tabla_elegida->p_id);//busca un marco libre y se lo asigna ala pagina
			pagina->p=true;
			asignarAlArchivo(tabla_elegida->p_id);
			log_info(memoria_logger,"antes de traer pagina swap  \n");
			void* paginaTraida=traerPaginaDeSwap(pagina->indice + indice_tabla_en_swap * config_valores_memoria.entradas_por_tabla);//trae la pagina desde el swap
			log_info(memoria_logger,"pagina traida de swap \n");
			escribirPagEnMemoria(paginaTraida,pagina->marco);//la escribe en memoria
			log_info(memoria_logger,"Se escribio pagina traida de swap en memoria \n");
			pagina->p=true;//pagina ahora está presente en memoria
			return pagina->marco;

		}
		log_info(memoria_logger,"Cant max de marcos del proceso ocupados,obtener pagina reemplazar \n");
		uint32_t numPagAReemplazar=obtenerPaginaAReemplazar(tabla_elegida->p_id);
		log_info(memoria_logger,"pagina a reemplazar  %d \n",numPagAReemplazar);
		log_info(memoria_logger,"Se obtuvo pagina a reemplazar \n");
		uint32_t marcoAUsar=escribirModificaciones(numPagAReemplazar,tabla_elegida->p_id);//Veo que marco voy a usar para traer la pagina
		log_info(memoria_logger,"Se escribio pagina modificada en swap \n");															 //Si la pag reemplazada tiene m=1 => la escribo en swap, además, se le pone p=0
		pagina->marco=marcoAUsar;
		asignarAlArchivo(tabla_elegida->p_id);
		void* paginaTraida=traerPaginaDeSwap(pagina->indice + indice_tabla_en_swap * config_valores_memoria.entradas_por_tabla);//trae la pagina desde el swap
		log_info(memoria_logger,"pagina traida de swap \n");
		//asignarAlArchivo(tabla_elegida->p_id);
		escribirPagEnMemoria(paginaTraida,marcoAUsar);//la escribe en memoria, reemplazando a la anterior
		log_info(memoria_logger,"Se escribio y reemplazo pagina traida de swap en memoria \n");
		pagina->p=true;//pagina ahora está presente en memoria
		t_list* aux =paginasEnMemoria(tabla_elegida->p_id);
		log_info(memoria_logger,"Paginas en memoria despues del reemplazo con n de pagina, u, m y si es puntero\n");
		    for (int i = 0; i < list_size(aux); i++){
		    	t_p_2* auxiliar = (t_p_2*)list_get(aux,i);
		    	log_info(memoria_logger,"Pagina numero: %d, U: %d, M: %d, P: %d\n", auxiliar->indice, auxiliar->u,auxiliar->m,auxiliar->puntero_indice);
		    }
		log_info(memoria_logger,"pagina presente en memoria \n");
		return pagina->marco;
	}
}

//---------------------------CREAR TABLA DE SEGUNDO NIVEL----------------------------------

t_list* inicializar_tabla_segundo_nivel(){
	t_list* tabla_segundo_nivel=list_create();
	for(int i=0;i<config_valores_memoria.entradas_por_tabla;i++){
		t_p_2* entrada_tp2=malloc(sizeof(t_p_2));
		entrada_tp2->indice=(uint32_t)i;
		entrada_tp2->m=0;
		entrada_tp2->p=0;
		entrada_tp2->u=0;
		list_add(tabla_segundo_nivel,entrada_tp2);
	}

	return tabla_segundo_nivel;
}


///------------ALGORITMO_REEMPLAZO---------------

uint32_t  obtenerPaginaAReemplazar(uint32_t pid){

	uint32_t pagina_reemplazo = 0;

	t_list* tabla_marcos = paginasEnMemoria(pid);

	if(list_all_satisfy(tabla_marcos,tienePunteroEnCero)) {
		t_p_2* pag_aux = (t_p_2*)list_get(tabla_marcos,0);
		pag_aux->puntero_indice = 1;
	}
	for(int i = 0 ; i < list_size(tabla_marcos); i++){
			t_p_2* aux = (t_p_2*)list_get(tabla_marcos,i);
			log_info(memoria_logger,"Paginas en memoria antes de algoritmo %d \n",aux->indice);
		}
 		switch(algoritmo_memoria){
 		case CLOCK:
 			pagina_reemplazo = obtenerPaginaClock(tabla_marcos,pid);
 			log_info(memoria_logger,"pagina reemplazada %d en CLOCK  del proceso %d\n",pagina_reemplazo,pid);
 			break;
 		case CLOCK_M:
 			pagina_reemplazo = obtenerPaginaClockM(tabla_marcos,pid);
 			log_info(memoria_logger,"pagina reemplazada %d en CLOCK-M  del proceso %d\n",pagina_reemplazo,pid);
 			break;
 		default:break;
 		}
 		log_info(memoria_logger,"pag reemplazada despues de algoritmo %d \n",pagina_reemplazo);
 	return pagina_reemplazo;
 }

bool tienePunteroEnCero(t_p_2* pagina ) {
	return pagina->puntero_indice == 0;

}

algoritmo obtener_algoritmo(){

 	 algoritmo switcher;
 	 char* algoritmo_reemplazo = config_valores_memoria.algoritmo_reemplazo ;
 	    //CLOCK
 	 if (strcmp(algoritmo_reemplazo,"CLOCK") == 0)
 	 {
 		 switcher = CLOCK;
 	     log_info(memoria_logger, "El algoritmo de planificacion elegido es CLOCK.\n");
 	 }

 	    //CLOCK-M
 	 if (strcmp(algoritmo_reemplazo,"CLOCK-M") == 0)
 	 {
 		 switcher = CLOCK_M;
 	     log_info(memoria_logger, "El algoritmo de planificacion elegido es CLOCK-M.\n");
 	 }
 	 return switcher;
}


uint32_t obtenerPaginaClock(t_list* lista,uint32_t pid){

    t_p_2* pagina=(t_p_2*) list_find(lista,punteroEnUno);
    uint32_t numeroPaginaInicial=pagina->indice;
    t_p_2* averiguaIndice;
    uint32_t indice = 0 ;
    for (int i=0;i<list_size(lista);i++){
        averiguaIndice=list_get(lista,i);
        if(averiguaIndice->indice==pagina->indice){
            indice=i;
            break;
        }
    }
    t_p_2* aux;
    t_p_2* siguiente;
    cambiarPunterodePagina(pagina->indice,pid,0);

    while(1){
    for (int i =indice ; i < list_size(lista); i++){
        aux = list_get(lista, i);
        if (aux->u == 0){
            if(i==list_size(lista)-1){
                siguiente=list_get(lista, 0);
            }else{
                siguiente=list_get(lista, i + 1);
            }
            cambiarPunterodePagina(siguiente->indice,pid,true);
             return aux->indice;
        } else {
            cambiarUdePagina(aux->indice,pid,false);
            aux->u = 0 ;
        }
    }
    indice=0;
    }
    return numeroPaginaInicial;
}



bool punteroEnUno(t_p_2* pagina){
    return pagina->puntero_indice==1;
}


uint32_t obtenerPaginaClockM(t_list* lista,uint32_t pid){


    log_info(memoria_logger,"Paginas a analizar para el reemplazo con n de pagina, u, m y si es puntero\n");
    for (int i = 0; i < list_size(lista); i++){
    	t_p_2* auxiliar = (t_p_2*)list_get(lista,i);
    	log_info(memoria_logger,"Pagina numero: %d, U: %d, M: %d, Puntero: %d\n", auxiliar->indice, auxiliar->u,auxiliar->m,auxiliar->puntero_indice);
    }


    t_p_2* pagina=(t_p_2*) list_find(lista,punteroEnUno);
    t_p_2* aux;
    t_p_2* siguiente;
    uint32_t numeroPaginaInicial=pagina->indice;
    t_p_2* averiguaIndice;
    uint32_t indice = 0;
    cambiarPunterodePagina(pagina->indice,pid,0);

    for (int i=0;i<list_size(lista);i++){
    	averiguaIndice=list_get(lista,i);
    	if(averiguaIndice->indice==pagina->indice){
    		indice=i;
    		break;
    	}
    }


  while(1){
    	for (int i =indice ; i < list_size(lista); i++){
    		aux = list_get(lista, i);
    		if (aux->u == 0 && aux->m == 0){
    			if(i==list_size(lista)-1){
    				siguiente=list_get(lista, 0);
    			}else{
    				siguiente=list_get(lista, i +1);
    			}
    			cambiarPunterodePagina(siguiente->indice,pid,true);
               log_info(memoria_logger,"Pagina elegida para ser reemplaza [%d] U: %d, M: %d, Puntero: %d", aux->indice, aux->u, aux->m, aux->puntero_indice);
    			return aux->indice;
    		}
    	}

    	for (int i =0 ; i < indice; i++){
            aux = list_get(lista, i);
            if (aux->u == 0 && aux->m == 0){
                if(i==list_size(lista)-1){
                    siguiente=list_get(lista, 0);
                }else{
                    siguiente=list_get(lista, i +1);
                }
                cambiarPunterodePagina(siguiente->indice,pid,true);
              log_info(memoria_logger,"Pagina elegida para ser reemplaza [%d] U: %d, M: %d, Puntero: %d", aux->indice, aux->u, aux->m, aux->puntero_indice);
                 return aux->indice;
            }
    	}

    	for(int i= indice ; i< list_size(lista);i++){
    		aux = list_get(lista, i);
    	    if (aux->u == 0 && aux->m ){
    	    	if(i==list_size(lista)-1){
    	    		siguiente=list_get(lista, 0);
    	    	}else{
    	    		siguiente=list_get(lista, i +1);
    	    	}
    	        cambiarPunterodePagina(siguiente->indice,pid,true);
                log_info(memoria_logger,"Pagina elegida para ser reemplaza [%d] U: %d, M: %d, Puntero: %d", aux->indice, aux->u, aux->m, aux->puntero_indice);
    	        return aux->indice;
    	    }else {
    	    	cambiarUdePagina(aux->indice,pid,false);
    	    	aux->u = 0 ;
    	    }
    	}

    	for(int i= 0 ; i< indice;i++){
    		aux = list_get(lista, i);
        	if (aux->u == 0 && aux->m ){
        		if(i==list_size(lista)-1){
        			siguiente=list_get(lista, 0);
        		}else{
        			siguiente=list_get(lista, i +1);
        		}
        		cambiarPunterodePagina(siguiente->indice,pid,true);
                log_info(memoria_logger,"Pagina elegida para ser reemplaza [%d] U: %d, M: %d, Puntero: %d", aux->indice, aux->u, aux->m, aux->puntero_indice);
        	    return aux->indice;
        	}else {
        		cambiarUdePagina(aux->indice,pid,false);
        		aux->u = 0 ;
        	}
    	}
   }
    return numeroPaginaInicial;
}

bool punteroUyMEnCero(t_p_2* pagina){
    return !pagina->u && !pagina->m ;
}

bool punteroUEnCeroyMEnUno(t_p_2* pagina){
    return !pagina->u && pagina->m ;
}


t_list *paginas_por_proceso(int pid){

	t_list* lista_pags_por_proceso = list_create();
	tabla_de_segundo_nivel *aux;
	for (int i = 0; i < list_size(lista_tablas_segundo_nivel); i++){
		aux = list_get(lista_tablas_segundo_nivel,i);
		if (aux->p_id == pid){
			list_add(lista_pags_por_proceso,aux);
		}
	}
	return lista_pags_por_proceso;

}


bool condicion_misma_numero_p_id( tabla_de_segundo_nivel* tabla){
	return tabla->p_id == numero_tabla_2p; // ver con que se compara el p_id

}

bool pagina_con_presencia(void* tabla){
	return ((t_p_2 *)tabla)->p ;

}

bool pagina_con_modificado(t_p_2 *pagina){
	return pagina->m;
}

bool pagConIgualPid(tabla_de_segundo_nivel* tab){
	return tab->p_id==pid_comparador;
}

bool pagConIgualIndice(t_p_1* tab){
	return tab->numero_de_tabla2== indice_comparador;
}

t_list* paginasEnMemoria(uint32_t pid){

	pthread_mutex_lock(&mutex_comparador_pid);
	pid_comparador=pid;
	pthread_mutex_unlock(&mutex_comparador_pid);
	t_list *lista_pags_en_mem = list_create();
	t_list* tablas_del_proceso=(t_list*)list_filter(lista_tablas_segundo_nivel,pagConIgualPid);
	for(int i=0;i<list_size(tablas_del_proceso);i++){
		tabla_de_segundo_nivel* aux=(tabla_de_segundo_nivel*)list_get(tablas_del_proceso,i);
		t_list* pags_en_memoria=(t_list*)list_filter(aux->lista_paginas,pagina_con_presencia);
		for(int j=0;j<list_size(pags_en_memoria);j++){
			t_p_2* pagReal=malloc(sizeof(t_p_2));
			t_p_2* pagEnMemoria=list_get(pags_en_memoria,j);
			pagReal->m=pagEnMemoria->m;
			pagReal->marco=pagEnMemoria->marco;
			pagReal->p=pagEnMemoria->p;
			pagReal->puntero_indice=pagEnMemoria->puntero_indice;
			pagReal->u=pagEnMemoria->u;
			pagReal->indice=pagEnMemoria->indice+config_valores_memoria.entradas_por_tabla*i;
			list_add(lista_pags_en_mem,pagReal);
		}
	}

	list_sort(lista_pags_en_mem, marcosMin);
	return lista_pags_en_mem;
}

bool marcosMin(void* tp1, void* tp2){
	return (((t_p_2 *)tp1)->marco < ((t_p_2 *)tp2)->marco);
}
t_list* pagsDeUnProceso(uint32_t pid){
	t_list* paginas=list_create();
	pthread_mutex_lock(&mutex_comparador_pid);
	pid_comparador=pid;
	pthread_mutex_unlock(&mutex_comparador_pid);
	t_list* tablas=(t_list*)list_filter(lista_tablas_segundo_nivel,pagConIgualPid);
	for(int i=0;i<list_size(tablas);i++){
		tabla_de_segundo_nivel* aux=(tabla_de_segundo_nivel*)list_get(tablas,i);
		for(int j=0;j<list_size(aux->lista_paginas);j++){
			t_p_2* pagTabla=(t_p_2*) list_get(aux->lista_paginas,j);
			t_p_2* pagReal=malloc(sizeof(t_p_2));
			pagReal->m=pagTabla->m;
			pagReal->marco=pagTabla->marco;
			pagReal->p=pagTabla->p;
			pagReal->puntero_indice=pagTabla->puntero_indice;
			pagReal->u=pagTabla->u;
			pagReal->indice=pagTabla->indice+10*i;
			list_add(paginas,pagReal);
		}
	}
	return paginas;
}



uint32_t* indice_tabla_tp2(tabla_de_segundo_nivel* tp2){
	return tp2->id_tabla;
}

uint32_t devolverNroTablaEnSwap(uint32_t pid,uint32_t tabla){

		pthread_mutex_lock(&mutex_comparador_pid);
		pid_comparador=pid;
		pthread_mutex_unlock(&mutex_comparador_pid);
		t_list* tablas_del_proceso=(t_list*)list_filter(lista_tablas_segundo_nivel,pagConIgualPid);

		for(uint32_t i = 0 ; i<list_size(tablas_del_proceso);i++){
			tabla_de_segundo_nivel* aux = (tabla_de_segundo_nivel*)list_get(tablas_del_proceso,i);
			if(aux->id_tabla == tabla){
				return i ;
			}

		}
		exit(30);
}

