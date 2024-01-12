# Client-Server Sockets

Sistema de comunicacion entre procesos por medio de sockects con arquitectura cliente-servidor.

### Autores:
- **Bottini, Franco Nicolas**

### ¿ Como compilar ?

Para compilar el proyecto, una vez clonado el repositorio, basta con crear el Makefile utilizando el script CMake y ejecutarlo:

```bash
$ git clone https://github.com/francobottini99/LINUXCLIENTSERVER2-2023.git
$ cd LINUXCLIENTSERVER2-2023
$ cmake .
$ make
```

Como salida obtendremos dos ejecutables ubicados en la carpeta `/bin`: `client` y `server`.

> [!NOTE] 
> Para compilar el proyecto es necesario tener instalado el paquete `ZLIB` en el equipo.

## Cliente

Con el binario `client` se generan los procesos que se van a comunicar con el servidor por medio de sockets. Los clientes se pueden conectar con el servidor mediante tres tipos de sockets distintos: *UNIX* (0), *IPV4* (1) y *IPV6* (2). Ademas, Existen tres tipos de clientes, estos se diferencian en el tipo de tarea que solicitan realizar al servidor, estos son:

- *CLIENT_A* (0) -> Le solicita al usuario que ingrese un comando por consola que luego deriva al servidor para que este interactué con journalctl y obtener un resultado que imprime por consola. Este proceso se repite hasta que se finalice la instancia del cliente o del servidor.

- *CLIENT_B* (1) -> Le solicita al usuario que ingrese un comando por consola que luego deriba al servidor para que este interactué con journalctl y obtener el resultado en un archivo comprimido que almacena en la carpeta `/data` con el nombre `client_b_result_[yyyy]_[mm]_[dd]_[HH]_[mm]_[ss]`. Este proceso se repite hasta que se finalice la instancia del cliente o del servidor.

- *CLIENT_C* (2) -> El cliente directamente solicita al servidor un informe con datos del sistema que imprime por consola una vez obtenido. Este proceso se ejecuta una única vez y luego finaliza la instancia del cliente.

Se puede hacer uso de este programa de la siguiente manera:

```bash
$ ./bin/Client 0 0          # Ejecuta un cliente A que se conecta al servidor por medio de un socket UNIX
$ ./bin/Client 0 1 [IPV4]   # Ejecuta un cliente A que se conecta al servidor por medio de un socket IPV4
$ ./bin/Client 0 2 [IPV6]   # Ejecuta un cliente A que se conecta al servidor por medio de un socket IP6 

$ ./bin/Client 1 0          # Ejecuta un cliente B que se conecta al servidor por medio de un socket UNIX
$ ./bin/Client 1 1 [IPV4]   # Ejecuta un cliente B que se conecta al servidor por medio de un socket IPV4
$ ./bin/Client 1 2 [IPV6]   # Ejecuta un cliente B que se conecta al servidor por medio de un socket IP6 

$ ./bin/Client 2 0          # Ejecuta un cliente C que se conecta al servidor por medio de un socket UNIX
$ ./bin/Client 2 1 [IPV4]   # Ejecuta un cliente C que se conecta al servidor por medio de un socket IPV4
$ ./bin/Client 2 2 [IPV6]   # Ejecuta un cliente C que se conecta al servidor por medio de un socket IP6 
```

Para utilizar los sockets IPV4 y IPV6 se debe especificar como tercer argumento la IP del servidor con el cual se quiere establecer conexion. Se pueden ejecutar tantos procesos cliente como se desee.

## Server

Con el binario `server` se genera el proceso que crea los sockects a los cuales se van a conectar los clientes y se encarga de atender las solicitudes de los mismos. Para ejecutar el servidor basta con ejecutar el binario:

```bash
$ ./bin/server # Ejecuta el Servidor
```

El proceso no admite múltiples ejecuciones, es decir, no puede haber mas de un proceso servidor corriendo en el equipo al mismo tiempo.

## Funcionamiento

Al ejecutar el servidor se crea un socket de tipo **UNIX** en el equipo y se que queda a la espera de que se conecten clientes. Una vez que se conecta un cliente, este le enviá un mensaje al servidor indicándole el tipo de cliente que es y el servidor crea un hilo hijo que se encarga de atender las solicitudes de ese cliente. El hilo principal vuelve a quedar a la espera de que se conecten mas clientes. El hilo hijo atiende las solicitudes del cliente hasta que este se desconecta, en ese momento el hilo hijo finaliza su ejecución. El servidor puede atender múltiples clientes al mismo tiempo.

Para la comunicación a través del sockect, tanto el cliente como el servidor hacen uso de **API** de comunicación. Esta interfaz recibe la información a transmitir por el sockect en un *buffer* de memoria y ejecuta el siguiente protocolo:

- Los datos a transmitir pasan por un proceso de fragmentación y encapsulación donde se generan `N` paquetes de tamaño fijo. Cada paquete queda encapsulado en una estructura que contiene un *array* `data` donde se almacena la información del fragmento y un conjunto de *headers*. Estas *headers* incluyen: un *checksum* calculado a partir del array `data`, el tamaño total de los datos antes de fragmentar, el tamaño del fragmento actual y un *flag* que indica si este es el ultimo fragmento generado. Ademas de esto, en la estructura se incluye un puntero al siguiente fragmento en la lista. La estructura es la siguiente:

```c
/**
 * @brief Data fragment
 *
 */
typedef struct fragments 
{
    int checksum;                   // Checksum of data
    size_t total_size;              // Total size of all fragments
    size_t content_size;            // Total size of this fragment
    uint8_t last;                   // Last fragment flag
    char data[DATA_FRAGMENT_SIZE];  // Data of fragment
    struct fragments* next;         // Next fragment
} fragments;
```

- Posterior a esto, Cada paquete pasa por una rutina donde se le da un formato de cadena **JSON**. Por ejemplo, si se quiere transmitir la entrada: `-n 10`, se genera el siguiente paquete:

```json
{
    "checksum": 1982818317,
    "total_size": 6,
    "content_size": 6,
    "last": 1,
    "data": [45,110,32,49,48,0]
}
```

- La cadena **JSON** se transmite por un extremo del sockect y se recibe por el otro. El receptor deserializa la cadena para obtener el paquete original, el receptor del paquete verifica que el *checksum* del paquete recibido sea igual al *checksum* calculado por el emisor. Si esto es así, se procede a almacenar el paquete en memoria. En caso contrario, se descarta el paquete y se solicita al emisor que lo reenvié. Este proceso se repite hasta que se reciben todos los fragmentos que conforman la información original.

- Finalmente, una vez obtenidos y almacenados todos los fragmentos, se realiza un proceso de desfragmentación, que desencapsula los datos y rearma la información original.