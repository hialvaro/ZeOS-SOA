---
description: Explicamos los threads y el sistema de ficheros
tags: soa, threads, file, system, virtual
---
# SOA: Segundo Parcial

## Índice

- Threads
    - ¿Qué modificaciones se deben hacer para incorporar threads?
    - Condiciones de carrera
- Tema 5: Sistema de Ficheros
  - Dispositivos
    - ¿Cómo se crean esas asociaciones entre dispositivos?
    - Llamada al sistema `read`
    - Aparece un problema
    - La solución: E/S Síncrona
    - E/S Asíncrona
- Sistemas de ficheros
    - ¿Cómo funciona?

## Threads

Por ahora hay un sistema operativo con procesos. Los procesos hacen peticiones al sistema operativo a través de las syscalls para pedir recursos. Los procesos son lo que realmente se ejecuta.

Los procesos son un poco limitantes. Los procesos pueden ser independientes, cada uno hace su tarea. Pero también puede darse el caso en el que queremos tener procesos coopeartivos, es decir, que el trabajo que genera un proceso los pueda consumir otro.

Con la estructura actual, si dos procesos quieren compartir información, no queda más remedio que pasar por el sistema operativo. Los procesos no comparten memória, comparten el sistema de ficheros. El sistema opeartivo tiene mecanismos como las pipes para pasar información de un proceso a otro. El problema de todos estos mecanismos es que van a través del sistema operativo (llamadas al sistema) lo que introduce mucho overhead a la ejecución de los procesos. Se empieza a ocupar demasiada CPU y se está demasiado tiempo en modo sistema para poder comunicar dos procesos.

Para solucionar este problema, se crean los **threads**. Los threads pertenecen a un proceso y comparten todos los recursos de este. Ahora ya no se ejecuta un proceso sino que se ejecutan los threads de este proceso. Y son los threads quien ejecutan las llamadas al sistema para pedir recursos, pero estos recursos no se asignan al thread, sino que se asignan al proceso al cual pertenece este thread. Un proceso es un **contenedor de recursos**. Los threads son **lo que realmente se ejecuta**.  El planificador ahora planifica **threads** de procesos en vez de procesos.

De esta forma el movimiento de un proceso a otro desaparece, como los threads pertenecen a un proceso, estos comparten los recursos; si dos threads dentro de un mismo proceso quieren enviarse datos lo podrán hacer a través de la propia memoria del proceso. Gracias a esto se puede explotar mucho más el paralelismo a nivel de tarea. 

Los threads no pueden moverse a otros procesos. Siempre pertenecen al proceso que los ha creado.

### ¿Qué modificaciones se deben hacer para incorporar threads?

**1. ¿Qué información necesita el thread para ejecutarse?**

La información propia del thread, en Linux, el proceso deja de ser real. El **task_union** que se usaba para guardar los datos del proceso pasa ahora a ser del thread. Dentro del **task_union** hay el **task_struct** que guarda la información del **thread** y su pila de sistema. El **task_union** identifica al thread y no al proceso. El proceso realmente no está en ninguna estructura, es una abstracción. 

El thread además necesita un identificador; un **thread identifier** (TID). En Linux, el TID es único en el sistema; no pueden haber dos threads con el mismo ID ya que el proceso realmente no existe y deben ser identificadores globales.

La pila de sistema también es un elemento necesario para el thread. Como ahora se ejecuta el **thread** cada thread debe tener su propia pila de sistema para poder hacer las llamadas al sistema. También una **pila de usuario** cada thread también ejecuta el modo usuario.

Cada thread también tiene su propio **contexto de ejecución** (CTX HW +   CTX SW + PCB).

Una variable **errno** propio para cada thread es necesario ya que cada thread ejecuta las llamadas al sistema.

Además los threads deben tener lo que llamamos TLS (Thread Local Storage), un trozo de memoria donde los threads guardan solamente variables que son interesantes para él. No es la pila, es dónde se guardan cosas como el **errno**. No son variables locales de función, sino variables propias del thread.

**2. ¿Cómo afecta al planificador?**

En Linux el planificador no se modifica. Los procesos no existen y ahora tan solo debemos planificar threads. Seguimos moviendo **task_union**.

En **Windows** se hace un planificador a 2 niveles: primero threads y luego procesos. Intentando minimizar los flushes de TLB. 

**3. ¿Cómo se crea un nuevo thread?**

En Linux se copia el task_union del thread creador al thread creado. No se necesita alocatar memoria por lo que la creación es muy rápida.

### Condiciones de carrera

