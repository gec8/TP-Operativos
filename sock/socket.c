#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket.h"



/** \brief Funcion que se pone a la escucha de nuevas conexiones (Servidor)
 *  \param fd : el nuevo socket
 *  \param ip : ip donde estoy a la escucha
 *  \param puerto : puerto donde estoy a la escucha
 *  \return 0 si tuvo exito, -1 en caso contrario
 */
int ponerme_a_la_escucha (int *fd, char *ip, int puerto)
{
	int yes = 1; /* Para setsockopt() */
	int backlog = 10; /* Para listen(), Cuántas conexiones vamos a mantener en cola */
	struct sockaddr_in my_addr;


	/* Obtengo un descriptor de fichero para el socket */
	if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return -1;
	}


	/* Si una parte de un Socket todavía está colgada en el Kernel y cuando hacemos "Bind" nos tira: "ADDRES ALREADY IN USE", esto hace reutilizable ese puerto */
	if (setsockopt (*fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1)
	{
		return -1;
	}

	/* Relleno la estructura "sockaddr_in" correspodiente a mi dirección */
	my_addr.sin_family = AF_INET; /* Familia AF_INET, Ordenación de bytes de la Máquina */
	my_addr.sin_port = htons (puerto); /* Puerto aleatorio con la Ordenacion de la Red */
	my_addr.sin_addr.s_addr = inet_addr (ip); /* Mi direccion de IP */
	memset (&(my_addr.sin_zero), '\0', 8 );/* Pongo en cero el resto de la estructura */

	/* Asocio el puerto que me llego con el socket */
	if ( (bind( *fd, (struct sockaddr *) &my_addr, sizeof (struct sockaddr))) == -1)
	{
		return -1;
	}

	/* Me pongo a la escucha */
	if (listen (*fd, backlog) == -1 )
	{
		return -1;
	}

	return 0;
}



/** \brief Acepta una conexion entrante (Servidor)
 *  \param fd : el fd con el que acepto la conexion
    \return el fd con la nueva conexion
 */
int aceptar_conexion (int fd)
{
	int nueva_conexion = 0;
	int sin_size;
	struct sockaddr_in their_addr;


	/* Variable local que contiene la longitud de la estructura "sockaddr_in" */
	sin_size = sizeof (struct sockaddr_in);

	/* Aceptamos la conexion */
	if ((nueva_conexion = accept (fd, (struct sockaddr *) &their_addr,(socklen_t *) &sin_size)) == -1)
	{
		return -1;
	}

	return nueva_conexion;
}



/** \brief Funcion que se conecta a la ip y puerto especificado (Cliente)
 *  \param fd : el nuevo socket
 *  \param ip : ip donde me voy a conectar
 *  \param puerto : puerto donde me voy a conectar
 *  \return 0 si tuvo exito, -1 en caso contrario
 */
int conectar (int *fd, char *ip, int puerto)
{
	struct sockaddr_in dest_addr;


	if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return -1;
	}

	/* Relleno la estructura "sockaddr_in" correspodiente a la dirección de destino con la que me voy a conectar */
	dest_addr.sin_family = AF_INET; /* Familia AF_INET, Ordenación de bytes de la Máquina */
	dest_addr.sin_port = htons(puerto); /* Puerto al que voy a conectar con la Ordenación de la Red */
	dest_addr.sin_addr.s_addr = inet_addr(ip); /* Direccion de IP a la cual me voy a conectar */
	memset (&(dest_addr.sin_zero), '\0', 8 ); /* Pongo en cero el resto de la estructura */

	if ((connect(*fd, (struct sockaddr *) &dest_addr, sizeof (struct sockaddr))) == -1)
	{
		return -1;
	}

	return 0;
}



/** \brief Cierra una conexion, es decir libera un descriptor de socket (Servidor/Cliente)
 *  \param fd : el fd que se va a cerrar
 */
void desconectar (int *fd)
{
	if ( fd != 0 )
	{
		close (*fd);
	}
	return;
}



/** \brief Envia un mensaje: send()
 *  \param destino : el socket al cual se quieren enviar los datos
 *  \param msg : el mensaje a enviar
 *  \param longitud : tamaño del mensaje a enviar
 *  \return 0 si tuvo exito, -1 en caso contrario
 */
