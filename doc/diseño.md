
# Redes 2 - Servidor

## Introduccion
Una descripcion de lo que se pretende realizar en la practica


## Diseño

### Modulos del programa

### Estructuras de datos

Tenemos tres estructuras de datos:

* Canales  - representa un canal IRC
* Usuarios - encapsula una conexion con un cliente
* Servidor - engloba varios globales y usuarios

### Decisiones de diseño

servidor iterativo o concurrente
procesos o hilos, sincronizacion
estructuras de datos, etc.

### Organizacion del proyecto


## Funcionalidad IRC

a grandes rasgos, que funciones del protocolo se han implementado

## Conclusiones tecnicas
temas concretos de la asignatura que se han aprendido al realizar la practica

## Conclusiones personales
a que se ha dedicado mas esfuerzo, comentarios generales






---------------------------------------------------------------------------
# Queja de REDES2


* La no-publicación de unos criterios de corrección.
  No ha habido una manera clara de evaluar el progreso. A esto le
  sumamos la rigidez en la normativa de entrega, que enuncia:

  > El incumplimiento de cualquier punto de la normativa de entrega
  > llevará automáticamente a que la práctica no sea corregida y por
  > tanto tenga, automáticamente un 0.

  Estos dos hechos han sido una causa de ansiedad entre los alumnos.
  Además, la extensión del RFC también ha provocado que mas de uno
  se quejase de que "hay que implementar 10.000 comandos".

* La dificultades experimentadas al instalar los paquetes.
  La librería se ha distribuido empaquetada en un paquete `.deb`, por lo
  que fue ineludible la necesidad de tener acceso a un Debian. Aquellos
  estudiantes que no disponían de una maquina con este sistema operativo
  se han visto en desventaja. Transcurrió aproximadamente una semana hasta
  que se instalaron los paquetes en (unos pocos!) laboratorios y no
  siempre se cargaban correctamente al arrancar.

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
  la libreria y el código relacionado con la interfaz gráfica. Además, esta
  metrica no incluye todo el trabajo de planificación, estudio de
  documentación, lectura de RFCS y elaboración de la documentación exigida.

  Alguna sugerencias son:
  + Emancipar las prácticas de la teoría, como se hace con ADSOFT, INGS...
  + Aumentar el peso de las prácticas sobre la nota final.
  + Reducir la carga de trabajo.

* El profesor asignado al grupo 2301 no ha hecho una labor satisfactoria.
  Este profesor no estaba preparado para impartir las clases de prácticas.
  No fue capaz de responder las dudas preguntadas, ya que desconocía el
  temario y contenidos de la práctica.

* Obligar a leer los RFCs para entender los mecanismos del protocolo IRC.
  Para ello, es necesario un nivel alto de inglés, algo que no todos los
  estudiantes matriculados tienen.