Al trabajar con threads y a causa de la compartición de datos se producen casos de condiciones de carrera. Esto ocurre cuando dos threads estan trabajando sobre el valor de una misma variable.

Para solucionar este problema se implementan los **semáforos** que nos permiten ejecutar secciones de código con exclusión mutua. Eso se explicó ya en el resumen del primer parcial.

# Tema 5: Sistema de Ficheros

## Dispositivos

El sistema operativo, a través de una interfície genérica (open, read, write...) debe poder atacar a cualquier dispositivo.

En un sistema operativo hay un hardware, este hardware está compuesto (entre otras cosas) por dispositivos; tarjeta de red, tarjeta de vídeo, teclado, disco duro, etc. Los procesos, a través del sistema operativo, atacan estos dispositivos.

El sistema operativo ofrece una interfície genérica para acceder a estos dispositivos, un conjunto de llamadas al sistema. El programador no debe saber el código específico necesario para poder trabajar con cada dispositivo, es el sistema operativo quién a través de estas interfícies genéricas permite al programador trabajar con los dispositivos y luego este se ocupa de traducir la llamada al código necesario para el dispositivo.

Vamos a ver como el sistema puede traducir de una llamada genérica, por ejemplo *`read`* a un `read` específico para un fichero, una tecla de teclado o cualquier dispositivo. A groso modo, cualquier dispositivo que tengamos en la máquina va a ser tratado como un fichero dentro el sistema de ficheros; desde el punto de vista de usuario, **todo son ficheros**.

Lo primero que debemos hacer es **diferenciar** las clases de dispositivos. Habrá 3 clases de dispositivos:

1. **Dispositivos hardware**: El hardware de la máquina: tarjeta de red, teclado, ratón, etc. Cualquier cosa que podamos tocar con nuestras manos.
2. **Dispositivos lógicos**: Ficheros en el sistema de ficheros. Un directorio es un dispositivo lógico. Un fichero de datos, es un dispositivo lógico.

    El hardware se orfrece a través de ficheros lógicos; habrá ficheros relacionados con el teclado, con una tarjeta de red...
    
    Un dispositivo lógico puede valer para 3 cosas:
    
    1. **Abstracción de dispositivos hardware**. Se ofrece un dispositivo hardware como al usuario como un fichero (/dev/kbd01 = teclado).
    2. **Agregar dispositivos hardware**. Se puede tener un dispositivo lógico, por ejemplo uno llamado *terminal*, que agrupe dispositivos hardware; agrupa una tarjeta de red, una pantalla, un telcado y un ratón. Cuando nos referimos a un dispositivo lógico como `/dev/tty` nos estamos refiriendo a un terminal que en realidad es un **conjunto de dispositivos hardware**.
    3. **Añadir nuevas características al sistema operativo**. Por ejemplo `/dev/null` es un dispositivo lógico que no está asignado a ningún dispositivo físico. Todo lo que vaya a `/dev/null` no se guardará en memória sino que desaparece del sistema. Si se lee de este disposivio, siempre estará vacío.
 
3. **Dispositivos virtuales** (Canales o File descriptors): son instancias de dispositivos lógicos. Un programador cuando quiere trabajar con un dispositivo lógico lo primero que hace es **abrir** ese fichero. El sistema operativo entonces le devuelve un identificador de ese dispositivo lógico.

    Cuando ya estamos trabajando con un fichero, realmente trabajamos con un dispositivo virtual asociado a un dispositivo lógico. Si, por ejemplo, trabajamos con la tarjeta de vídeo estaremos trabajando con un dispositivo virtual asociado a un dispositivo lógico asociado a un dispositivo lógico.
    
En el proceso de boot, antes de saltar al modo usuario en el proceso `init`, se hace un reconocimiento del hardware de la máquina. El sistema operativo, a través de la **bios**, empieza a escanear (recoger información) el hardware que hay en la máquina. No solo la información sobre ese dispositivo físico, sino también se guarda la información sobre **dónde** está conectado ese dispositivo (p.ej. un disco duro se guarda si está en el conector SATA0, SATA1...).

Los sistemas operativos suelen tener una carpeta de *drivers*, o el propio hardware lleva un CD con el driver a instalar. Durante ese proceso de reconocimiento el  SO también mira si existe algún driver capaz de controlar ese dispositivo. Para esto, una de las informaciones necesarias es el **PID** (Product Identifier); un número que identifica ese dispositivo unívocamente de forma universal.

Con este PID el sistema operativo itera por todos los drivers que tiene; un driver, tiene una estructura formada por un ***header*** y una parte de **código**. En el header es dónde aparecen, entre otros elementos, los **PIDs** de los productos hardware con los cuales sabe trabajar este driver.

