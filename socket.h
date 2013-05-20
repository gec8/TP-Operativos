/*
 *        socket.h Headers
 */
#ifndef _SOCKET_H
#define _SOCKET_H

#include <netinet/in.h>
#include <arpa/inet.h>


/* Declaración del typedef para la estructura cabecera */
typedef  int8_t  tipo_int8;   /* Asociación: signed char int8_t */
typedef  int16_t tipo_int16;  /* Asociación: signed int  int16_t*/



/** Estructura cabecera.
 *  Es utilizada para almacenar el tipo y la longitud del mensaje a enviar ó recibir
 * */
typedef struct s_cabecera
{
    tipo_int8  tipo;
    tipo_int16 longitud;
} cabecera;


/** Estructura mensaje.
 *  Es utilizada para almacenar la cabecera y el mensaje a enviar ó recibir
 * */
typedef struct s_mensaje
{
	struct s_cabecera cabecera;
	char * msg;
} mensaje;


/* Funciones esenciales para el manejo de sockets */
int ponerme_a_la_escucha (int *fd, char *ip, int puerto);  // Funcion exclusiva de Servidor
int aceptar_conexion (int fd);  // Funcion exclusiva de Servidor
int conectar (int *fd, char *ip, int puerto);  //Funcion exclusiva de Cliente
void desconectar (int *fd);

/* Funciones para leer y recibir datos */
int enviar_mensaje (int destino, char *msg, tipo_int16 longitud);
int leer_mensaje (int origen, char *buffer, tipo_int16 tam);
char *empaquetar_mensaje (tipo_int8 tipo, char *msg, tipo_int16 longitud);
int desempaquetar_mensaje (int fd, struct s_mensaje * mensaje);


#endif /* socket.h */

