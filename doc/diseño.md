
# Redes 2 - Servidor

## Introducción

El objetivo de esta practica es implementar un servidor para el protocolo IRC. Dicho servidor sera capaz de atender las peticiones de múltiples clientes según lo descrito en los RFCs 1459 y 2812.

## Diseño

### Módulos del programa

Hemos dividido el programa en 5 módulos diferentes:

* `Channel` representa un canal IRC.
* `Server` contiene la estructura de servidor y el programa principal.
* `Switches` contiene las implementaciones de las funciones IRC.
* `User` encapsula las conexiones con usuarios.

#### User
Un usuario contiene el socket de un cliente, la referencia a su hilo de ejecución y finalmente todos los sus datos como su nick, nombre y flags de usuario.

### Decisiones de diseño

La implementación es concurrente ya que da mas flexibilidad y posibilidad de extensión al servidor.

Hemos desarrollado un servidor con un diseño concurrente. A cada conexión entrante se le asigna un hilo para que atienda sus peticiones. El hilo queda bloqueado al llegar a `recv()` hasta que reciba datos del cliente o pasen 10 segundos, lo que ocurra primero. Ademas, comprobara periódicamente una bandera en su estructura para saber si debe finalizar la conexión y matar el hilo. De este modo, cada hilo controla su propia ejecución y evitamos las condiciones de carrera que pueden surgir a partir de `pthread_kill` o `pthread_cancel` inoportunos.

Usamos hilos en vez de procesos para cada cliente. Nota: las razones a continuación están basadas en la Implementación de pthreads de Linux, y no tienen porque aplicarse a otros sistemas operativos.

* Son considerablemente mas ligeros que los procesos, siendo su creación y destrucción mucho mas rápida.
* La comunicación entre hilos no requiere asistencia del sistema operativo, ya que comparten la memoria.
* Es mas facil controlarlos ya que cuando muere un proceso mueren todos sus hilos. Con procesos

Disponemos de otros dos hilos en el modulo `Server`:

* une encarga de mandar `PING`s periódicos a los usuarios para comprobar que sigan vivos. Se mandaran pings aunque la c


estructuras de datos, etc.


## Funcionalidad IRC

Hemos implementado las siguientes funciones

* `ADMIN` - Imprime la información del administrador del servidor.
* `AWAY` - Marca/desmarca un usuario como ausente.
* `INFO` - Devuelve información general sobre el servidor.
* `ISON` - Comprueba si un usuario esta conectado
* `JOIN` - Comando para unirse a una sala
* `KICK` - Expulsa a un usuario de una sala.
* `LIST` - Lista todos los canales visibles del servidor.
* `LUSERS` - Envía información sobre el numero de usuarios.
* `MODE` - Comando que realiza diversas operaciones con los modos
* `MOTD` - Envía el mensaje del día a un usuario
* `NAMES`
* `NICK`
* `NOTICE`
* `OPER`
* `PART` - Abandona un canal
* `PING` - Envía un ping a un usuario
* `PONG` - Envía un pong a un usuario
* `PRIVMSG` - Manda un mensaje a un canal o usuario
* `QUIT` - Cierra la conexión con el servidor
* `SUMMON` - Manda el mensaje de error especificado por el RFC por cuestiones de seguridad.
* `TIME` - Imprime el tiempo en el servidor
* `TOPIC` - Cambia el tema de un canal
* `USER` - Mensaje del handshake IRC inicial
* `USERHOST` - Muestra el host de un usuario
* `USERS` - Manda el mensaje de error especificado por el RFC por cuestiones de seguridad.
* `VERSION` - Imprime la version del servidor
* `WALLOPS` - Envía el comando a todos los usuarios `+w`.
* `WHO` - Lista la gente que hay en un canal
* `WHOIS` -
* `WHOWAS` - Lista la información sobre un usuario que ya se desconecto

## Conclusiones técnicas

A continuación están todas las cosas que hemos aprendido haciendo esta practica:

Aprendido a usar telnet

## Conclusiones personales

Otra dificultad ha sido las inconsistencias entre los RFCss

Los RFCs están mal hechos
xchat no respeta el RFC
la librería usada no tiene ni un solo `const`.

El comando MODE esta muy sobrecargado

contradicciones entre los RFC 1459 y RFC 2812

erratas del rfc

las habilidades siendo evaluadas son la lectura de RFC y la capacidad de programacion de los estudiantes


No ha habido una publicación de unos criterios de corrección. Por tanto, no hemos tenido manera clara de evaluar el progreso. A esto le sumamos la rigidez en la normativa de entrega, que enuncia:

  > El incumplimiento de cualquier punto de la normativa de entrega
  > llevará automáticamente a que la práctica no sea corregida y por
  > tanto tenga, automáticamente un 0.




* La no-publicación del código fuente de la librería de procesamiento.
  Al no disponer del código fuente, ha habido que hacer ingeniería inversa
  para averiguar el comportamiento exacto de algunas funciones. Además,
  su publicación habría aminorado el punto anterior, permitiendo a los
  alumnos con otros SO y/o arquitecturas recompilar la librería.

* Las horas de trabajo necesarias no se corresponden con las estimadas
  en la guía docente. Las prácticas de esta asignatura no encajan con la
  dinámica establecida previamente de una asignatura. Las competencias
  que se pretenden desarrollar abarcan mucho más allá del uso de sockets
  en C. Usando la métrica de lineas de código, la programación con sockets
  representa aproximadamente un 9% del total de código. El 91% restante
  lo compone el tratamiento de cadenas, manejo de hilos, interacción con
  la librería y el código relacionado con la interfaz gráfica. Además, esta
  metrica no incluye todo el trabajo de planificación, estudio de
  documentación, lectura de RFCS y elaboración de la documentación exigida.

  Alguna sugerencias son:
  + Emancipar las prácticas de la teoría, como se hace con ADSOFT, INGS...
  + Aumentar el peso de las prácticas sobre la nota final.
  + Reducir la carga de trabajo.