Una vez se ha encontrado con un controlador compatible, alocata en memoria una estructura que se llama ***device descriptor*** que tiene un *header* dónde copia el **PID** del producto, el **nombre** del driver... Y otro campo importante que también se encuentra en el *header* del *driver* es el llamado ***major***; un número único que se le da al driver y que se refiere a la **família de productos** que pueden trabajar con este driver (driver de teclados, de tarjetas de vídeo, etc...).

Además como sistema operativo, sabe dónde está conectado ese dispositivo. En otro campo llamado ***minor*** se guarda **dónde** está conectado este dispositivo (en qué puerto físico). A través del *major* y el *minor* podemos saber qué tipo de dispositivo es y dónde esta conectado. De este modo podemos hablar de *"teclado en USB0"* por ejemplo.

Por último cuando el sistema operativo ha alocatado el **device descriptor** y ha guardado en su header los datos mencionados, el sistema empieza a cargar el **código** que se encuentra dentro del driver en memoria. A medida que hace esto, en el **device descriptor** se van guardando punteros a las funciones de código que tiene el controlador, entre las cuales (por obligación del sistema operativo) deben existir opearciones para (abrir, cerrar, leer, escribir...).

Los device descriptor son iguales para todos los drivers. Por lo tanto sabemos que en cierta posición, por ejemplo dónde hay el puntero a la función `open`, va a ser igual para todos los device descriptors del sistema, independientemente del hardware al que haga referencia. Dentro del device descirptor los punteros a las distintas funciones siempre se encuentran en la misma posición.

Una vez ha creado el **device descriptor** para el primer dispositivo hardware, hace lo mismo para el siguiente. Al final lo que tendremos es un vector de device descriptors.

> El **device descriptor** nos informa sobre qué controlador debemos utilizar para un hardware en concreto.

### ¿Cómo se crean esas asociaciones entre dispositivos?

#### Disp hardware <> Disp lógico

Se usa una llamada al sistema llamada `mknod`. Crea una asociación entre un dispositivo físico (hw) y un dispositivo lógico.

##### ¿Qué hace `mknod`?

Esta llamada tiene un primer parámetro que puede ser `b` o `c` indicando cómo hace la transferencia de datos ese dispositivo: si es un dispositivo de `chars` o de `blocks`. Un teclado es un dispositivo de carácteres, porque se envían una a una las teclas.

El segundo parámetro es el **major** del **device descriptor** que quiere utilizar. El tercer parámetro es el **minor** hace referencia también al minor del **device descriptor**. Por último se debe indicar el **nombre del dispositivo lógico**.

Ahora `mknod` crea  un **dispositivo lógico** (fichero) cuyo **inodo** tiene el **major** y el **minor** que se le ha dado al comando. 

> `mknod` crea un dispositivo lógico asociado a través de un major y un minor a un **device descriptor**.

#### Disp lógico <> Disp virtual

Se usa la llamada al sistema llamada `open`. Open no abre un fichero, sino que crea una asociación entre un dispositivo lógico y un dispositivo virtual. Crea una instancia de un dispositivo lógico.

`int open(char *name, int mode [,int flags])`

- `name` - Nombre es la ruta hasta el dispositivo lógico que queremos abrir.
- `mode` - Modo de apertura: leer, escribir...
- `flags` - Parámetro opcional. Normalmente dependen del modo; pueden ser permisos, etc.

##### ¿Qué hace `open`?

Esta función va a coger el nombre que le llega como primer parámetro (ruta al dispositivo) y empieza a travesar todo el grafo de directorios para llegar hasta el inodo que el dispositivo lógico tiene asociado.

Si no existe da error. Pero si lo encuentra, lee el inodo y lo mete en memoria. A continuación trabajará con el **major** y el **minor** del inodo para ir a buscar a la lista de **device descriptors** con el mismo *major* y *minor* para comprobar si hay algún driver para poder trabajar con este fichero.

La **tabla de inodos** es una estructura única en el sistema operativo. Dentro de cada entrada, entre otros campos, tiene el **inodo**, el puntero al *device descriptor* y el **número de referencias** que indica cuantas entradas de la **TFA** hacen referencia a ese inodo; también será el critero para vaciar o no de la tabla de inodos una entrada a disco.

Una vez ha comprobado que existe el driver, `open` accederá a la **tabla de inodos** y guardará en una entrada el **inodo** y el puntero al *device descriptor* asociada.