int enviar_mensaje (int destino, char *msg, tipo_int16 longitud)
{
	int total = 0;
	int bytes_pendientes = longitud;
	int bytes_enviados = -1;

   /* En caso de error en lectura de datos a enviar */
   if (msg == NULL || longitud < 1)
   {
    return -1;
   }

	/* Mientras todavía haya datos a enviar */
	while (total < longitud)
	{
		bytes_enviados = send (destino, msg + total, bytes_pendientes, 0);

		if (bytes_enviados == -1)
		{
			return -1;
		}

		total += bytes_enviados;
		bytes_pendientes -= bytes_enviados;
	}

	return 0;
}



/** \brief Lee/Recibe un mensaje: recv()
 *  \param origen : el socket de donde se quiere leer
 *  \param buffer : variable en donde se almacena el mensaje
 *  \param tam : tamaño del mensaje a enviar
 *  \return Al igual que la función recv() devuelve los bytes leídos, o -1 en caso de error
 */
int leer_mensaje (int origen, char *buffer, tipo_int16 tam)
{
	int bytes_recibidos = 0;

   /* En caso de error en lectura de datos a recibir */
   if (buffer == NULL || tam < 1)
   {
    return -1;
   }

	/* Mientras todavía haya datos a recibir */
	do
	{
		tam -= bytes_recibidos;
		bytes_recibidos += recv (origen, &buffer[bytes_recibidos], tam, 0);

		if (bytes_recibidos == -1)
		{

			desconectar (& origen);

			if (errno == 104)
				return 0;
			else
				return -1;
		}
		else if (bytes_recibidos == 0) /* Se cerró el socket */
		{

			desconectar (& origen);
			return 0;
		}
	} while (bytes_recibidos < tam);

	return bytes_recibidos;
}



/** \brief Empaqueta un mensaje a partir del tipo y la longitud del mensaje a enviar
 *  \param tipo : el identificador/tipo del mensaje
 *  \param msg : el mensaje que se desea empaquetar
 *  \param longitud : la longitud del mensaje que se desea empaquetar
 *  \return Un puntero con el mensaje a enviar.
 */
char *empaquetar_mensaje (tipo_int8 tipo, char *msg, tipo_int16 longitud)
{
	char *buffer = NULL;
	struct s_cabecera cabecera;

	/* calloc(), pide memoria dinámicamente y la inicializa */
	buffer = (char *) calloc (1, longitud + sizeof(struct s_cabecera) );

	if (buffer == NULL)
	{
		printf("Empaquetar_mensaje: Memoria Insuficiente! \n");
		return NULL;
	}

	/* Primero, arma la cabecera con el tipo y la longitud del mensaje */
	cabecera.tipo = tipo;
	cabecera.longitud = longitud;

	/* Segundo, copio al buffer el contenido de la cabecera del mensaje */
	memcpy (& buffer [0], & cabecera, sizeof(struct s_cabecera));

	/* Ultimo, copio al buffer el mensaje propiamente dicho pegado atras de la cabecera del mensaje ya almacenada */
	if ( longitud != 0 )
		memcpy (& buffer [sizeof (struct s_cabecera)], msg, longitud);

	return (char *) buffer;
}



/** \brief Desempaqueta un mensaje. Lee/Recibe un mensaje: recv()
 *  \param fd : el socket de donde se quiere leer
 *  \param mensaje : un puntero de tipo estructura s_mensaje (el mensaje se devuelve por referencia a esta variable)
 *  \return La cantidad de bytes recibidos (bytes_leidos)
 */
int desempaquetar_mensaje (int fd, struct s_mensaje * mensaje)
{
	int bytes_leidos = 0;

	memset (mensaje, '\0', sizeof(struct s_mensaje) );


	bytes_leidos = leer_mensaje (fd, (char * ) & mensaje->cabecera, sizeof(struct s_cabecera) );

	if ( (bytes_leidos == -1) || (bytes_leidos == 0 ) )
		return bytes_leidos;

	/* Esto sucede cuando se envia un mensaje vacio, es decir la Id ("tipo" de la struct s_cabecera) sola, sin datos. Si esto ocurre no tengo que leer, sino se bloquea en el recv() */
	if (mensaje->cabecera.longitud == 0)
		mensaje->msg = NULL;
	else
	{
		mensaje->msg = (char *) calloc(1, mensaje->cabecera.longitud);

		if ( mensaje->msg == NULL)
		{
			printf("desempaquetar_mensaje: Memoria Insuficiente! \n");
			return -1;
		}


		bytes_leidos = leer_mensaje (fd, mensaje->msg, mensaje->cabecera.longitud);

		if ( (bytes_leidos == -1) || (bytes_leidos == 0 ) )
			return bytes_leidos;
	}

	return bytes_leidos;
}