Esta tabla de inodos realmente no es necesaria en un **sistema operativo**. Es una optimización para evitar atacar demasiado al disco. Mientras se trabaja con un dispositivo virtual, se van a tener que hacer muchos accesos al inodo (p.e. si cambiamos el tamaño del fichero, la fecha de modificación... son campos que se encuentran dentro el inodo). Con esta tabla en memória se evita tener que ir cada vez al disco a consultar el inodo; de vez en cuando se volcarán estos inodos modificados al disco.

Esta tabla de inodos tambié sirve a modo de caché, ya que se comprueba primero si el inodo existe en la tabla antes de ir al disco.

**Siempre** (siempre!) que se hace un `open`, una vez guardado el inodo y el *device descriptor* dentro de la tabla de inodos, se coge una nueva entrada dentro de la **tabla de ficheros abiertos** (TFA, única en el sistema). Esta tabla tiene muchos campos, pero nos centraremos en dos: el **puntero de lectura y escritura**, el **puntero a la tabla de inodos** y el **número de referencias**.

El **puntero de lectura y escritura** indica el offset desde el inicio del fichero se efectuará la siguiente instrucción de lectura o escritura, open siempre inicializa este valor a *cero*. El **puntero a la tabla de inodos** es un puntero que apunta a la tabla de inodos dónde está el inodo con el que se va a trabajar.

El número de referencias de la TFA indica cuantos canales en el sistema operativo apuntan a esta entrada dentro de la TFA.

Dentro del PCB del proceso, hay un puntero a lo que se conoce como **tabla de canales** del proceso (TC, única por proceso). Ahora `open` buscará una entrada vacía en la TC. Esta TC tiene, entre otros, un puntero a la entrada dentro de la TFA a la que hace referencia y añade una referencia a la TFA.

Por último, devuelve el número de entrada dentro de la **tabla de canales**. El `int` que devuelve la llamada `open`, que conocemos como **file descriptor** o **canal**.

###### **`dup`, `dup2` y `fork`:**

`dup(int fd)` duplica el contenido de el canal que se le pasa por parámetro en la primera entrada que esté libre y `dup2(int fd_dest, int fd_src) ` duplica el contenido origen al canal destino. Sirven para management de la TC.

Si hacemos un `dup` de un canal, tendremos dos canales referenciando a la **misma entrada* de la **TFA**. Por lo tanto el **número de referencias** a esa entrada de la *TFA* va a incrementar en 1.

Con el `fork`, se duplica la tabla de canales del proceso. Eso significa que también va a incrementar el número de referencias a la entrada de la TFA.

#### Cosas a tener en cuenta

Esta estructura de tablas que hemos visto, a través de un número (`fd` dentro de la tabla de canales), se consigue llegar hasta el driver que acabará atacando al dispositivo.

Por un lado, la relación entre device descriptor e inodo nos crea la relación entre dispositivo físico y dispositivo lógico y por otro lado, las tres tablas vistas nos crean la relación entre dispositivo virtual y dispositivo lógico; crean una instancia de dispositivo lógico.

Así podemos decir que la **TC** y la **TFA** contienen la **información dinámica** del dispositivo lógico; es decir: el **dispositivo virtual**. Por otro lado, la tabla de inodos es la **información estática** compuesta por el inodo y el device descriptor; una información que no suele cambiar.

La conexión entre la TFA y la tabla de inodos, proporciona un **acceso concurrente** al dispositivo lógico. Podemos tener un proceso escribiendo en el byte 0 y otro escribiendo en el byte 1290, por ejemplo. Esto es gracias al puntero L/E de la entrada en la TFA.

La relación entre la TC y la TFA nos permite el **acceso compartido** a un dispositivo lógico. Como tenemos varios canales apuntando a la misma entrada de la TFA, el puntero L/E se está compartiendo. La operación que se haga desde un canal, se verá reflejada en otro canal. Si desde un canal escribimos 10 bytes, se modificará el offset del puntero L/E y la verán los otros canales que apunten a esa misma entrada de la TFA. Podemos tener más de un canal operando exactamente en el mismo punto del dispositvo lógico.

Gracias a estas dos características, podemos tener accesos concurrentes y compartidos a la vez. Es decir, puedo estar trabajando en diferentes partes del dispositivo lógico y, a la vez, puedo tener varios canales dentro de TCs trabajando con el mismo punto del dispositivo lógico. Tenemos relaciones **

### Llamada al sistema `read`

`int read(int fd, void *buffer, size_t length)`

- `fd` - File descriptor / número de canal / dispositivo virtual.
- `*buffer` - Dónde quiero que se dejen los datos.
- `length` - Tamaño de los datos.

Sabemos que `read` en realidad es un wrapper. Dentro del wrapper, se hace la llamada al sistema y se salta a modo sistema. Lo primero que nos encontramos es el handler de la llamada al sistema y al final, este handler acaba llamando a la función `sys_read`.

El proceso tiene en el PCB un puntero a la TC. Con el parámetro del `fd` accederemos a la entrada correspondiente de la TC; dentro de esta entrada, entre otros, hay un campo que contiene el puntero a la entrada correspondiente de la TFA. Dentro de la TFA se encuentra el **puntero L/E** que nos dice a partir de qué punto se debe empezar a leer esa cantidad de bytes.

Lo siguiente que hace es coger el puntero (de la TFA) a la entrada dentro de la **tabla de inodos**. El read necesita acceder a esta entrada porque:

1. Contiene el inodo, que contiene muchos datos sobre el dispositivo lógico; por ejemplo la cantidad de bytes para comprobar si el tamaño de la lectura que pide el usuario es correcto o si estamos al final del fichero.
2. Contiene el puntero al **device descriptor** dónde hay los punteros a las funciones del driver de ese dispositivo.

Ahora, a través del puntero al *device descriptor* que se encuentra en la tabla de inodos, puede buscar dentro del *device descriptor* la función `read` del driver (que siempre está en una misma posición determinada). 

Ahora que ya se ha conseguido el puntero al `read` del driver, se hace una llamada a la función que apunta; una función que estará implementada dentro del driver y que tiene un códgio totalmente dependiente al dispositivo.

Esto será exactamente igual para cualquier dispositivo lógico y cualquier dispositivo físico ya que, hasta que se llega al **device descriptor** (incluido el acceso a este), todo es código independiente al dispositivo lógico que se esté atacando. Como programadores del sistema operativo, solo se debe implementar hasta el acceso al *device descriptor*; el resto lo hará cada fabricante.

De este modo, el sistema operativo es totalmente independiente al dispositivo físico que se conecta. Es el fabricante quien se debe preocupar para ofrecer los drivers.

![1.png](Teoria/img/1.png)

### Aparece un problema

> Hablamos de **read** como ejemplo, pero aplica a todas las funciones.

En el momento de ejecutar el `read` dependiente estamos en **modo sistema** (todo el proceso de accesos a tablas para llegar allí ocurre dentro del `sys_read`); el acceso puede ser una operación de larga latencia (acceder a un HDD, por ejemplo) dejándolo todo congelado.

Esto supone un problema; deberíamos poder realizar operaciones de larga latencia sin que se penalize el rendimiento de todo el sistema. El problema concretamente reside en que (con el esquema descrito) es el propio proceso, a través del driver quién hace la entrada/salida al dispositivo físico.

### La solución: E/S Síncrona

Debemos modificar el `sys_read` de modo que vamos a solapar la entrada/salida de un proceso con la ejecución de otro proceso.

Recordamos el ciclo de vida de un proceso. Los procesos se crean en estado `ready`, luego se pasan a ejecutar en `run` y cuando están haciendo operaciones de larga latencia pasan al estado `blocked`, a continuación un proceso en estado `ready` se pasará a `run`. Una vez la operación de larga latencia haya finalizado, se pasará a `ready` de nuevo. 

![2.png](Teoria/img/2.png)

El estado de `blocked` es necesario para poder aprovechar los tiempos de larga latencia para ejecutar otros procesos.

Para hacer esto se debe modificar el esquema que hemos visto hasta ahora. Los procesos a partir de este momento, no van a hacer entrada/salida sino que van a hacer peticiones de entrada/salida. Es decir, un proceso no realizará en modo sistema la entrada/salida sino que cuando lo necesite hacer va a lanzar una petición de entrada/salida y se van a bloquear esperando.

Para ello, se crea un nuevo proceso de sistema llamado **gestor** que será quien realmente acabará realizando la entrada/salida. Estos gestores se crearán en tiempo de boot.

Para esto, el puntero al *device_descriptor* dentro de la tabla de inodos no será el *device_descriptor* que ataque directamente al driver, sinó que habrá un segundo *device_descriptor* con los punteros a las **funciones dependientes del gestor**. Antes se atacaba a través del *device_descriptor* directamente al driver del dispositivo físico. Ahora, a través de un *device descriptor* secundario se atacará a funciones dependientes de un proceso llamado **gestor** el cual acabará atacando a las funciones del driver del dispositivo físico.

![3.png](Teoria/img/3.png)

De este modo, cuando un proceso haga una entrada/salida el proceso no ocupará la CPU, sino que el **gestor** a medida que pueda realizará las entradas/salidas a través del driver mientras el proceso está bloqueado. La latencia se la queda el gestor y de este modo otros procesos se pueden ir ejecutando.

Este **gestor** básicamente va a ser un bucle infinito que irá aceptando y procesando peticiones. Como solo hay un gestor para el dispositivo, y podré tener más de un dispositivo atacandolo a la vez, será necesario tener una cola de peticiones, la **cola de iorbs (`q_iorbs`)**; o una cola de ***input/output request blocks***. 

> Puede haber como mínimo uno por dispositivo físico o un gestor que ataque a varios dispositivos físicos. Lo importante es que las llamadas al sistema ya no ataquen directamente a la función dependiente, sino que ataquen al gestor.

El gestor irá leyendo peticiones de la cola, hará las operaciones al driver y cuando acabe irá devolviendo los resultados. Estos resultados serán devueltos en una segunda cola: **`q_iofin`** (*input/output finished*).

Con esto `sys_read` hará los accesos a todas las tablas que hemos visto antes del mismo modo; cuando llegue al punto de coger el puntero al *device descriptor* ahora lo que tendrá será un puntero para hacer peticiones al **gestor**. El `sys_read` hará un `read_dependiente` que encolará una petición al **gestor** en la `q_iorb` y se esperará al resultado en la `q_iofin`.

![4.png](Teoria/img/4.png)

Una vez la petición ha sido encolada a la cola de **iorbs**, el proceso será sacado del estado `run` para que otro proceso pueda pasar a ejecutarse.

Un **iorb** tiene muchos campos, entre ellos hay un identificador de operación que indica si es un read, un write; también hay un puntero a un buffer para dejar o escribir datos; y un **semáforo** de la operación (`sem_op`) inicializado a cero.

Para cada petición en la cola de iorbs, habrá un semáforo. La función del `read_dep` de `encolar_pet()` va a devolver un identificador de petición (que podría corresponder con la posición dónde se encuentra dentro de `q_iorbs`). Una vez encolada la petición, se va a hacer un `sem_wait` sobre el semáforo de la operación para bloquear el proceso hasta que llegue el resultado de la operación. A continuación se llama al planificador para que ejecute otro proceso. Por lo tanto, es este `sem_wait` lo que realmente implementa el estado de **blocked** del que hablamos en este caso.

El gestor, una vez ha encolado el resultado en la `q_iofin`, hará un sem_post al semaforo de la operación para desbloquear a ese proceso y que pase a **ready**.

Cuando se hace una operación de E/S tenemos dos opciones: hacer una **espera activa** (preguntando constantemente al dispositivo si ha acabado) o a través de interrupciones. Los dispositivos actuales funcionan por **interrupciones**. El `do I/O` que vemos en el diagrama anterior trabaja con el driver, el driver está implementado de forma que hace una petición al dispositivo físico y cuando este acabe enviará una **interrupción** para poder recibir el resultado. El gestor no se queda esperando, sigue con su trabajo hasta que le llega una interrupción.

El gestor simplemente gestiona peticiones de modo que el gestor no ocupa la CPU. Gracias a que los dispositivos funcionan por interrupciones y que podemos bloquear y desbloquear procesos a través de semáforos. Esto abre la posibilidad de hacer un cambio de contexto dentro del `read_dep` y conseguimos solapar la ejecución de procesos con E/S en modo sistema.

##### ¿Qué pasa con el gestor cuando no hay peticiones?

En casos de dispositivos físicos que se usan muy poco, puede darse el caso que el gestor no tenga peticiones durante largos periodos de tiempo y la `q_iorbs` estará vacía.

La función del gestor `coger_pet()` (ver diagrama anterior), no va a estar constantemente consultando si hay nuevas peticiones. No tiene sentido hacer todo esta implementación para evitar ocupar la CPU con la E/S en modo sistema y ocuparla para revisar si hay nuevas peticiones.

En el caso de que no haya peticiones el gestor estará **bloqueado** y se desbloqueará cuando, como mínimo, haya una petición. Para conseguir esto, se añade un **semáforo** (`sem_gest`) para cada uno de los gestores inicializado a cero. El gestor, al empezar a ejecutarse (y antes de `coger_pet()`), hará un `sem_wait(sem_gest)`; este `sem_wait` será bloqueante hasta que como mínimo haya una petición. Sabremos que como mínimo hay una petición cuando el `read_dep()` encole una petición en la `q_iorbs`. Así que vamos a tener que hacer un `sem_signal` (o `sem_post` es lo mismo) cuando se haya encolado una petición para desbloquear el gestor. Así tendremos un semaforo por cada petición.

![5.png](Teoria/img/5.png)

Por último debemos tener en cuenta que las dos colas (`q_iorbs` y `q_iofin`) deben ser accedidas en exclusión mutua. El gestor está sacando **iorbs** de la `q_iorbs` y metiendo resultados en la cola `q_iofin` mientras que puede haber otros threads de usuario que pueden estar haciendo peticiones y cogiendo resultados de estas mismas colas.

Así que el gestor debe tener también un semáforo para cada cola (`sem_iorbs`, `sem_iofin`) para asegurar que las operaciones a las colas se hacen en exclusión mutua. Como son de exclusión mutua, se deben inicializar a 1. En resumen:

- 3 semáforos en el gestor
    - bloqueo del gestor 
    - exclusión mutua de colas (x2)
- 1 semáforo por petición en `q_iorbs`

A este tipo de entrada/salida con gestores, se le llama **I/O síncrona** porque al hacer la petición se bloquea el thread hasta recibir los datos.

### E/S Asíncrona

Al programar un proceso, el programador sabiendo que la e/s va a tardar bastante, podría aprovechar para hacer otras cosas de forma asíncrona a esa e/s hasta que esté lista. La idea es **bloquearse solo cuando se necesiten realmente** los datos de la e/s.

Por ejemplo, al trabajar con matrices de dimensiones muy grandes puede interesar ir trabajando con los trozos de matriz a medida que se van cargando el resto de trozos. Se solapa el cálculo con la e/s.

Para conseguir esta e/s asíncrona se debe realizar un pequeño cambio. Lo primero es tener una **llamada al sistema** de petición de datos asíncronos. Debe haber, por ejemplo, un `read` síncrono y otro asíncrono.

El `async_read` es exactamente igual que el `read` pero la única diferencia es que **pide los datos que quiere traer** sin bloquearse. Después del `async_read` el programa va a seguir ejecutándose y en algún punto va a comprobar si los datos que había pedido ya han llegado para poder seguir.

Para eso, se necesita una segunda **llamada al sistema**, `data_available`, que diga si los datos que había pedido el `async_read` están o no están. Solo en caso de que no estén, es cuando el proceso se bloquea.

Desde el punto de vista de usuario, la petición se divide en dos trozos: petición de datos con un read asíncrono y comprobación de datos para trabajar.

El `async_read` va a llamar a `sys_async_read`, este hará prácticamente el mismo recorrido que el `read` (PCB, TC, TFA, T_inodos...) hasta llamar al `async_read_dep` el cual prepara la petición para el gestor; la encola y desbloquea (o no) al gestor. 

En el código de usuario, en algún punto, se ejecuta el `data_available` que hará también el recorrido de PCB, TC, TFA, T_inodos... que ejecutará el `data_available_dep` dónde este sí se va a bloquear en caso de no tener los datos y en caso de tenerlos debe devolverlos. Simplemente se añade una segunda llamada al sistema que pide los resultados y, en caso de que no estén se bloquea. 

> **En resumen**: El `async_read` lanza una petición de unos datos sin esperar el resultado y sin bloquearse. Cuando el usuario necesita los datos usa una segunda llamada al sistema `data_available` que se encargará de ir a buscar los datos y, si no estan, bloqueará el proceso.

Los sistemas operativos actuales solo implementan la e/s asíncrona. La e/s síncrona es emulada; el `read_dep` llama al `async_read_dep` y luego immediatamente llama a `data_available`.

## Sistemas de ficheros

Los sistemas operativos implementan lo que se llama **virtual filesystem**; un sistema de ficheros que realmente no está implementado en ningún medio sino que es una abstracción genérica de cualquier clase de sistema de ficheros. Los ficheros y directorios no existen de forma "física", no están implementados en el disco. Si exploramos distintos discos como usuarios, los veremos todos iguales; no diferenciamos entre un disco con un sistema de ficheros NTFS, ext4...

El **virtual filesystem** unifica las características de cualquier sistema de ficheros específico y las ofrece al usuario como si fueran un único sistema de ficheros.

### ¿Cómo funciona?

El sistema operativo ofrece a los procesos interfícies genéricas (**llamadas al sistema**) de acceso al sistema de ficheros (leer, escribir) como ya hemos visto antes: TC -> TFA -> T_inodos.

En un **virtual filesystem (VFS)**, la **tabla de inodos** no hace referencia a los inodos de un sistema de ficheros específico sino a **inodos virtuales**, inodos del VFS.

El VFS se encarga de relacionar estos inodos virtuales con algo parecido a un inodo que pueda tener cualquier sistema de ficheros específico.

Si se quiere que la estructura de directorios de un sistema pueda contener sistemas de ficheros distintos, se debe tener un **punto de montaje** indicando que a partir de cierto punto del árbol de directios ya no se está haciendo referencia a este disco sino que se referencia a otra partición en este u otro disco que puede tener un sistema de ficheros específico distinto al del propio disco.

Lo que se hace es que dentro del grafo de directorios hay subdirectorios que **hacen referencia a sistemas de ficheros**. Para un VFS, un sistema de ficheros es un dispositivo lógico más que hace referencia a un **driver de puntos de montaje**.

Si, por ejemplo, C:\\\ es un putno de montaje lo que veremos es va a tener un **inodo** con un **major** de tipo ***mountpoint*** y un **minor** que indica qué punto de montaje es. Con el **major** podrá ir al *device descriptor* de un driver específico que gestiona puntos de montaje (*map_manager* en Linux). a través de estos majors, el **map_manager** es capaz de decirle al SO que debe utilizar otro sistema de ficheros. El **minor** es un dato específico del driver que indica, dentro de la tabla de puntos de montaje que tiene el driver, a cuál nos referimos.

Un **punto de montaje** es un conjunto de datos que indica de qué dispositivo se está refeririendo, qué sistema de ficheros tiene y, por lo tanto, qué *device descriptor* debo utilizar para trabajar con el dispositivo.

Entonces, lo que hace el VFS es ir recorriendo el árbol de directorios mirando el major y el minor de los inodos y en el caso de encontrar un major que sea un punto de montaje habla con el driver de punto de montaje para saber cómo debe seguir trabajando a partir de ese punto.

Las syscalls, gracias a los puntos de montaje, van a acabar usando **drivers específicos** para los sistemas de ficheros específicos (ext2, ntfs, fat...). La comptaibilidad del sistema de ficheros del sistema con los distintos sistemas de ficheros específicos se da gracias a estos **puntos de montaje** y a los **drivers específicos** para gestionar estos **SFS** (Specific Filesystem - palabra inventada).

La raíz del sistema de ficheros también es un punto de montaje que indica, para el sistema de ficheros base que se utiliza, qué **SFS** se debe utilizar. Todo funciona igual, pero la diferencia es que vamos a tener funciones dependientes del **SFS**.

El VFS implementa una capa intermedia de optimización que es una **buffer cache**, una caché de trozos de disco. No se accede directamente al dispositivo físico, sino que se utiliza una caché intermedia. Suelen ser pequeñas, de unos 8MB. Si hay un **miss**, se deberá acceder a los dispositivos físicos a través de los drivers.

El VFS alocata inodos virtuales, una estructura propia del VFS. Dentro del inodo virtual, se guarda un puntero a la estructura específica del sistema de ficheros específico. El VFS trabajará con inodos virtuales pero cuando deba pasar la información al SFS le pasará el puntero que tiene dentro del inodo a los datos específicos del sistema de ficheros específico. De este modo se establece la relación entre el VFS y el SFS.

> "Me das unos datos que yo no entiendo. Yo me guardo una referencia a estos datos. Y cuando yo, como VFS, esté hablando de este dispositivo lógico pasaré el puntero y el SFS se busca la vida."
> > Pajuelo.

![6.png](Teoria/img/6.png)

La unidad mínima de transferencia de datos de un sistema operativo hacia un dispositivo es el **bloque de datos**. El sistema no sabe lo que son sectores. Dentro de los inodos también tenemos **bloques de datos**.

Los drivers utilizaran una interfície genérica para acceder a la buffer cache y la unidad mínima de transferencia siempre va a ser el **bloque de datos**. Los drivers se comunican con la buffer cache con la traducción de dispositivo lógico a bloque de datos hecha.

Para ello, los drivers tienen los datos del SFS (bloques de datos que tiene).

Por otro lado, la **buffer cache** si en algún momento hay un **miss** va tener que comunicarse con un dispositivo físico (el cual solo entiende sectores de disco). Así que dentro de la buffer cache, con la información que le llega del **SFS**, se hace la traducción de bloque de datos a sector para poder pedir sectores al *device descriptor* específico.

**Con esta estructura, ¿dónde deben ir gestores?**

> Recordatorio: Los gestores se usan para solapar la e/s (larga latencia) con la ejecución de otros procesos.

La **buffer cache** tiene un gestor entre ella y el acceso al dispositivo físico. Por el contrario, se paga la latencia en las operaciones de la buffer cache cuando hay un miss.

Como la buffer cache puede recibir peticiones secuencialmente (en paralelo no, porque estamos en modo sistema y solo hay un proceso) mientras aún no ha terminado una opearción de larga latencia, será necesario tener un gestor para administrar estas **peticiones pendientes**.
