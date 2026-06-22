UNIVERSIDAD DE SANTIAGO DE
COMPOSTELA

ESCUELA T´ECNICA SUPERIOR DE INGENIER´IA

Optimizaci´on en el acceso a nubes de
puntos de 3D mediante la utilizaci´on de
octrees y coordenadas polares

Autor/a:
Manuel Ramallo Blanco

Tutores:
Francisco M. Fern´andez Rivera
Miguel Yermo Garc´ıa

Grado en Ingenier´ıa Inform´atica

Junio 2026

Trabajo de Fin de Grado presentado en la Escuela T´ecnica Superior de
Ingenier´ıa de la Universidad de Santiago de Compostela para la obtenci´on del
Grado en Ingenier´ıa Inform´atica

Agradecementos

Se se quere p´or alg´un agradecemento, este vai aqu´ı.

3

4

Resumen

En este trabajo se describe y se presenta una soluci´on de mejora de eficiencia
temporal en el acceso a nubes de puntos 3D utilizando como estructura de da-
tos octrees lineales. El estudio consiste en el reordenamiento espacial de puntos
para tratar de explotar la capacidad de poda a la hora de encontrar vecindades.
Se eval´uan resultados temporales usando diferentes t´ecnicas de reordenamien-
to, operando tanto con las coordenadas cartesianas como con las coordenadas
polares de los puntos. Los resultados experimentales muestran un impacto po-
sitivo en la eficiencia temporal bajo determinadas circunstancias, a cambio de
un gasto en memoria utilizada.

5

6

Indice

1. Introducci´on

2. Estado del arte

1

5

3. Materiales

9
3.1. Equipos de c´omputo y entorno de desarrollo . . . . . . . . . . . .
9
3.2. Marco te´orico matem´atico . . . . . . . . . . . . . . . . . . . . . . 10
3.2.1. Coordenadas tridimensionales . . . . . . . . . . . . . . . . 10
3.2.2. Curvas de llenado de espacio . . . . . . . . . . . . . . . . 12
3.3. Algoritmos y estructuras de datos de partida . . . . . . . . . . . 13
3.3.1. Octree lineal
. . . . . . . . . . . . . . . . . . . . . . . . . 13
3.3.2. Algoritmos de b´usqueda . . . . . . . . . . . . . . . . . . . 14
3.4. Conjuntos de datos empleados . . . . . . . . . . . . . . . . . . . . 16

4. Metodolog´ıa

19
. . . . . . . 19

4.1. Fundamentos geom´etricos de la poda en nodos hoja
4.2. Primera Fase de Implementaci´on: Selectores de rango polares y

vectores de permutaciones . . . . . . . . . . . . . . . . . . . . . . 23
4.3. An´alisis de Cuellos de Botella y Redise˜no del Sistema . . . . . . 24
4.4. Segunda Fase de Implementaci´on: Refactorizaci´on y Simplifica-

ci´on de Modelos . . . . . . . . . . . . . . . . . . . . . . . . . . . . 26

4.5. Construcci´on de r´eplicas contiguas e integraci´on en el flujo de

b´usqueda del Octree . . . . . . . . . . . . . . . . . . . . . . . . . 27

5. Pruebas

31
5.1. Plan de Verificaci´on y Correcci´on Global . . . . . . . . . . . . . . 31
5.2. Rendimiento de b´usquedas de vecinos paralelas con centros alea-

torios

. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 32
5.3. Rendimiento de b´usquedas de vecinos paralelas sobre toda la nube 37
5.4. Eficiencia del paralelismo sobre b´usquedas de vecinos . . . . . . . 38

6. Discusi´on de los Resultados

45

6.1. An´alisis de Estructuras de Almacenamiento: algoritmo neighborsStruct

frente a neighborsPrune . . . . . . . . . . . . . . . . . . . . . . 45
. . . . . 46
6.2. Sensibilidad al Tama˜no de Hoja y Din´amicas del Kernel
6.3. Evaluaci´on de los Modos de Reordenamiento Local
. . . . . . . . 47
6.4. Calibraci´on del Espacio de Hiperpar´ametros . . . . . . . . . . . . 47

7

6.5. Din´amicas en B´usquedas de Cobertura Completa . . . . . . . . . 48

7. Conclusi´ons e posibles ampliaci´ons

49
7.1. Principales aportaciones y limitaciones . . . . . . . . . . . . . . . 49
7.2. V´ıas de Mejora y Trabajo Futuro . . . . . . . . . . . . . . . . . . 50

A. Manuais t´ecnicos

B. Manuais de usuario

Bibliograf´ıa

51

53

55

8

´Indice de figuras

4.1. Funcionamiento de la poda en un espacio tridimensional usando

ejes cartesianos. . . . . . . . . . . . . . . . . . . . . . . . . . . . . 20

4.2. Funcionamiento de la poda en un espacio tridimensional usando

coordenadas esf´ericas/cil´ındricas.

. . . . . . . . . . . . . . . . . . 23

4.3. Distribuci´on de claves ´optimas por hoja en modo esf´erico sobre

la nube sg27 station 8 . . . . . . . . . . . . . . . . . . . . . . . . 25

4.4. Distribuci´on de claves ´optimas por hoja en modo cil´ındrico sobre

la nube sg27 station 8 . . . . . . . . . . . . . . . . . . . . . . . . 26
4.5. B´usqueda base (Acceso secuencial). . . . . . . . . . . . . . . . . . 28
4.6. B´usqueda reordenada (Acceso indexado).
. . . . . . . . . . . . . 28
4.7. B´usqueda base para el algoritmo neighborsStruct. . . . . . . . . . 29

5.1. Efectividad te´orica de cada modo de poda sobre el dataset Li-
lle 0, sobre kernels c´ubicos y esf´ericos de diferentes tama˜nos,
usando un tama˜no m´aximo de hoja de 128 puntos.

. . . . . . . . 33

5.2. Tiempos medios de b´usqueda de vecindades de 10.000 puntos

aleatorios tomando diferentes radios en 5085 54320 . . . . . . . . 34

5.3. Tiempos medios de b´usqueda de vecindades de 10.000 puntos
aleatorios tomando diferentes radios en bildstein station1
5.4. An´alisis del rendimiento temporal de hiperpar´ametros en las nu-

. . . . 35

bes de puntos Mar18Train y Mar18Test

. . . . . . . . . . . . . . 36

5.5. Tiempos medios de b´usqueda de vecindades de 10.000 puntos

aleatorios para diferentes radios en la nube Mar18 Train . . . . . 37

5.6. Tiempos medios de b´usqueda de vecindades de 10.000 puntos

aleatorios para diferentes radios en la nube Mar18 Test

. . . . . 37

5.7. Tiempos de ejecuci´on de b´usquedas completas variando el radio

operativo sobre kernels esf´ericos y c´ubicos. . . . . . . . . . . . . . 40

5.8. Mapas de calor de eficiencia de paralelizaci´on sobre centros alea-

torios en la nube de puntos Paris Luxembourg 6 . . . . . . . . . 42

5.9. Mapas de calor de eficiencia de paralelizaci´on sobre coberturas

completas en la nube de puntos Lille 0

. . . . . . . . . . . . . . 44

9

10

´Indice de cuadros

3.1. Especificaciones t´ecnicas del entorno de ejecuciones [5]. . . . . . .
3.2. Nubes de puntos utilizadas en las evaluaciones experimentales.

9
. 17

4.1. Porcentaje de puntos podados seg´un el tipo de reordenamien-
to, geometr´ıa del kernel y radio de b´usqueda (r) para las nubes
analizadas.

. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 25

11

12

Cap´ıtulo 1

Introducci´on

Las nubes de puntos 3D son una representaci´on de una realidad, como un
objeto o un terreno, mediante un conjunto de puntos en el espacio tridimensio-
nal. Cada uno de esos puntos est´a caracterizado por sus coordenadas espaciales,
as´ı como otras propiedades como el color o la intensidad. Entre las aplicaciones
m´as importantes de la elaboraci´on de estas nubes est´an la cartograf´ıa, la elabo-
raci´on de modelos tridimensionales en CAD de piezas fabricadas, la metrolog´ıa,
la arqueolog´ıa o la conducci´on aut´onoma. La tecnolog´ıa predominante para la
captura de estos puntos es la LiDAR (Light Detection And Ranging).

Una de las tareas m´as comunes al trabajar sobre nubes de puntos 3D es la
b´usqueda de vecindades alrededor de un punto dado. Una vecindad se define
como el conjunto de puntos que est´an dentro de una figura geom´etrica determi-
nada en el espacio con radio R y con el centro en el punto para el cual se est´an
buscando vecinos. Esta combinaci´on de figura geom´etrica y de radio se conoce
como kernel de b´usqueda. Otro m´etodo alternativo de an´alisis de vecindades
es buscar los K puntos m´as cercanos al elemento de partida, t´ecnica llamada
KNN (k nearest neighbors). Cabe destacar, eso s´ı, que el presente trabajo se
centra exclusivamente en el primer enfoque (b´usquedas de geometr´ıa de kernel),
quedando las b´usquedas KNN fuera de alcance.

Un gran problema a nivel de hardware de este proceso sobre nubes de puntos
es que estos datos se recogen sin ning´un tipo de estructura ni orden basado en
localidad. Este fen´omeno se junta con el hecho de que estas nubes pueden tener
millones de puntos almacenados. Por eso, se necesitan m´etodos eficientes para
acelerar este proceso, intr´ınsecamente costoso, de acceso a puntos cercanos.

Los Octrees y los KDTrees son estructuras jer´arquicas ampliamente utiliza-
das para lidiar con este problema de ineficiencia. Consisten en dividir la nube de
puntos de manera recursiva en zonas pr´oximas en el espacio f´ısico. Los Octrees
son estructuras donde cada nodo se divide exactamente en 8 hijos, dividiendo
cada dimensi´on a la mitad (solo se usa en espacios tridimensionales). Los pun-
tos pertenecen al nodo hoja que los contiene en su interior. Por otra parte, los
KDTrees son ´arboles binarios que dividen cada nodo interno solamente seg´un
una dimensi´on. En cada nivel se elige un eje y se traza un hiperplano que di-
vide a los puntos en dos nodos. De esta forma, este ´arbol es generalizable para
cualquier espacio de dimensiones.

1

2

CAP´ITULO 1. INTRODUCCI ´ON

El rendimiento de estas estructuras depende directamente de la distribuci´on
de los puntos. Por eso es muy ´util la aplicaci´on de t´ecnicas de reordenamiento
espacial basadas en curvas de llenado de espacio (SFC). Su objetivo es mejorar la
coherencia entre la localidad espacial de los puntos y su disposici´on en memoria
para optimizar los accesos en el c´alculo de puntos vecinos.

En este trabajo se usar´a como base el modelo de Octrees propuesta por
Vi˜nambres et al.[17], donde se aplican las reordenaciones espaciales de Mor-
ton y Hilbert sobre diferentes Octrees implementados en el lenguaje C++. Se
construy´o tanto una estructura cl´asica basada en punteros enlazados como una
versi´on basada en almacenamiento en un vector lineal. Sobre estas estructuras,
y los algoritmos de b´usqueda de vecindades asociados a ellas, se tratar´a de opti-
mizar todav´ıa m´as el acceso a puntos cercanos. Para ello se emplear´a la t´ecnica
algor´ıtmica de la poda.

Esta estrategia, de forma general, consiste en descartar o podar regiones del
espacio (nodos) o conjuntos de datos para los que matem´aticamente se puede
demostrar que no satisfacen las condiciones necesarias para ser considerados
soluci´on. Su aplicaci´on en este contexto es ser capaz de conocer a priori qu´e
partes del Octree no pueden contener ning´un vecino para una b´usqueda dada.
Esta t´ecnica es, de hecho, el motivo por el que los Octrees (y los KDTrees)
son estructuras ideales para almacenar nubes de puntos 3D. Es muy sencillo
podar nodos comprobando si sus l´ımites est´an fuera del alcance del kernel de
vecindad. Sin embargo, en este trabajo se explorar´a la poda a nivel de puntos
dentro de un nodo, es decir, se busca ser capaz de descartar puntos pertene-
cientes a un nodo hoja sin tener que evaluarlos individualmente. De esta forma,
se acotar´ıa un rango espec´ıfico dentro del dominio espacial del nodo y se re-
chazar´ıan sin necesidad de recorrerlos secuencialmente todos los puntos de la
hoja que no perteneciesen a ese rango. Para conseguir esta tarea se realizar´an
reordenamientos de puntos a nivel de hoja siguiendo diversos criterios basados
en coordenadas cartesianas con ejes X, Y, Z como definidores de los puntos,
pero tambi´en se manejar´an sistemas alternativos de de representaciones tridi-
mensionales: sistemas esf´ericos y cil´ındricos. Estos sistemas son expansiones a
espacios tridimensionales de las coordenadas polares.

Las coordenadas polares son un sistema de coordenadas bidimensional donde
cada punto es caracterizado por una distancia radial y un ´angulo con respecto
del origen. Las coordenadas cil´ındricas a˜naden el componente de la altura a las
propiedades de un punto. Por otro lado, las coordenadas esf´ericas descomponen
la posici´on en el radio tridimensional, el ´angulo θ (coaltitud) y el ´angulo φ
(azimutal).

Los reordenamientos en cuesti´on consisten en almacenar los puntos dentro de
cada nodo terminal ordenados seg´un alguna caracter´ıstica de las mencionadas.
As´ı, se podr´an calcular los l´ımites de forma te´orica para ese valor que puede
tener un punto de forma que satisfaga la restricci´on de vecindad, dada una
b´usqueda con un kernel determinado.

Para resumir, esta propuesta de algoritmo y este estudio se pueden descom-

poner en una serie de objetivos claros, que se definen a continuaci´on:

1. Usar las t´ecnicas de los reordenamientos y las podas para mejorar la

3

eficiencia en las b´usquedas de vecinos mediante kernels geom´etricos so-
bre nubes de puntos 3D capturadas por sensores LiDAR. Al hablar de
eficiencia se hace referencia a la temporal, buscando la mayor rapidez po-
sible. En este trabajo no se tomar´a como prioridad el ahorro en memoria
computacional, aunque como es l´ogico s´ı se tendr´a en cuenta y se tratar´a
de minimizar su gasto siempre que no genere un impacto negativo en el
rendimiento temporal.

2. Comprobar la validez te´orica de la estrategia de los reordenamientos y
podas a nivel de hoja en Octrees, de forma que se pueda considerar una
t´ecnica plausible, m´as all´a de los resultados temporales obtenidos en esta
implementaci´on del problema.

3. Estudiar y analizar el rendimiento de los algoritmos de b´usqueda optimi-
zados mediante la citada estrategia frente a los algoritmos de b´usqueda
de vecindades de partida, de forma que se puedan sacar conclusiones lo
m´as realistas posibles de la efectividad de este proyecto.

4. Analizar la escalabilidad de la paralelizaci´on en los nuevos algoritmos de
b´usqueda, comparando este factor tambi´en con los algoritmos de partida.

4

CAP´ITULO 1. INTRODUCCI ´ON

Cap´ıtulo 2

Estado del arte

El uso de Octrees como estructura de datos en la gesti´on y almacenamien-
to de nubes de puntos 3D es en la actualidad predominante a nivel global.
Existen librer´ıas de software libre ampliamente reconocidas que proporcionan
soporte nativo tanto a estructuras Octrees como a KD-Trees para optimizar la
indexaci´on espacial.

Una de estas librer´ıas es Open3D [20]. Est´a escrita en C++ pero cuenta
con interfaces estables para numerosos lenguajes, consider´andose de facto el
est´andar en Python para la manipulaci´on y visualizaci´on 3D. Open3D incluye
m´etodos avanzados para el c´alculo de distancias y la b´usqueda de vecindad,
dando soporte tanto a consultas basadas en kernels de radio como a la t´ecnica
de los K vecinos m´as cercanos (K-Nearest Neighbors o KNN).

Otro gigante en el manejo de nubes de puntos, considerada la librer´ıa de
referencia absoluta en el entorno acad´emico e industrial para el procesamiento
de geometr´ıa 3D en C++, es PCL (Point Cloud Library) [15]. PCL proporciona
un ecosistema masivo y modular que da soporte nativo a estructuras espaciales
mediante las clases pcl::Octree y pcl::KdTree. El enfoque de PCL en sus
Octrees tradicionales se basa en una estructura din´amica de punteros. Este
dise˜no otorga una gran versatilidad para realizar modificaciones en tiempo real
sobre la nube, como la inserci´on o eliminaci´on din´amica de puntos y la detecci´on
de cambios espaciales. Por el contrario, introduce una severa penalizaci´on en el
rendimiento de la memoria. Esto se debe a la incoherencia entre la localidad
espacial de los datos y su disposici´on f´ısica en el heap o mont´on, provocando
constantes fallos de cach´e y saltos de punteros en el direccionamiento de la CPU
durante los recorridos de b´usqueda.

Un aspecto cr´ıtico que comparten tanto Open3D como PCL es que no imple-
mentan sus algoritmos de b´usqueda desde cero, sino que integran internamen-
te la librer´ıa FLANN (Fast Library for Approximate Nearest Neighbors) [12].
FLANN es una biblioteca altamente optimizada para la b´usqueda de vecinos
m´as cercanos en espacios vectoriales multidimensionales. Si bien su principal
caracter´ıstica es la capacidad de realizar b´usquedas aproximadas (Approximate
Nearest Neighbor o ANN) para mitigar la maldici´on de la dimensionalidad sa-
crificando un margen m´ınimo de precisi´on, en el contexto de b´usquedas exactas
por radio o kernel (como las evaluadas en este estudio) se configura en su modo

5

6

CAP´ITULO 2. ESTADO DEL ARTE

exacto para operar como l´ınea base de comparaci´on directa.

Otras librer´ıas relevantes con el mismo prop´osito, escritas tambi´en en C++,
son PicoTree [4] y NanoFLANN [3], ambas basadas en KD-Trees. PicoTree
genera la estructura integrada en un array contiguo mediante la indexaci´on
previa de los puntos, permitiendo b´usquedas esf´ericas y de KNN altamente
eficientes. Por su parte, NanoFLANN, que es un fork de la librer´ıa FLANN
hecha para optimizar el gasto en memoria y la eficiencia en b´usquedas por
radio, adem´as de ser flexible para operaciones en 2 y 3 dimensiones.

Por otro lado, las curvas de llenado del espacio (SFC) [19] son ampliamente
aplicadas a estructuras jer´arquicas para tareas de indexado o compresi´on. El
funcionamiento de las SFC consiste en representar un hipercubo N -dimensional
mediante una l´ınea continua. En el caso de las nubes de puntos 3D nos interesa
mapear los puntos tridimensionales a una representaci´on unidimensional. Para
lograr este reordenamiento, en la literatura destacan dos estrategias diferentes,
las curvas de Morton y las curvas de Hilbert.

La curva de Morton basa su estrategia en el entrelazado de bits de las coor-
denadas binarias de los puntos, dividiendo el espacio en cuadrantes de forma
recursiva. Al alternar los bits asociados a cada dimensi´on, se consigue un ´ındice
´unico para cada regi´on en el espacio tridimensional. La ventaja de esta apro-
ximaci´on radica en su eficiencia computacional y simplicidad algor´ıtmica, ya
que la codificaci´on y decodificaci´on se realizan mediante operaciones a nivel de
bits en la CPU. Sin embargo, introduce discontinuidades en los l´ımites de los
cuadrantes y esto provoca incoherencias en cuanto a localidad en memoria de
puntos pr´oximos.

Por el contrario, la curva de Hilbert se define como una curva fractal conti-
nua. Esta resuelve el problema de las discontinuidades presente en los c´odigos de
Morton, garantizando que aquellos puntos que se encuentran adyacentes en el
espacio tridimensional mantengan una proximidad estricta en la representaci´on
lineal. Para lograr esta continuidad f´ısica, el algoritmo hace uso de rotaciones
de los cuadrantes espaciales de forma recursiva. Aunque esta propiedad ofrece
una preservaci´on de la localidad espacial muy superior, su c´alculo matem´atico
conlleva una mayor complejidad algor´ıtmica y un coste computacional superior
al de la curva de Morton.

Recientemente, la integraci´on de c´odigos de Morton en estructuras de Octree
lineales ha sido una estrategia clave para mejorar la eficiencia en la ordenaci´on
de puntos. Un ejemplo de ello es el trabajo de Behley et al. con la librer´ıa unibn
[2], donde se emplean estos c´odigos para reducir la necesidad de recorrer el ´arbol
en toda su profundidad durante las b´usquedas de vecindad, logrando superar
el rendimiento de otras herramientas populares de la disciplina.

No obstante, la aplicaci´on directa de SFC en la b´usqueda de vecindades en
nubes de puntos 3D obtenidas con sensores LiDAR fue implementada original-
mente en el trabajo de Vi˜nambres et al. [17]. En dicha investigaci´on se analiza
en primer lugar una estructura Octree de punteros est´andar construida median-
te fragmentaciones recursivas. A pesar de que a esta estructura se le pueden
aplicar reordenamientos seg´un curvas de Morton o Hilbert para mejorar la cer-
can´ıa en memoria de los puntos pr´oximos, el enfoque cl´asico sigue adoleciendo
del problema de los accesos indexados: cada transici´on entre nodos durante el

7

recorrido implica saltar a una direcci´on del mapa de memoria que puede estar
totalmente dispersa.

Por este motivo, los autores propusieron e implementaron una versi´on lineal
del Octree, la cual almacena todos los puntos de la nube en un ´unico vector
contiguo, garantizando que todos los elementos pertenecientes a un mismo no-
do hoja residan en posiciones f´ısicas consecutivas del array. Adicionalmente, se
introdujo un algoritmo de b´usqueda por kernel geom´etrico que optimiza el pro-
ceso de recorrido: a diferencia del m´etodo cl´asico que desciende recursivamente
verificando la mera intersecci´on, este algoritmo identifica de forma temprana
si el volumen del kernel contiene por completo el ´area que cubre el nodo (sea
este interno o terminal). De cumplirse esta condici´on de inclusi´on total, el al-
goritmo realiza un volcado secuencial directo de todos los puntos del sub´arbol,
abortando el descenso.

En la publicaci´on de Vi˜nambres et al. [17] se presentan resultados expe-
rimentales que demuestran la superioridad del Octree lineal (en sus variantes
con reordenamiento de Morton y Hilbert) frente al Octree cl´asico de punteros.
Asimismo, se incluye un exhaustivo an´alisis comparativo de rendimiento fren-
te a las soluciones de c´odigo abierto de referencia mencionadas anteriormente,
tales como los m´odulos de PCL, PicoTree, NanoFLANN y la librer´ıa unibn.
Bajo diferentes tipolog´ıas de datasets y variaciones en el radio del kernel de
b´usqueda, el Octree lineal report´o, de manera sistem´atica, los menores tiempos
de ejecuci´on computacional.

Sobre este escenario t´ecnico se fundamenta el presente trabajo. En la pr´acti-
ca totalidad de las librer´ıas del estado del arte existe un patr´on de dise˜no com´un:
las estructuras justifican su eficiencia bas´andose exclusivamente en la aplicaci´on
de la poda a nivel de nodos intermedios (ramas). Mediante funciones de solapa-
miento geom´etrico, se descartan de forma temprana sub´arboles completos cuyos
vol´umenes no intersecan con el kernel de consulta. Sin embargo, una vez que el
algoritmo alcanza un nodo hoja v´alido, se ve obligado a evaluar la distancia de
todos los puntos contenidos en ´el de manera individual y secuencial.

Hasta donde se tiene constancia en la literatura t´ecnica y en las soluciones de
c´odigo abierto analizadas, no existe ninguna implementaci´on que acote el rango
de puntos a evaluar mediante una poda matem´atica interna que act´ue de forma
nativa en el interior del propio nodo hoja. Este cuello de botella constituye la
principal motivaci´on y el n´ucleo de la contribuci´on de este trabajo.

8

CAP´ITULO 2. ESTADO DEL ARTE

Cap´ıtulo 3

Materiales

En este cap´ıtulo se definir´an y precisar´an todas las herramientas, m´etodos,
algoritmos y equipos que fueron empleados y tomados para la realizaci´on del
estudio.

3.1. Equipos de c´omputo y entorno de desarrollo

El entorno hardware en el que fueron desarrollados los experimentos de este
proyecto es el Centro de Supercomputaci´on de Galicia (CESGA). Este centro es
un nodo de la Red Espa˜nola de Supercomputaci´on y datos (RES) y cuenta con
una capacidad de computaci´on total de 4.36 PetaFLOPS, estando compuesto
por 357 nodos interconectados a trav´es de una red Infiniband HDR.

Existen diferentes grupos de nodos con distintas especificaciones hardwa-
re. En concreto, estos experimentos fueron realizados en un nodo del tipo ilk,
grupo formado por un total de 256 nodos de c´omputo con las especificaciones
detalladas a continuaci´on.

Cuadro 3.1: Especificaciones t´ecnicas del entorno de ejecuciones [5].

Componente

Detalle y Configuraci´on

Procesador (CPU)
N´ucleos de C´omputo
Memoria RAM
Almacenamiento Local
Interconexi´on

2× Intel Xeon Ice Lake 8352Y
32 n´ucleos por procesador (64 cores/nodo)
256 GB (247 GB disponibles para uso real)
960 GB SSD NVMe
1× InfiniBand HDR 100

El entorno de software configurado y desplegado sobre esta arquitectura de

hardware consta de las siguientes especificaciones:

Sistema Operativo: Distribuci´on Rocky Linux versi´on 8.4 (Green Ob-
sidian), compatible con entornos Red Hat Enterprise Linux (RHEL).

Est´andar de Lenguaje: Se emple´o el est´andar C++20.

Compilador: Las implementaciones fueron compiladas mediante la GNU
Compiler Collection (GCC), cargando expl´ıcitamente el m´odulo gcc/12.3.0.

9

10

CAP´ITULO 3. MATERIALES

Directivas de Optimizaci´on y Extensiones Vectoriales: Para la ge-
neraci´on de los binarios ejecutables, se aplic´o el nivel de optimizaci´on -O2
junto con la macro -DNDEBUG para inhabilitar aserciones de depuraci´on y
maximizar el rendimiento. Adicionalmente, el sistema realiza un an´alisis
de compatibilidad por hardware en el nodo de c´omputo para activar de
forma nativa las siguientes extensiones vectoriales:

(cid:136) -mavx2 (Advanced Vector Extensions 2 ): Permite la vectorizaci´on de
bucles mediante instrucciones SIMD en registros de 256 bits, proce-
sando m´ultiples coordenadas de puntos 3D de forma simult´anea.
(cid:136) -mbmi2 (Bit Manipulation Instruction Set 2 ): Introduce soporte di-
recto por hardware para instrucciones avanzadas de manipulaci´on de
bits (como PDEP y PEXT).

Asimismo, se habilit´o la optimizaci´on interprocedural o en tiempo de en-
lazado (LTO/IPO) a trav´es de CMake, forzando a GCC a optimizar el
proyecto de forma global y permitiendo la inserci´on en l´ınea (inlining) de
funciones entre diferentes archivos fuente.

Soporte de Paralelismo: Se incluy´o la API OpenMP [13], encargada
de la gesti´on y paralelizaci´on de hilos en memoria compartida. Se aplica
en el c´odigo a˜nadiendo directivas que definen el nivel de concurrencia en
la inicializaci´on de los bucles de b´usqueda masiva sobre la estructura del
Octree.

Aunque el descrito fue el entorno de ejecuci´on real, se utiliz´o un entorno
local de desarrollo para la programaci´on y depuraci´on del c´odigo fuente. Este
entorno f´ısico de soporte est´a constituido por una arquitectura de c´omputo
x86 64 basada en un procesador AMD Ryzen 7 6800H de 8 n´ucleos f´ısicos y 16
hilos de ejecuci´on concurrentes y un sistema de almacenamiento de memoria
cach´e multinivel (L1, L2 y L3 de 512 KiB, 4 MiB y 16 MiB respectivamente).
Con el fin de compatibilizar el ecosistema de herramientas de desarrollo y
mantener un entorno de terminal nativo unificado, el software se implement´o
sobre una capa de virtualizaci´on completa mediante el subsistema Windows
Subsystem for Linux (WSL2) bajo el hipervisor de Microsoft. Sobre esta capa se
despleg´o una distribuci´on del sistema operativo Linux, concretamente Ubuntu
24.04.3 LTS. Este entorno local sirvi´o como plataforma de validaci´on para el
dise˜no de los algoritmos, la verificaci´on de la correcta aplicaci´on de la poda y su
integraci´on con el c´odigo original antes de su posterior despliegue y compilaci´on
en los nodos de c´omputo del CESGA.

3.2. Marco te´orico matem´atico

3.2.1. Coordenadas tridimensionales

La base de este trabajo es optimizar el c´alculo de distancias entre dos pun-
tos en un espacio tridimensional. Concretamente, se trabaja con dos figuras
geom´etricas (cubo y esfera) que envuelven los puntos de b´usqueda. En ambas

3.2. MARCO TE ´ORICO MATEM ´ATICO

11

se hace uso de las 3 componentes cartesianas de un punto tridimensional en las
comprobaciones de vecindad:

Kernel esf´erico: Dos puntos son vecinos si la distancia dij entre el centro
de b´usqueda es menor al radio de b´usqueda.

(cid:136) F´ormula de la distancia:

dij =

(cid:113)

(xi − xj)2 + (yi − yj)2 + (zi − zj)2

(cid:136) Existe vecindad si se cumple: dij < Rb´usqueda

Kernel c´ubico: Dos puntos son vecinos si la distancia entre los puntos
para los tres ejes dimensionales por separado es menor al radio del cubo
de b´usqueda:

(cid:136) Para un eje k ∈ {X, Y, Z}, la distancia dkij se define como:

(cid:136) Existe vecindad si se cumple:

dkij = |ki − kj|

|xi − xj| ≤ R ∧ |yi − yj| ≤ R ∧ |zi − zj| ≤ R

Para potenciar la poda o el descarte de puntos sin tener que realizar esta
comprobaci´on, se har´a uso de otros sistemas de coordenadas tridimensionales
existentes aparte de los cartesianos: el sistema esf´erico y el cil´ındrico.

Ambos son extensiones del sistema bidimensional polar. En estas coorde-
nadas un punto se define a trav´es de dos magnitudes: un radio R y un ´angulo
θ sobre el origen. Para la proyecci´on a 3 dimensiones, existe un sistema que
a˜nade un ´angulo con respecto al plano y proyecta el valor del radio bidimen-
sional a un radio tridimensional (coordenadas esf´ericas) [18]. Por otra parte,
se puede incluir la componente Z, que le otorga la propiedad de altura a los
puntos(coordenadas cil´ındricas). La conversi´on de un punto a estos sistemas a
partir de sus componentes se hace a trav´es de las siguientes operaciones:

Coordenadas esf´ericas

R =

(cid:112)

x2 + y2 + z2

φ = arctan(y/x)

θ = arc cos(z/R)

Coordenadas cil´ındricas

R =

(cid:112)

x2 + y2

φ = arctan(y/x)

Z = z

En el contexto concreto de los puntos dentro del Octree, se debe tener en
cuenta que para poder operar a nivel de hoja se debe trasladar el origen de
las coordenadas al centro del octree antes de realizar cualquier conversi´on u
operaci´on.

12

CAP´ITULO 3. MATERIALES

3.2.2. Curvas de llenado de espacio

Con el fin de optimizar la localidad espacial de los datos en memoria se
introducen las curvas de llenado del espacio (SFC, Space-Filling Curves) co-
mo herramientas de ordenaci´on [1]. Se encargan de forzar que aquellos puntos
pr´oximos en el espacio tridimensional se almacenen de forma contigua en la
zona de memoria reservada para la nube.

El proceso requiere la discretizaci´on previa del espacio continuo de la nube
de puntos. Sea una nube de puntos P ⊂ R3, se define su caja contenedora o
bounding box como la regi´on semiabierta B = [xm, xM ) × [ym, yM ) × [zm, zM ),
delimitada por sus esquinas inferiores y superiores. Fijando un nivel de subdi-
visi´on L ∈ N que determina una resoluci´on de N = 2L, se proyecta una rejilla
discreta tridimensional SL = [0, N ) × [0, N ) × [0, N ) ⊂ N3, cuya cardinalidad
total es de 23L celdas. Cada punto continuo p = (x, y, z) ∈ P es trasladado,
escalado y transformado en un elemento entero de la rejilla SL mediante la
aplicaci´on de la funci´on suelo.

En la implementaci´on de los Octrees se fij´o un nivel m´aximo de subdivisi´on
L = 21. Esto permite codificar la posici´on tridimensional de cada celda en un
´unico identificador entero de 64 bits (restringiendo las coordenadas discretas
originales a tipos de 32 bits), minimizando la probabilidad de colisi´on de m´ulti-
ples puntos en una misma celda sin penalizar el rendimiento del sistema. Una
curva de llenado del espacio de nivel l se define formalmente como la funci´on
biyectiva:

C : Sl −→ [0, 23l)

Para el prop´osito de evaluar el impacto de la localidad en el rendimiento de
los algoritmos de b´usqueda, se consideran las dos familias ya mencionadas de
curvas de llenado del espacio recursivas:

C´odigo de Morton (Curva Z)

La curva de Morton, o curva Z, mapea las coordenadas tridimensionales
discretas mediante el entrelazado directo de sus bits a nivel de registros de
la CPU. Para un punto discretizado en la rejilla cuyas coordenadas binarias
se expresan como p = (x0 . . . xL−1, y0 . . . yL−1, z0 . . . zL−1) ∈ SL, su c´odigo de
Morton se calcula de acuerdo con la ecuaci´on:

CM (x0 . . . xL−1, y0 . . . yL−1, z0 . . . zL−1) = x0y0z0x1y1z1 . . . xL−1yL−1zL−1

La ventaja computacional de esta estructura radica en la velocidad y efi-
ciencia de su c´alculo. Al tratarse de un mapeo posicional directo de bits, la co-
dificaci´on y decodificaci´on puede implementarse a trav´es de operaciones l´ogicas
de enmascaramiento (bitmasks) y tablas de b´usqueda (LUT, Look-Up Tables).

Curva de Hilbert

El c´alculo del c´odigo de Hilbert se realiza mediante un proceso iterativo
por nivel de subdivisi´on en el cual, tras extraer los bits hom´ologos de cada

3.3. ALGORITMOS Y ESTRUCTURAS DE DATOS DE PARTIDA

13

dimensi´on, se aplica una secuencia de transformaciones geom´etricas y rotaciones
que dependen del estado de orientaci´on previo y del octante evaluado.

Matem´aticamente, la propiedad fundamental de las curvas de Hilbert es su
continuidad en el espacio, satisfaciendo la condici´on de vecindad de forma que
dos c´odigos consecutivos en el espacio unidimensional corresponden a celdas
contiguas en el espacio tridimensional:

∥C−1(i) − C−1(i − 1)∥∞ = 1,

∀i ∈ [1, 23l−1]

(3.1)

A nivel te´orico, esta curva ofrece una localidad espacial superior a la de
Morton. Sin embargo, carece de una correspondencia de bits directa y requiere
un algoritmo iterativo m´as costoso para computar las matrices de rotaci´on.
Sin embargo, en este problema concreto debe prevalecer la calidad sobre la
complejidad a la hora de calcular los reordenamientos de las nubes de puntos,
ya que es una operaci´on que solo hace falta realizar una vez. Se puede ejecutar
incluso en un programa separado y cargar las codificaciones cuando se inicialice
la nube.

3.3. Algoritmos y estructuras de datos de partida

En este apartado se describir´an las estructuras y explicar´an los algoritmos de
b´usqueda que se usaron como base para el trabajo y que ya fueron mencionados:
el Octree lineal implementado por Vi˜nambres et al. y sus algoritmos de b´usqueda
iterativos con poda a nivel de nodo [17].

3.3.1. Octree lineal

El proceso de construcci´on es un proceso iterativo, en el cual se parte de un
nodo ra´ız que contiene toda la bounding box de la nube y divide repetidamen-
te cada nodo terminal que contenga m´as de maxLeaf puntos, siendo este un
valor param´etrico introducido por el programador. Una vez finalizada la cons-
trucci´on se obtiene un n´umero N de nodos, separados en nInternal (n´umero
de nodos internos) y nExternal (n´umero de nodos hoja). Todo el Octree est´a
formado por el vector de puntos de la nube (reordenados mediante una curva
de llenado y por lo tanto con una codificaci´on asociada) junto con una serie de
vectores persistentes que mapean las relaciones de descendencia entre nodos sin
necesidad de usar punteros. Esta serie de vectores es la siguiente:

std::vector<uint32 t>offsets

Tiene N elementos. La posici´on i almacena el ´ındice de del primer hijo
del nodo i. Todos los hijos de un nodo est´an en posiciones consecutivas.
En caso de ser el nodo i un nodo hoja se cumple que offsets[i] == 0.

std::vector<std::pair<size t, size t> >internalRanges

Tiene N elementos. En cada ´ındice guarda las posiciones inicial y final de
los puntos contenidos en ´el. Se usa para recorrer los puntos uno a uno en
caso de llegar a una hoja que interseca con el kernel. Este es el punto de
ineficiencia sobre el que se despliega este proyecto.

14

CAP´ITULO 3. MATERIALES

std::vector<Point>centers

Cada elemento centers[i] contiene las coordenadas del centro geom´etri-
co del nodo i. Tambi´en tiene N elementos.

std::vector<Vector>precomputedRadii

Tiene L elementos siendo L la profundidad m´axima encontrada en el
Octree. Almacena para cada nivel de profundidad las dimensiones de los
octantes.

Este formato presenta una serie de ventajas con respecto a un Octree de
punteros. Se eliminan las indirecciones que provocan fallos cach´e y se asegura
que toda la estructura est´e en un grupo de bloques de memoria contiguos.
Adem´as, se conoce y se tiene acceso a los puntos contenidos por un octante, ya
sea una hoja o no, y se consigue que esos puntos est´en consecutivos en memoria
gracias a la reordenaci´on por SFC.

3.3.2. Algoritmos de b´usqueda

A diferencia de las aproximaciones cl´asicas basadas en punteros, donde el
recorrido del ´arbol requiere desreferenciar enlaces hacia nodos hijos dispersos
en el mont´on o heap, el algoritmo optimizado implementado en este proyec-
to explota la geometr´ıa de los octantes y la contig¨uidad de los ´ındices para
discriminar bloques enteros de puntos.

El procedimiento general de b´usqueda geom´etrica se rige por un esquema de
recorrido recursivo sobre los nodos del ´arbol, evaluando la colisi´on espacial entre
la caja del nodo actual y el volumen del kernel de consulta. El flujo algor´ıtmico
se define en los siguientes pasos:

1. Evaluaci´on de nodos tipo rama: Si el nodo actual del octree es una ra-
ma (nodo interno), se computa la posici´on relativa de su volumen geom´etri-
co respecto a las fronteras del kernel de b´usqueda:

a) Inclusi´on completa: Si el octante se encuentra totalmente conteni-
do en el interior del kernel, se activa el mecanismo de rangos internos
(internal ranges). Esto permite indexar y volcar de manera secuen-
cial todos los puntos contenidos en dicho sub´arbol sin necesidad de
descender m´as niveles ni evaluar individualmente sus elementos.

b) Exclusi´on completa: Si el octante se halla completamente fuera
de los l´ımites del kernel, se procede a la poda (pruning) inmediata
de la rama, abortando esa v´ıa de exploraci´on.

c) Intersecci´on parcial: Si el octante interseca con las fronteras del
kernel pero no est´a contenido estrictamente en ´el, se contin´ua el
recorrido recursivo descendiendo hacia sus nodos hijos.

2. Evaluaci´on de nodos tipo hoja: Al alcanzar un nodo hoja que inter-
seca parcialmente con el kernel, el algoritmo desciende al nivel at´omico
del dato, validando punto a punto mediante criterios geom´etricos si sus
coordenadas pertenecen o no al interior del volumen de consulta.

3.3. ALGORITMOS Y ESTRUCTURAS DE DATOS DE PARTIDA

15

3. Retorno de resultados: Tras concluir el recorrido, se consolida y devuel-
ve la colecci´on de ´ındices de los puntos que se encuentran estrictamente
contenidos en el espacio delimitado por el kernel.

Para la materializaci´on y almacenamiento de estos ´ındices resultantes, se han
desarrollado dos variantes algor´ıtmicas diferenciadas por su estrategia de gesti´on
de memoria y estructuraci´on de datos: neighborsPrune y neighborsStruct.

Algoritmo neighborsPrune

Esta variante implementa una estrategia de inserci´on directa y plana. Cuan-
do el algoritmo identifica puntos v´alidos en el interior del kernel (ya sea median-
te la evaluaci´on individual en hojas o por la inclusi´on completa de una rama),
a˜nade cada ´ındice de punto vecino como un elemento independiente y separa-
do dentro de un vector din´amico est´andar (std::vector<size t>). Si bien el
coste de inserci´on es secuencial, esta aproximaci´on puede resultar innecesaria
en nubes densas, donde se insertan numerosos puntos consecutivos.

Algoritmo neighborsStruct

Con el objetivo de mitigar el coste de inserci´on masiva de elementos indi-
viduales, la variante neighborsStruct aprovecha la contig¨uidad f´ısica de los
datos reordenados en memoria. En lugar de almacenar elementos aislados, de-
fine una estructura de rangos indexados mediante el tipo:

std::vector<std::pair<size t, size t>> ranges;

Bajo este esquema, cuando una secuencia de puntos consecutivos en me-
moria pertenece de forma un´anime al kernel de consulta (caso seguro al incluir
ramas totalmente contenidas), el algoritmo compacta la informaci´on realizando
´unicamente dos operaciones de inserci´on por bloque: el ´ındice de la posici´on ini-
cial y el ´ındice de la posici´on final del rango. Esta t´ecnica reduce dr´asticamente
el espacio de almacenamiento temporal, minimiza las escrituras en memoria
cach´e, optimizando la velocidad del algoritmo.

Mediciones temporales

Para evaluar de manera rigurosa el rendimiento temporal de las estructuras
y algoritmos propuestos, en este estudio se dise˜n´o e implement´o un sistema de
medici´on integrado en la infraestructura del proyecto. Las mediciones tempo-
rales se encapsularon en una clase denominada TimeWatcher, la cual abstrae y
centraliza la captura de m´etricas temporales a trav´es de una interfaz basada en
los m´etodos start() y stop().

La infraestructura interna de esta clase hace uso de la biblioteca est´andar
de C++ <chrono> [6]. En concreto, el inicio y el fin de cada intervalo de eva-
luaci´on se gestionan mediante punteros inteligentes ´unicos (std::unique ptr),
los cuales aseguran una gesti´on de memoria eficiente y un overhead m´ınimo.

16

CAP´ITULO 3. MATERIALES

La funci´on usada para marcar los instantes temporales es high resolution clock.

Este reloj es el m´as preciso de la biblioteca <chrono>, llegando a definir m´etricas
de nanosegundos.

Con el objetivo de mantener una consistencia en la fase experimental de mi
estudio, se usar´an exactamente estas mediciones temporales ya implementadas
e integradas en la arquitectura del programa.

3.4. Conjuntos de datos empleados

Las nubes de puntos 3D que fueron usadas para evaluar el rendimiento del
algoritmo de poda a nivel de hoja vienen descritos a continuaci´on, en el Cuadro
3.2. Las nubes est´an almacenadas en archivos binarios est´andar con extensi´on
.LAS. Este formato contiene los datos espaciales tridimensionales aparte de
campos opcionales con m´as propiedades de los puntos, como intensidad, color
o clasificaciones de terreno.

La selecci´on de nubes de puntos se realiz´o tratando de buscar la m´axima
variedad posible: se mezclan conjuntos de datos de baja y alta densidad, as´ı
como nubes extra´ıdas con sensores LiDAR tanto a´ereos como terrestres y por
´ultimo variedad en el n´umero total de puntos contenidos.

3.4. CONJUNTOS DE DATOS EMPLEADOS

17

Nube

nPuntos Dataset

bildstein station1 xyz
intensity rgb

29.6 M

Semantic3D [8]

sg27 intensity rgb

73.9 M

5085 54320.las

14.4 M

DALES [16]

5095 54440.las

11.7 M

Lille 0

10.0 M Paris-Lille 3D
[14]

Paris Luxembourg 6

10.0 M

PNOA 2024 PNR
489-4672 NPC01

18.5 M PNOA-LiDAR
[9]

Mar18 train

59.4 M Hessigheim 3D
[10]

Mar18 test

51.7 M

Descripci´on
Escaneo
LiDAR
terrestre y urbano
desde una posici´on
est´atica. Nubes muy
densas.

LiDAR a´ereo obteni-
do desde aeronaves.
El conjunto se divide
en celdas disjuntas.

Sensor LiDAR de
tipo Mobile Laser
(MLS)
Scanning
instalado sobre un
veh´ıculo que mapea
dos calles urbanas en
Par´ıs.

Cobertura del terri-
torio espa˜nol median-
te LiDAR a´ereo. Pre-
senta una densidad
baja, cubriendo gran-
des extensiones.
Dataset a´ereo de alta
resoluci´on capturado
mediante UAV (Un-
manned Aerial Vehi-
cle) sobre el pueblo
de Hessigheim.

Cuadro 3.2: Nubes de puntos utilizadas en las evaluaciones experimentales.

18

CAP´ITULO 3. MATERIALES

Cap´ıtulo 4

Metodolog´ıa

En este cap´ıtulo se expone detalladamente el proceso de dise˜no, implemen-
taci´on y optimizaci´on incremental de los nuevos algoritmos de poda geom´etrica
a nivel de nodo hoja sobre la estructura del Octree lineal de partida [?]. El obje-
tivo metodol´ogico fundamental de este estudio es romper el car´acter indivisible
de los nodos hoja tradicionales, acotando el espacio de b´usqueda lineal me-
diante la aplicaci´on de criterios anal´ıticos basados en proyecciones cartesianas,
esf´ericas y cil´ındricas.

A diferencia de un desarrollo est´atico, la metodolog´ıa de este proyecto se
ha concebido como un proceso iterativo guiado por el rendimiento. A lo largo
de las siguientes secciones se describir´an las t´ecnicas utilizadas a lo largo de la
evoluci´on del software. Se parti´o de un modelo te´orico inicial enfocado en la
m´axima restricci´on geom´etrica y el ahorro de memoria, pasando por el an´alisis
de cuellos de botella computacionales hasta converger en una soluci´on de alta
localidad espacial y bajo coste de computaci´on.

4.1. Fundamentos geom´etricos de la poda en nodos

hoja

En esta secci´on se explica c´omo se realiza exactamente la poda para des-
cartar puntos dentro de hojas conflictivas. Como ya se ha visto anteriormente,
es sencillo conseguir los valores de coordenadas esf´ericas o cil´ındricas a partir
de coordenadas cartesianas tridimensionales. Para cada propiedad que las de-
fine, se puede delimitar unos valores m´aximo y m´ınimo que un punto puede
tener para que, dado un kernel de b´usqueda definido, pueda estar dentro de ese
kernel de vecindad, de forma que si no tiene un valor fuera de ese intervalo se
descartar´a autom´aticamente. Lo interesante de esta cuesti´on es no tener que
calcular esos valores uno a uno, eso es exactamente lo que hacen los algoritmos
originales. Lo que convierte este c´alculo en un algoritmo eficiente es ordenar los
puntos, de forma independiente en cada hoja, seg´un cada una de esas claves en
la inicializaci´on de la estructura. De esta forma, basta con encontrar los puntos
que suponen el l´ımite inferior y superior del intervalo y se tiene autom´atica-
mente el conjunto de puntos a evaluar, descartando el resto. La eficiencia de
esta t´ecnica depende de dos factores fundamentales: la efectividad directa de

19

20

CAP´ITULO 4. METODOLOG´IA

la poda, es decir, el porcentaje de puntos descartados, que debe ser lo mayor
posible; y la reducci´on de la sobrecarga en el c´alculo, debiendo evitar que el
coste de definir los ´ındices sobre los que buscar no supere el coste de rechazar
los puntos descartados uno a uno, tal y como se estaba haciendo.

A continuaci´on se detalla como se definen los valores m´aximos y m´ınimos
para cada propiedad posible del punto. Una consideraci´on a tener es que este
proceso no es ´unico de las coordenadas polares, se puede tomar la misma t´ecnica
con los valores de los ejes X, Y, Z. De hecho, es a priori una buena opci´on ya que
se evitan las operaciones de transformaci´on de sistema de coordenadas, por lo
que es una estrategia que se tendr´a en cuenta. En la Figura 4.1 se puede ver una
simulaci´on gr´afica de c´omo se realiza la poda en el espacio de 3 dimensiones,
en este caso usando el eje X para la poda. El cubo negro representa la hoja
del Octree mientras que el azul representa el kernel de b´usqueda c´ubico, con el
punto de b´usqueda en su centro. La zona cortada por el hiperplano que queda
en el interior de la hoja es la ´unica zona donde puede haber vecinos, sea la
distribuci´on que sea.

Figura 4.1: Funcionamiento de la poda en un espacio tridimensional usando ejes
cartesianos.

El c´alculo matem´atico de los l´ımites de b´usqueda es directo si se traslada
el origen al centro de la hoja del Octree. Dado un punto de b´usqueda Q =
(qx, qy, qz) y un radio de b´usqueda rk para cada eje k ∈ {X, Y, Z} (ya sea una
esfera o un cubo), un punto cualquiera P = (px, py, pz) perteneciente a dicha
hoja se considera un candidato potencial a vecino si y solo si sus coordenadas
satisfacen la siguiente condici´on para el componente k ∈ {X, Y, Z} que est´a
siendo estudiado:

pk ∈ [qk − rk,

qk + rk]

Lo cual equivale a la restricci´on de vecindario por componentes dada por el

valor absoluto:

|pk − qk| ≤ rk

4.1. FUNDAMENTOS GEOM ´ETRICOS DE LA PODA EN NODOS HOJA21

Aqu´ı se debe realizar un apunte. La restricci´on real para que un punto de
b´usqueda Q = (qx, qy, qz) sea vecino es la agregaci´on de la restricci´on anterior
para todas sus componentes:

∀k ∈ {X, Y, Z},

pk ∈ [qk − rk,

qk + rk]

Sin embargo, de esta forma no se pueden hacer ordenaciones y habr´ıa que
hacer intersecciones de rangos para hallar los puntos candidatos. Esto eliminar´ıa
el objetivo de la optimizaci´on, evitar tener que comprobar puntos uno a uno,
adem´as de eliminar la localidad y suponer una sobrecarga demasiado grande.
Por estos motivos se trabajar´a con las propiedades de los puntos de forma
separada para la extracci´on de rangos.

De forma an´aloga, es posible determinar anal´ıticamente los l´ımites inferiores
y superiores para las variables de los sistemas de coordenadas alternativos. Dado
un punto de b´usqueda Q = (qx, qy, qz) y un radio de consulta r, se debe realizar
una consideraci´on previa cuando se emplean entornos volum´etricos ortogonales:
en el caso del kernel c´ubico, para garantizar que ning´un candidato potencial
quede excluido y no sobreestimar las condiciones de vecindad, se toma como
radio efectivo el de la esfera m´ınima circunscrita que lo envuelve completamente,
el cual se define formalmente como refectivo = r

√

3.

En el marco del sistema de coordenadas esf´ericas, la evaluaci´on del ´angulo
azimutal φ (cuyo dominio se restringe al intervalo [−π, π)) se estructura a partir
de la proyecci´on del punto de b´usqueda en el plano xy. Se define la distancia
x + q2
q2
de la query al origen como ρq =
y. El ´angulo azimutal del punto de
b´usqueda se calcula mediante la funci´on arcotangente de dos par´ametros como
φq = atan2(qy, qx).

(cid:113)

Geom´etricamente, la m´axima desviaci´on angular posible que puede tener
un vecino a una distancia horizontal ρq viene acotada por las tangentes a la
esfera de b´usqueda. Por lo tanto, el intervalo de tolerancia angular dentro del
cual debe encontrarse un punto para optar a ser vecino se define como:

Φ = [φq − ∆φ, φq + ∆φ]

Donde el semi´angulo de apertura ∆φ se deduce de la relaci´on trigonom´etrica:

∆φ = arcsin

(cid:19)

(cid:18) r
ρq

Continuando con el sistema de coordenadas esf´ericas, la segunda com-
ponente angular a evaluar corresponde al ´angulo de coaltitud o ´angulo polar θ,
cuyo dominio f´ısico se restringe al intervalo [0, π]. Este ´angulo describe la des-
viaci´on respecto al eje de ordenadas Z y se calcula anal´ıticamente a partir de la
y + q2
distancia eucl´ıdea tridimensional al origen, definida como rq =
z ,
mediante la relaci´on:

q2
x + q2

(cid:113)

θq = arc cos

(cid:19)

(cid:18) qz
rq

De manera hom´ologa al an´alisis azimutal, la dispersi´on angular m´axima
para los candidatos a vecinos en esta dimensi´on se encuentra acotada por el

22

CAP´ITULO 4. METODOLOG´IA

contorno de la esfera de b´usqueda de radio r. Por consiguiente, el intervalo de
aceptaci´on para el ´angulo polar se establece como:

Θ = [θq − ∆θ,

θq + ∆θ]

Donde la apertura angular cr´ıtica ∆θ se deriva directamente como:

∆θ = arcsin

(cid:19)

(cid:18) r
rq

No obstante, existe una condici´on geom´etrica singular para estas dos com-
ponentes angulares que hace autom´aticamente nula la poda y permite mitigar
el coste computacional al eludir el c´alculo de funciones trigonom´etricas inversas,
de alto coste en CPU. La estrategia de poda es est´eril si se satisface la relaci´on:

r ≥ rq

(o r ≥ ρq en el plano xy)

La naturaleza de esta restricci´on radica en que, si el radio de consulta es
mayor o igual a la distancia del punto de b´usqueda al origen, el volumen del
kernel contiene de forma impl´ıcita dicho origen. En consecuencia, el espacio de
b´usqueda abarca todas las direcciones del espacio y por lo tanto todo el dominio
de valores angulares, imposibilitando cualquier restricci´on angular efectiva.

Por ´ultimo, la cota asociada a la componente radial tridimensional rp de
cualquier punto candidato P para el filtrado en esf´ericas es directa, obligando
al punto a posicionarse dentro del rango de distancias determinado por:

Resf´erico = [rq − r,

rq + r]

En contraposici´on, cuando el an´alisis se reduce a un sistema de coordena-
das cil´ındricas, la componente radial se eval´ua de forma bidimensional. La
distancia proyectada de la query se define como ρq =
y, y el criterio de
vecindad para la magnitud del radio cil´ındrico ρp de los puntos de la hoja se
simplifica al intervalo:

x + q2
q2

(cid:113)

Rcil´ındrico = [ρq − r,

ρq + r]

En la Figura 4.2 se ilustra un ejemplo de mecanismo de poda en el espacio
tridimensional, empleando en este caso el ´angulo azimutal φ. Como se observa,
el algoritmo traza dos hiperplanos orientados seg´un los l´ımites angulares calcu-
lados, descartando inmediatamente cualquier punto cuyos l´ımites geom´etricos
queden completamente fuera del sector angular Φ.

4.2. PRIMERA FASE DE IMPLEMENTACI ´ON: SELECTORES DE RANGO POLARES Y VECTORES DE PERMUTACIONES23

Figura 4.2: Funcionamiento de la poda en un espacio tridimensional usando
coordenadas esf´ericas/cil´ındricas.

4.2. Primera Fase de Implementaci´on: Selectores de
rango polares y vectores de permutaciones

Las decisiones de dise˜no iniciales se caracterizaron por buscar un equilibrio
entre la minimizaci´on de la sobrecarga en memoria introducida y en maximizar
la cuota de puntos podados. Un trabajo inicial que hubo que realizar fue ex-
poner una serie de funciones en la clase del Octree lineal para poder operar a
nivel de hoja con comodidad. Estas funciones dan acceso a una serie de vectores
que contienen los centros de las hojas y pares de ´ındices que indican el inicio y
el final de los puntos de la hoja en la estructura original del Octree, as´ı como
una funci´on que devuelve el n´umero total de hojas. Una vez integradas estas
funciones preliminares, la primera parte de la implementaci´on es construir el
reordenamiento de puntos a nivel de hoja. Esto se hace a la hora de inicializar
la estructura y las operaciones solo se llevan a cabo una vez al cargar la nube de
puntos, por lo que la eficiencia temporal no es una gran preocupaci´on. S´ı lo pue-
de ser, sin embargo, el consumo de memoria. Por eso la primera aproximaci´on
fue la de crear vectores de permutaciones. La utilidad de estos es almacenar,
dentro de una hoja, el ´ındice global de los puntos en orden creciente siguiendo
una clave concreta de las analizadas en la secci´on anterior. Para ordenar cada
vector se us´o la funci´on std::sort [7], con complejidad temporal O(N log(N )).
As´ı, para acceder, por ejemplo, al punto con el radio esf´erico menor de la hoja
i habr´ıa que acceder a points[permsRadius[i][0]] siendo points el array de
puntos originales. El beneficio de este m´etodo es que cada elemento es un size t
y ocupa 8 bytes frente a los 32 bytes que ocupa un objeto de la clase Point.
Por otro lado, se dise˜n´o la t´ecnica de selecci´on de rango. Esta funcionalidad
est´a pensada para ser ejecutada una vez por cada ocasi´on en la que se llegue
a una hoja conflictiva durante una b´usqueda, por lo que se ejecutar´a miles de
veces por ejecuci´on y ser´a trascendental buscar la m´axima eficiencia temporal,
a la vez que se debe conseguir una poda lo mejor posible. Este equilibrio a

24

CAP´ITULO 4. METODOLOG´IA

buscar es el factor m´as decisivo a la hora de enfocar el dise˜no de esta funci´on.
Este m´odulo debe devolver los rangos m´ınimo y m´aximo para examinar de la
hoja que se est´a evaluando. En ella se hacen los c´alculos matem´aticos descritos
en la anterior secci´on para conseguir los l´ımites num´ericos y a continuaci´on se
lleva a cabo una b´usqueda binaria sobre los puntos reordenados de la hoja para
transformar los valores en el intervalo de ´ındices correspondiente, por lo que la
complejidad del selector es O(log2 N ), aparte de la sobrecarga de complejidad
constante introducida por los c´alculos matem´aticos.

Se implementaron de salida dos modos de reordenamiento independientes,
uno usando las caracter´ısticas de un punto seg´un coordenadas esf´ericas y otro
seg´un coordenadas cil´ındricas. Se hizo de esta manera porque no tiene sentido
mezclar claves de ambos sistemas en una ´unica b´usqueda: Las podas que se
pueden realizar con coordenadas esf´ericas son por definici´on m´as restrictivas
que las que se pueden hacer con el sistema cil´ındrico. La poda por el ´angulo φ
es directamente la misma. La poda que el ´angulo θ lleva a cabo se define por
la posici´on en el eje Z del punto de b´usqueda, pero es m´as estricta que podar
directamente en base al valor de Z (la ventaja del sistema cil´ındrico aqu´ı es que
puede podar casos en los que el kernel cubra el centro). Por ´ultimo, el radio
tridimensional es m´as estricto que el radio en el plano XY . En conclusi´on, las
coordenadas esf´ericas consiguen un rango m´as estricto mientras que las coorde-
nadas cil´ındricas son algo menos costosas computacionalmente. Esto hace que
no tenga sentido mezclar los sistemas en las b´usquedas, y proponen una compa-
rativa interesante a estudiar entre los dos modos. Se a˜nadi´o como par´ametro de
entrada el par´ametro --local-reorder, que pod´ıa tomar uno o varios valores
de entre los siguientes: [none,spherical,cylindrical] Una vez tomada la
decisi´on de separar las reordenaciones esf´ericas y cil´ındricas se definieron los
detalles de la estrategia en la selecci´on de rango, que fue la siguiente:

1. Se investigan las tres posibles reordenaciones para el sistema de coorde-

nadas elegido. Para cada clave:

a) Se calculan los valores l´ımite tal y como se expuso anteriormente.

b) Se traducen los valores a ´ındices mediante la b´usqueda binaria.

2. Al finalizar el bucle, se comprueba qu´e clave deja un rango m´as peque˜no

a examinar.

3. Se devuelve un c´odigo que se asocia con el reordenamiento elegido, los
´ındices m´ınimo y m´aximo y el vector de permutaciones correspondiente.

4.3. An´alisis de Cuellos de Botella y Redise˜no del

Sistema

Una vez terminada la implementaci´on e integraci´on de la versi´on base de
la poda, se llevaron a cabo unas pruebas preliminares para poder tener una
primera referencia de su desempe˜no. Los resultados fueron negativos para di-
ferentes tipos de nubes, por lo que hubo que pasar por un proceso de an´alisis

4.3. AN ´ALISIS DE CUELLOS DE BOTELLA Y REDISE ˜NO DEL SISTEMA25

exhaustivo para encontrar las causas y soluciones del mal rendimiento de la im-
plementaci´on de las podas con respecto a los algoritmos base. Para conocer la
efectividad de la poda en s´ı se introdujo el modo de debug en el programa, con
el que se exportan a un archivo informaci´on de cada hoja conflictiva a la que
se llega. Se imprime qu´e modo de reordenamiento se est´a usando (cil´ındrico o
esf´erico), qu´e clave concreta fue mejor y por tanto se us´o en esa hoja, el n´umero
de puntos de la hoja y el n´umero de puntos restantes despu´es de la poda. Con
esta informaci´on se llev´o a cabo un estudio de la efectividad de la poda sobre
un peque˜no conjunto de nubes que dio lugar a los resultados mostrados en el
Cuadro ??. Tambi´en se analiz´o el ´exito de cada clave de reordenamiento, cuyos
resultados se recogen en las Figuras 4.3 y 4.4.

Reordenamiento

Radio (r)

Cubo Esfera Cubo Esfera Cubo Esfera

Paris-Lille

sg 27 station

Lille 0

Cil´ındrico

Esf´erico

0.5
1.0
2.0
3.0

0.5
1.0
2.0
3.0

13.8 % 37.0 % 39.2 % 51.9 % 5.2 % 28.2 %
8.5 % 28.8 % 6.4 % 33.3 % 6.1 % 28.4 %
7.0 % 26.0 % 3.6 % 31.7 % 12.4 % 33.5 %
6.7 % 23.9 % 2.4 % 32.2 % 17.2 % 39.1 %

9.3 % 38.7 % 22.3 % 48.6 % 2.8 % 32.3 %
3.9 % 32.6 % 6.2 % 36.2 % 3.8 % 32.4 %
2.6 % 31.4 % 3.5 % 33.8 % 9.2 % 35.5 %
1.4 % 30.8 % 2.3 % 34.2 % 13.8 % 40.1 %

Cuadro 4.1: Porcentaje de puntos podados seg´un el tipo de reordenamiento,
geometr´ıa del kernel y radio de b´usqueda (r) para las nubes analizadas.

Figura 4.3: Distribuci´on de claves ´optimas por hoja en modo esf´erico sobre la
nube sg27 station 8

26

CAP´ITULO 4. METODOLOG´IA

Figura 4.4: Distribuci´on de claves ´optimas por hoja en modo cil´ındrico sobre la
nube sg27 station 8

De estos datos se pueden extraer varios aprendizajes. En primer lugar, las
podas con kernel c´ubico tienen una efectividad pr´acticamente nula debido a
la sobreestimaci´on que hay que realizar, lo cual probablemente provoque que
para la mayor´ıa de hojas el kernel cubra el centro o incluso la hoja entera. Ese
porcentaje que se ahorra, de entre el 5 y el 10 % de las hojas, no compensa para
nada la sobrecarga introducida y el gasto en memoria. Tambi´en se aprecia un
comportamiento a tener en cuenta de aqu´ı en adelante, y es que cu´anto m´as
peque˜no sea el radio, m´as efectiva ser´a la poda de forma general. Esto ocurre as´ı
porque la secci´on angular que cubre una esfera, estando a la misma distancia
siempre del centro, es proporcional al tama˜no de esta, y esferas peque˜nas se
pueden encerrar en regiones muy estrechas. Por ´ultimo, en las Figuras 4.3 y 4.4
hay un patr´on muy claro. La clave que mejor funciona con mucha diferencia es
el ordenamiento por ´angulo φ, seguido de lejos por la coordenada Z.

Este estado de conocimiento dio pie a una serie de optimizaciones y redise˜nos

que fueron aplicados sobre los algoritmos.

4.4. Segunda Fase de Implementaci´on: Refactoriza-

ci´on y Simplificaci´on de Modelos

Tras analizar los resultados preliminares, se tomaron una serie de cambios
en el enfoque de las podas. Los principales problemas a arreglar eran la inefecti-
vidad con el kernel c´ubico y la excesiva sobrecarga en proporci´on a la magnitud
de hojas podadas.

Dada la dominancia del ´angulo φ para los reordenamientos, se decidi´o usar
solo esa clave de aqu´ı en adelante. Esta decisi´on tiene una desventaja, y es que
se quedar´an sin podar hojas que quiz´as con el ´angulo de coaltitud o el radio s´ı
descartar´ıan puntos. Pero las ventajas son un gran ahorro en memoria, teniendo
en cuenta que tambi´en se eliminan los dos modos separados de coordenadas
esf´ericas y cil´ındricas, y un gran ahorro en la sobrecarga de la selecci´on de
rangos. El funcionamiento de la selecci´on de rango en las b´usquedas ahora

4.5. CONSTRUCCI ´ON DE R ´EPLICAS CONTIGUAS E INTEGRACI ´ON EN EL FLUJO DE B ´USQUEDA DEL OCTREE27

simplemente consiste en calcular los valores l´ımites angulares en el domimio
[−π, π), realizar la b´usqueda binaria para encontrar el rango de puntos en la hoja
y devolver el rango junto con el vector de permutaciones necesario para efectuar
la b´usqueda. Cabe destacar, eso s´ı, un condicional implementado a mayores. El
dominio de valores es circular, es decir, que un kernel puede soportar un rango
del tipo [x, π), [−π, y]. Se incluy´o esta posibilidad devolviendo dos pares de
´ındices e incluyendo un nuevo condicional en los algoritmos de b´usqueda, que
recorren el segundo intervalo de puntos en caso de ser necesario.

En este punto se tom´o la decisi´on de implementar otro tipo de reordena-
miento, de forma que hubiese dos modos independientes de nuevo. En este caso,
se usar´ıan los valores de los ejes de coordenadas X, Y, Z para hacer los reorde-
namientos a nivel de hoja, de forma que habr´ıa 3 posibles claves, al igual que en
el primer algoritmo propuesto. Sin embargo, en este caso hay una serie de ven-
tajas que hacen esta implementaci´on mucho m´as atractiva. Para empezar, los
valores cartesianos ya est´an definidos en los puntos, no hay que hacer c´alculos a
mayores para transformar las coordenadas y posteriormente ordenar los puntos.
Adem´as, el c´alculo de l´ımites es extremadamente r´apido. Tal y como se detall´o
en la secci´on 4.1, no es m´as que una suma/resta. A mayores, se espera que este
formato arregle el problema presente en los resultados de poda obtenidos de la
dependencia al tama˜no del radio. Los ejes cartesianos pueden podar cualquier
secci´on de la hoja, mientras que el efecto del ´angulo azimutal se anula cuando
el kernel sobrepasa el centro de la hoja. Por ´ultimo, hay una nueva optimiza-
ci´on sobre la selecci´on de rango posible para este caso. El c´alculo de la regi´on
entre los valores l´ımites es trivial y nada costosa, por lo que se puede usar como
referencia para determinar cu´al es el eje que m´as puntos desecha y se elimina
la necesidad de ejecutar las b´usquedas binarias por partida triple. Dado que
no se asegura una distribuci´on uniforme de los puntos, este m´etodo no asegura
que se est´e escogiendo la clave ´optima siempre, pero es una aproximaci´on muy
eficiente temporalmente y no se espera una p´erdida significativa en efectividad
de poda.

Por tanto, se llevo a cabo la refactorizaci´on, uni´on y simplificaci´on del al-
goritmo de poda basado en coordenadas esf´ericas/cil´ındricas (modo de reor-
denamiento polar de aqu´ı en adelante) y se implement´o el modo basado en
coordenadas ordenadas (modo cartesiano de aqu´ı en adelante). La implemen-
taci´on de las funciones de selecci´on de rango y algoritmos de b´usqueda se hizo
en funciones separadas para aumentar el desacoplamiento y evitar saltos con-
dicionales en tiempos de ejecuci´on.

4.5. Construcci´on de r´eplicas contiguas e integraci´on

en el flujo de b´usqueda del Octree

En paralelo a la implementaci´on de los nuevos modos polar y cartesiano,
se analiz´o y dise˜n´o otra optimizaci´on temporal, basada esta vez en la localidad
de memoria durante el bucle de b´usqueda. Hasta ahora, las reordenaciones se
estaban haciendo mediante vectores de permutaciones, lo que provocaba accesos
a memoria extra en relaci´on al algoritmo base. En las Figuras 4.5 y 4.6 vemos

28

CAP´ITULO 4. METODOLOG´IA

una comparaci´on entre el bucle base y el modificado para poder recorrer el
intervalo podado mediante indirecciones para el algoritmo neighborsPrune:

Require:

leafStart, leafEnd , points, k

Ensure: ptsInside

1: for i = leafStart to leafEnd − 1

do

2:

3:

if k.isInside(points[i]) then
ptsInside.push back(i)

end if

4:
5: end for

Require:

rangeStart, rangeEnd , points, perms, k

Ensure: ptsInside

1: for i = rangeStart to rangeEnd −1

do

if
then

2:

3:

k.isInside(points[perms[i]])

ptsInside.push back(perms[i])

end if

4:
5: end for

Figura 4.5: B´usqueda base (Acceso
secuencial).

Figura 4.6: B´usqueda reordenada (Ac-
ceso indexado).

El objetivo es poder aplicar el bucle 4.5 a la vez que las podas a nivel de
hoja. Para conseguirlo, es necesario hacer una r´eplica del vector de puntos de
forma que cada hoja empiece en el mismo ´ındice, para mantener la coherencia
con la parte exterior del algoritmo, pero estando los puntos reordenados a nivel
de hoja. Esta decisi´on introduce un gasto mayor en memoria, pero se asume
teniendo en cuenta que el objetivo principal del trabajo es encontrar eficiencia
temporal y que hasta ahora los resultados no hab´ıan sido positivos, adem´as de
que con la simplificaci´on de los algoritmos de selecci´on de rangos ya se hizo un
ahorro de memoria sobre la versi´on inicialmente planteada. Concretamente, el
tama˜no de cada vector contiguo pasa a tener tama˜no De esta forma, despu´es
de la construcci´on de la estructura, se intercambia la creaci´on de los vectores
persistentes de permutaciones por duplicaciones de points con reordenamientos
incluidos a nivel de hoja.

En el caso del modo polar, este cambio funcion´o de manera correcta. El
formato del bucle de b´usqueda es el mismo. Los ´ındices guardados en el al-
mac´en de vecinos ptsInside hacen referencia al vector de reordenamiento po-
lar polarPoints. La ´unica diferencia es que en este modo se incluye el bucle
condicional sobre el segundo rango que se recorrer´a en caso de existir, tal y
como se expuso en la secci´on 4.4.

Sin embargo, en el modo cartesiano la b´usqueda no es tan sencilla. Se est´an
mezclando tres vectores de reordenamiento diferentes en diferentes hojas de la
misma b´usqueda. Por tanto hay que recuperar un puntero al inicio de la hoja
de uno de ellos dependiendo de cu´al produjese la mejor poda. Aparte de esa
m´ınima sobrecarga, hay un problema mayor a la hora de incluir los ´ındices en
el almac´en de vecinos. No se pueden a˜nadir mediante la forma push back(i)
porque una vez terminada la b´usqueda, a la hora de acceder y reconstruir los
vecinos, ser´ıa imposible saber a qu´e vector apunta cada ´ındice.

La soluci´on que se adopt´o fue usar el campo id existente en la estructura
Point. En ella, a la hora de construir los vectores reordenados, se incluyeron las

4.5. CONSTRUCCI ´ON DE R ´EPLICAS CONTIGUAS E INTEGRACI ´ON EN EL FLUJO DE B ´USQUEDA DEL OCTREE29

posiciones originales de cada punto. De esta forma, durante el bucle se insertan
los puntos accediendo a la funci´on id() del punto, una funci´on getter del campo.
La ineficiencia de este acceso extra existe, pero teniendo en cuenta que es una
funci´on inline y que el objeto Point fue accedido justo antes y deber´ıa estar en
cach´e, es la mejor soluci´on posible ante el problema planteado.

Esta explicaci´on, sin embargo, solo aplica al algoritmo neighborsPrune.
El algoritmo neighborsStruct incluye una forma de almacenado de vecinos
basada en pares de ´ındices, como se aprecia en la siguiente Figura 4.7.

Require: leafStart, leafEnd , points, k
Ensure: ptsInside

1: rangeStart ← leaf Start
2: for i = leafStart to leafEnd − 1 do
3:

if ¬ k.isInside(points[i]) then
if rangeStart < i then

result.addRange(rangeStart, i)

end if
rangeStart ← i + 1

4:

5:

6:

7:

end if

8:
9: end for

Figura 4.7: B´usqueda base para el algoritmo neighborsStruct.

Nuevamente, el modo polar aplica exactamente la misma l´ogica que las
b´usquedas originales, por lo que la adaptaci´on a este algoritmo es trivial. Por
otra parte, hasta donde se analiz´o, no es posible la adaptaci´on del modo carte-
siano. Los rangos en un vector de reordenamiento concreto se corresponden con
puntos aleatoriamente distribuidos en el vector Points, por lo que es imposible
hacer una inserci´on de vecinos mediante rangos.

Para terminar el cap´ıtulo de implementaci´on, cabe mencionar que se in-
trodujo un par´ametro personalizable threshold al programa. Su funci´on es
impedir que la sobrecarga de selecciones de rango sea mayor al tiempo ahorra-
do en el bucle de b´usqueda. Por tanto, cualquier hoja con Npuntos < threshold
ser´a recorrida de forma secuencial.

30

CAP´ITULO 4. METODOLOG´IA

Cap´ıtulo 5

Pruebas

En este cap´ıtulo se expondr´an los experimentos ejecutados en los nodos de
computaci´on del CESGA y los resultados obtenidos. El experimento se divide
en dos secciones: el an´alisis principal, el de tiempos de ejecuci´on, y un an´alisis
complementario de escalabilidad de paralelizaci´on. Los experimentos de tiempos
de ejecuci´on se dividen a su vez en b´usquedas de subconjuntos con centros
aleatorios y en b´usquedas sobre nubes completas.

5.1. Plan de Verificaci´on y Correcci´on Global

Como paso previo al an´alisis de rendimiento, se dise˜n´o un script de prue-
bas unitarias automatizadas para verificar la fidelidad de los nuevos algoritmos
orientados a la poda en nodos hoja. Para cada una de las configuraciones eva-
luadas en este cap´ıtulo, se contrastaron los resultados de vecindad devueltos por
las variantes optimizadas frente a los resultados reportados por la implementa-
ci´on base del Octree lineal cl´asico. La verificaci´on produjo una coincidencia del
100 %, por tanto se pas´o al estudio de los resultados temporales, con la garant´ıa
de que en las podas se descartan ´unicamente regiones del espacio geom´etrico
disjuntas al kernel.

Adicionalmente, se us´o el modo de debug implementado para tener una me-
dida de la efectividad te´orica de las podas, m´as all´a de saber si son eficientes
temporalmente o no. En la Figura 5.1 se recoge el porcentaje puntos analizados
despu´es de calcular la poda y el porcentaje de puntos que resultaron ser vecinos
en relaci´on al n´umero de puntos totales de cada hoja para la nube de puntos
de Lille 0. Los resultados a lo largo de las diferentes nubes fueron muy consis-
tentes, siendo este dataset lo suficientemente representativo. Para obtener estos
resultados se us´o una muestra de 80 b´usquedas con centros aleatorios. El caso
ideal es que el n´umero de puntos analizados despu´es de la poda y el n´umero de
puntos vecinos sea el mismo. El tama˜no de los kernels se muestra representado
como la media de la relaci´on entre el radio y el tama˜no de las hojas que se
podan, de forma que se pueda tener una visi´on m´as independiente al dataset
de c´omo funciona la poda con diferentes tama˜nos de b´usqueda.

Los porcentajes mostrados indican un comportamiento claro: el modo polar
poda de manera aceptable en las b´usquedas con kernel esf´erico, pero apenas

31

32

CAP´ITULO 5. PRUEBAS

poda en b´usquedas c´ubicas. Por el contrario, el modo cartesiano realiza podas
que est´an cerca de ser ideales para geometr´ıa c´ubica, en cambio descarta muy
pocos puntos en b´usquedas esf´ericas. En la poda polar hay otra tendencia clara
observable, y es que a medida que el radio es m´as peque˜no en relaci´on a la mag-
nitud de los octantes, se descarta un mayor porcentaje de puntos y se reduce
la diferencia con el porcentaje de vecinos. Por otra parte, en el reordenamien-
to cartesiano sobre kernel esf´erico ocurre lo contrario, se abre esa distancia a
medida que aumenta el radio de b´usqueda. A´un as´ı, esta degradaci´on es muy
leve y contin´uan siendo podas muy efectivas (no se supera el 25 % de diferencia
entre puntos recorridos y aceptados en ning´un caso).

5.2. Rendimiento de b´usquedas de vecinos paralelas

con centros aleatorios

Estos experimentos exploran la eficiencia temporal en la ejecuci´on de un lote
de 10.000 consultas espaciales con centros distribuidos aleatoriamente. Cada
configuraci´on se repiti´o 5 veces, tomando como m´etrica el tiempo medio de
ejecuci´on. Cabe destacar que se emple´o una paralelizaci´on a escala de 40 n´ucleos
en todas las ejecuciones. Se usaron todas las nubes de puntos descritas en el
apartado 3.4, variando los siguientes hiperpar´ametros:

Modo de reordenamiento. Se probaron tanto los dos modos de poda
propios como la versi´on de b´usqueda sobre un Octree lineal base propuesta
por Vi˜nambres et al [17].

Algoritmo de b´usqueda. Se sometieron a pruebas los dos m´etodos es-
tudiados y modificados: neighborsPrune y neighborsStruct .

Kernel. Los dos kernels tridimensionales soportados: cubo y esfera.

Algoritmos de SFC. Se usaron tanto los c´odigos de Morton como los
de Hilbert. Sin embargo, no se analizaron los resultados por separado,
eso est´a fuera del alcance del trabajo. Se incluyeron para promediar los
resultados con los dos encoders y tener una visi´on m´as contrastada de los
resultados.

Radio. Se dividieron las nubes en tres grupos: de densidad alta, media y
baja, seg´un la naturaleza de la nube. Para cada grupo se ejecut´o una se-
lecci´on de 8 radios distintos adaptada a la cantidad de vecinos aproximada
que producir´an.

Tama˜no de hoja (n´umero m´aximo de puntos). Este par´ametro es de
los m´as cr´ıticos para este trabajo. Puede influir directamente en el rendi-
miento temporal de la poda. Se probaron valores de 128, 256, 512, 768, 1024

Umbral de poda (threshold). Par´ametro dedicado de esta optimiza-
ci´on. Puede presentar cambios en el rendimiento, midiendo en qu´e punto
deja de ser rentable o no el algoritmo de poda. Se probaron los valores
16, 64, 100.

5.2. RENDIMIENTO DE B ´USQUEDAS DE VECINOS PARALELAS CON CENTROS ALEATORIOS33

(a) Modo de reordenamiento polar

(b) Modo de reordenamiento cartesiano

Figura 5.1: Efectividad te´orica de cada modo de poda sobre el dataset Lille 0,
sobre kernels c´ubicos y esf´ericos de diferentes tama˜nos, usando un tama˜no m´axi-
mo de hoja de 128 puntos.

34

CAP´ITULO 5. PRUEBAS

A continuaci´on, se muestran los resultados obtenidos. Para empezar, se vi-
sualiza el tiempo de ejecuci´on medio, dado un radio y una geometr´ıa de b´usque-
da, de cada combinaci´on de algoritmo y reordenaci´on local para los diferentes
tama˜nos de hoja propuestos. En las Figuras 5.2 y 5.3 se ven los resultados
variando el radio y la nube de puntos usada.

Figura 5.2: Tiempos medios de b´usqueda de vecindades de 10.000 puntos alea-
torios tomando diferentes radios en 5085 54320

Tal y como se evidencia en las Figuras 5.2 y 5.3 , los m´ınimos absolutos de
tiempo se alcanzan de manera consistente al configurar un tama˜no de nodo hoja
fijado en 256. Asimismo, destaca la superioridad del algoritmo neighborsStruct.
Aislando sus dos versiones, las implementaciones que incorporan reordenaciones
locales y selectores de rango reportan una aceleraci´on notable del tiempo de
ejecuci´on respecto al algoritmo original del Octree lineal, en especial en kernels
esf´ericos.

En cuanto a los resultados aislados de neighborsPrune, en general est´an
alejados, pero tanto el reordenamiento polar como el cartesiano tienen un des-
empe˜no similar, en ciertos casos mejor, al algoritmo base. Compar´andolos entre
ellos, ninguno de los dos reordenamientos aplicados a neighborsPrune destaca
claramente sobre el otro.

Una tendencia que s´ı se puede apreciar es que los reordenamientos mantienen
el rendimiento de forma m´as estable a medida que se disparan los tama˜nos
de las hojas, contrastando con la degradaci´on en los tiempos que sufren las
implementaciones originales.

Seguidamente, se analiza el impacto combinado de las dos variables de con-
trol cr´ıticas en el rendimiento de la poda: el tama˜no m´aximo de nodo hoja

5.2. RENDIMIENTO DE B ´USQUEDAS DE VECINOS PARALELAS CON CENTROS ALEATORIOS35

Figura 5.3: Tiempos medios de b´usqueda de vecindades de 10.000 puntos alea-
torios tomando diferentes radios en bildstein station1

36

CAP´ITULO 5. PRUEBAS

(maxLeaf) frente al umbral de activaci´on del selector (threshold). Para aislar
el efecto del resto de par´ametros, ambas variables se contraponen en una matriz
bidimensional.

El valor indexado en cada celda de la matriz corresponde a la media aritm´eti-
ca del speedup (S) obtenido para el conjunto de radios evaluados. Para evitar
que los radios con mayores tiempos absolutos sesguen el mapa de calor, el spee-
dup de cada radio r se normaliza dividiendo el tiempo de la configuraci´on actual
entre el tiempo m´ınimo absoluto registrado para ese mismo radio entre todo el
espectro de hiperpar´ametros testados. Finalmente, este indicador se promedia
combinando los resultados de ambos kernels de b´usqueda (cubo y esfera), res-
pondiendo a la siguiente expresi´on:

Smedio =

1
|K| · |R|

(cid:88)

(cid:88)

k∈K

r∈R

Toptimo(k)
T (k, r, maxLeaf, threshold, algoritmo)

(5.1)

Figura 5.4: An´alisis del rendimiento temporal de hiperpar´ametros en las nubes
de puntos Mar18Train y Mar18Test

Debido a que la reordenaci´on polar en conjunto con las b´usquedas mediante
neighborsStruct fue la m´as eficiente, ser´a esa configuraci´on en la que se centrar´a
el estudio de hiperpar´ametros. Como se puede apreciar en la Figura 5.4, la
combinaci´on m´as eficiente para el comportamiento del Octree en las nubes del
dataset Hessigheim 3D se produce con hojas de 256 puntos y con un umbral
de activaci´on de poda (threshold) fijado en 16 puntos.

No obstante, distintas distribuciones de puntos en cada dataset provocan
que las combinaciones ´optimas presenten ligeras variaciones seg´un la nube de
puntos analizada. Por este motivo, se evalu´o de forma independiente qu´e par
de hiperpar´ametros favorec´ıa el rendimiento de cada entorno espec´ıfico.

Estas configuraciones ´optimas personalizadas constituyen las bases de datos
con las que se alimentaron los resultados a continuaci´on expuestos. En dichos
gr´aficos se cuantifica el tiempo medio de ejecuci´on de las 10.000 consultas espa-
ciales a medida que se incrementa el radio de b´usqueda de forma continua, dis-
tribuyendo cada serie como una combinaci´on ´unica de algoritmo de asignaci´on
y modo de reordenamiento local. Se pueden ver los resultados para diferentes
nubes de puntos en las Figuras 5.5 y 5.6.

5.3. RENDIMIENTO DE B ´USQUEDAS DE VECINOS PARALELAS SOBRE TODA LA NUBE37

Estos valores temporales muestran una continuaci´on de los resultados pro-
puestos en las Figuras 5.2 y 5.3, pero pudi´endose apreciar tendencias con la
introducci´on de la variaci´on de radio y present´andose, adem´as, comportamien-
tos disjuntos entre nubes. En en el dataset Mar18 Train los tiempos siguen
una distribuci´on notablemente uniforme. Por otro lado, los valores asociados
al algoritmo neighborsStruct con poda local son los que dominan en cuanto a
eficiencia temporal. A´un as´ı, las podas con el algoritmo de pruning b´asico (des-
tacando el modo cartesiano) presentan un rendimiento equiparable en radios
intermedios. Eso s´ı, en ambos datasets estas ejecuciones se degradan en gran
medida al escalar los tama˜nos de las figuras geom´etricas de b´usqueda.

Figura 5.5: Tiempos medios de b´usqueda de vecindades de 10.000 puntos alea-
torios para diferentes radios en la nube Mar18 Train

Figura 5.6: Tiempos medios de b´usqueda de vecindades de 10.000 puntos alea-
torios para diferentes radios en la nube Mar18 Test

5.3. Rendimiento de b´usquedas de vecinos paralelas

sobre toda la nube

A continuaci´on, se trata un caso de uso muy com´un en el manejo de nubes
de puntos 3D. Estos experimentos eval´uan el comportamiento del sistema an-

38

CAP´ITULO 5. PRUEBAS

te b´usquedas masivas secuenciales sobre la totalidad de la nube. Debido a la
naturaleza del recorrido y el reparto de carga contiguo por hilos, es necesario
explorar las din´amicas de estas consultas de forma independiente. Ya que es-
tas ejecuciones son muy costosas, se seleccionaron un subconjunto de las nubes
preparadas, manteniendo variedad entre LiDAR terrestre y a´ereo provenien-
tes de diversos datasets. Tambi´en se mantuvieron fijos los hiperpar´ametros de
maxLeaf y threshold. Se escogi´o para cada conjunto de nubes un par de valores
para estas variables de control que fue exitosos en los experimentos de centros
aleatorios:

Perfil de alta densidad: Nube bildstein station1. Configuraci´on: [256,
64]

Perfil de densidad media: Nubes Mar18 train y Paris Luxembourg 6.
Configuraci´on: [128, 64]

Perfil de baja densidad: Nubes PNOA 2024 PNR 489-4672 NPC01 y
5085 54320. Configuraci´on: [256, 64]

Adicionalmente, se fij´o el algoritmo SFC a los c´odigos de Hilbert y se fij´o

una ´unica repetici´on de las b´usquedas.

En la Figura 5.7 se exploran todos los resultados conseguidos.
Estas Figuras muestran un patr´on claro entre ellas, a la vez que difieren de
los resultados de b´usquedas en subconjuntos. El algoritmo neighborsStruct se
mantiene como la mejor implementaci´on. El rendimiento de los algoritmos con
reordenamientos, para ambos kernels, es parejo pero ligeramente peor a lo largo
de los datasets. Las dos formas de poda local existentes en neighborsPrune tam-
bi´en funcionan de forma similar. Var´ıan ligeramente entre kernels, destacando
la forma cartesiana algo m´as en los kernels c´ubicos, pero sin producir una ten-
dencia muy clara. De forma general, se puede afirmar que las reordenaciones
tienen menos impacto en b´usquedas completas secuenciales que en consultas
aleatorias.

5.4. Eficiencia del paralelismo sobre b´usquedas de

vecinos

De forma complementaria, se estudi´o el rendimiento al modificar el n´umero
de hilos operando de forma paralela en las b´usquedas. Para aislar lo m´aximo
posible la escalabilidad de hilos se dejaron fijos los siguientes par´ametros:

Kernel - esf´erico

Codificador - Hilbert

Tama˜no de hoja - 256

Umbral de poda - 64

Algoritmo - neighborsPrune

5.4. EFICIENCIA DEL PARALELISMO SOBRE B ´USQUEDAS DE VECINOS39

(a) Dataset 5085 54320

(b) Dataset PNOA 2024 PNR 489-4672 NPC01

(c) Dataset Mar18 train

40

CAP´ITULO 5. PRUEBAS

(d) Dataset Paris Luxembourg 6

(e) Dataset bildstein station1

Figura 5.7: Tiempos de ejecuci´on de b´usquedas completas variando el radio
operativo sobre kernels esf´ericos y c´ubicos.

5.4. EFICIENCIA DEL PARALELISMO SOBRE B ´USQUEDAS DE VECINOS41

Cabe comentar la decisi´on de aislar el algoritmo. Se hizo as´ı porque es el
´unico que incluye los dos reordenamientos implementados y las diferencias de
almacenamiento de vecinos no supone un factor cr´ıtico en materia de parale-
lizaci´on. Por otro lado, s´ı se introdujo un conjunto de tres tama˜nos de kernel
en cada caso para estudiar el impacto a medida que crece el n´umero de hojas
visitadas y vecinos encontrados.

Las pruebas de escalabilidad, al ser ejecutadas en nodos de c´omputo del
CESGA, son realizadas sobre nodos de memoria NUMA (Non-Uniform Me-
mory Access). En esta arquitectura, la latencia de acceso viene definida por la
proximidad f´ısica entre el n´ucleo de la CPU que ejecuta el hilo de OpenMP y
el banco de memoria donde residen los datos del Octree.

Para mitigar el sesgo introducido por la asignaci´on aleatoria de p´aginas de
memoria por parte del sistema operativo, todas las pruebas de paralelismo se
confinaron utilizando la herramienta numactl [11].

Espec´ıficamente, se aplic´o la directiva --interleave=all. Esta configura-
ci´on fuerza un intercalado uniforme de las p´aginas de memoria virtual emplea-
das a lo largo de todos los nodos NUMA disponibles. As´ı, se distribuyen entre
los dos sockets del nodo de computaci´on las p´aginas correspondientes a la es-
tructura del Octree, las r´eplicas de puntos reordenadas y el resto de variables
con memoria reservada en tiempo de ejecuci´on. Esto elimina el sesgo al variar el
n´umero de hilos, de forma que siempre se reparta la memoria de forma uniforme
y equitativa, optimizando adem´as el ancho de banda al no sobrecargar ninguno
de los dos sockets por nodo en exceso.

Los valores de eficiencia en la paralelizaci´on se calcularon a partir del tiempo
de ejecuci´on usando un solo n´ucleo de procesamiento, mediante la siguiente
f´ormula:

Ef icienciaN proc =

T1
TN proc ∗ N proc

(5.2)

Al igual que el estudio del rendimiento temporal, se separar´an los resultados
entre b´usquedas de subconjuntos aleatorias y b´usquedas completas sobre la
nube.

En los mapas de calor de la Figura 5.8 se muestra la variaci´on de la escala-
bilidad para el dataset Paris Luxembourg 6 para 10.000 b´usquedas de centros
aleatorios. Se pueden observar valores similares en los 3 heatmaps, manteni´endo-
se una eficiencia casi ideal para paralelizaci´on de hasta 8 hilos. A partir de 16
n´ucleos trabajando de forma paralela, existe una degradaci´on, bajando la efi-
ciencia del 90 % y llegando a tomar valores de entre 0.4 y 0.6 para paralelizaci´on
de 32 y 40 CPUs. Para realizar una comparativa entre los reordenamientos, las
diferencias de rendimiento se hacen m´as palpables cu´antos m´as hilos se est´en
usando y cu´anto m´as r´apida sea la ejecuci´on (radios m´as cortos). En estos casos
el pocentaje del tiempo gastado en el manejo del paralelismo es naturalmente
mayor al ser m´as breves las operaciones en s´ı. En esa secci´on superior derecha
de las matrices, la b´usqueda base presenta un peor comportamiento que los
algoritmos con poda implementada.

Para terminar, se exponen en la Figura 5.9 los mapas de calor para el da-
taset Lille 0, sobre el que se realizaron b´usquedas completas para los distintos
reordenamientos. En estos heatmaps se aprecia una paralelizaci´on muy potente,

42

CAP´ITULO 5. PRUEBAS

(a) Algoritmo de b´usqueda de Octree lineal base

(b) Reordenamiento de las hojas del Octree en modo polar

(c) Reordenamiento de las hojas del Octree en modo cartesiano

Figura 5.8: Mapas de calor de eficiencia de paralelizaci´on sobre centros aleatorios
en la nube de puntos Paris Luxembourg 6

5.4. EFICIENCIA DEL PARALELISMO SOBRE B ´USQUEDAS DE VECINOS43

solamente bajando del 90 % de eficiencia en los casos m´as extremos, con m´as
de 32 n´ucleos de procesamiento paralelos y radios cortos. De nuevo, se debe
poner el foco en la zona superior derecha para observar los resultados m´as co-
rrelacionados con el funcionamiento general de la escalabilidad. Al igual que en
las b´usquedas aleatorias, las ejecuciones sin poda a nivel de hoja tienen peor
eficiencia temporal en los casos m´as cr´ıticos de paralelizaci´on, sobre un 5 %
m´as bajo que en los modos del reordenamiento polar y cartesiano, que tienen
comportamientos casi id´enticos entre s´ı.

44

CAP´ITULO 5. PRUEBAS

(a) Algoritmo de b´usqueda de Octree lineal base

(b) Reordenamiento de las hojas del Octree en modo polar

(c) Reordenamiento de las hojas del Octree en modo cartesiano

Figura 5.9: Mapas de calor de eficiencia de paralelizaci´on sobre coberturas com-
pletas en la nube de puntos Lille 0

Cap´ıtulo 6

Discusi´on de los Resultados

En este cap´ıtulo se someten a un an´alisis cr´ıtico los resultados emp´ıricos
expuestos en el cap´ıtulo previo. El prop´osito fundamental es evaluar la eficiencia,
viabilidad y ´exito del dise˜no e implementaci´on de las t´ecnicas de poda a nivel
de nodo hoja sobre la arquitectura del Octree lineal en relaci´on a los resultados
y soluciones existentes a d´ıa de hoy para la b´usqueda de vecindades en nubes
de puntos capturadas con tecnolog´ıa LiDAR.

6.1. An´alisis de Estructuras de Almacenamiento: al-
goritmo neighborsStruct frente a neighborsPrune

Como tendencia independiente de las contribuciones directas de este traba-
jo, se constata que la arquitectura de almacenamiento de puntos vecinos basado
en pares de ´ındices, (neighborsStruct), reporta de manera sistem´atica una efi-
ciencia temporal superior a las inserciones secuenciales (presentes en neighbors-
Prune). Este comportamiento se explica por la reducci´on de inserciones en el
vector ptsInside realizadas, diferencia que se acent´ua m´as cuantos m´as puntos
consecutivos sean considerados v´alidos.

Sin embargo, el hallazgo m´as relevante de esta comparativa radica en el
comportamiento asim´etrico que manifiesta el algoritmo de poda polar. Esta op-
timizaci´on exhibe un impacto sustancialmente mayor al acoplarse sobre neigh-
borsStruct que al operar sobre neighborsPrune. La hip´otesis que lo justifica
reside en el modo de inserci´on en el vector de resultados:

En la variante b´asica neighborsPrune, el algoritmo eval´ua secuencialmente
los puntos y a˜nade un elemento al contenedor de salida de forma indivi-
dualizada si, y solo si, se verifica la condici´on de vecindad positiva. Bajo
este esquema, la poda angular act´ua ´unicamente reduciendo el coste de la
evaluaci´on booleana interna (k.isInside()), dado que todos los puntos
descartados habr´ıan devuelto un valor falso en cualquier caso. El ahorro
computacional queda limitado a la supresi´on de la comprobaci´on aritm´eti-
ca de isInside().

Por el contrario, en neighborsStruct la l´ogica opera por bloques de rangos
contiguos mediante la guarda condicional if (!k.isInside(points[i])).

45

46

CAP´ITULO 6. DISCUSI ´ON DE LOS RESULTADOS

Si un punto no pertenece al entorno, se introduce en el contenedor el ran-
go completo indexado previamente. Al efectuar la poda a nivel de hoja,
cuanto m´as efectiva sea la discriminaci´on geom´etrica, menor ser´a el n´ume-
ro de ocasiones en las que el flujo de ejecuci´on entra en esta bifurcaci´on,
reduciendo masivamente las operaciones de inserci´on y de redimensionado
del vector de salida.

Esta divergencia es particularmente visible bajo kernels esf´ericos, ya que es
la forma geom´etrica adaptada al c´alculo de poda angular que se realiza en el
modo polar. Aparte, en nubes de puntos con una cierta densidad espacial (tales
como Lille 0, bildstein station1, sg27 station8 o Mar18 test), se aprecia que a
medida que se incrementa el radio operativo, el n´umero de puntos consecuti-
vos que cumplen con el criterio de inclusi´on se dispara. En estas regiones de
alta saturaci´on, el algoritmo de pares de ´ındices de neighborsStruct ampl´ıa la
diferencia proporcionalmente en rendimiento contra el pruning est´andar.

6.2. Sensibilidad al Tama˜no de Hoja y Din´amicas del

Kernel

Un hito de la arquitectura propuesta es su capacidad para romper la degra-
daci´on del rendimiento asociada al crecimiento del tama˜no de los nodos hoja
(maxLeaf). En las implementaciones secuenciales cl´asicas, incrementar la capa-
cidad del nodo hoja implica obligatoriamente dilatar el tiempo de c´omputo, al
verse forzado el sistema a realizar un escaneo lineal sobre un mayor volumen de
puntos desordenados.

Con la introducci´on de los modos de reordenamiento local, el coste temporal
deja de depender directamente del n´umero absoluto de puntos contenidos en la
hoja. El rendimiento pasa a depender de la distribuci´on espacial y del porcen-
taje de descarte geom´etrico efectivo. Este porcentaje de descarte fue estudiado
con detalle en el cap´ıtulo anterior, y se pueden extraer una serie de apren-
dizajes sobre el funcionamiento te´orico, m´as all´a de los tiempos de ejecuci´on
producidos. El reordenamiento local polar funciona mejor en combinaci´on con
el modo de kernel esf´erico por la concordancia geom´etrica existente, mientras
que para el caso de los cubos hay que trazar una esfera que lo circunscriba para
poder hallar esos l´ımites. Adem´as, tambi´en se entiende la mayor efectividad de
poda en radios bajos. La secci´on angular m´ınima que contiene a la esfera de
b´usqueda es m´as cerrada cu´anto m´as peque˜na sea esa esfera. Esta tendencia
esperada s´ı se corresponde con los resultados del algoritmo neighborsStruct en
las Figuras 5.5 y 5.6. De forma complementaria, la efectividad de la poda en
los experimentos estudiados para el reordenamiento cartesiano tuvo ´exito en las
b´usquedas de geometr´ıa c´ubica, al separar perfectamente los puntos seg´un los
ejes ordenados. En estos casos, se hallaba una din´amica contrapuesta al variar
el tama˜no del radio: las podas son m´as d´ebiles en radios peque˜nos. Esto tiene
una explicaci´on geom´etrica detr´as, y es que en b´usquedas c´ubicas, si el kernel
solo corta la hoja en una dimensi´on la separaci´on de puntos entre vecinos y no
vecinos ser´a perfecta. Solo en los casos en los que dos o tres dimensiones cortan

6.3. EVALUACI ´ON DE LOS MODOS DE REORDENAMIENTO LOCAL47

el nodo se producir´an podas imperfectas. En cubos grandes (proporcionalmente
a los tama˜nos de los nodos terminales) m´as octantes tendr´an la cualidad de ser
cortados solamente por una dimensi´on, con lo que m´as puntos ser´an separados
de forma perfecta. Sin embargo, este comportamiento no se aprecia directamen-
te en los experimentos realizados usando el algoritmo neighborsPrune, y tiene
una explicaci´on que se introducir´a a continuaci´on.

6.3. Evaluaci´on de los Modos de Reordenamiento

Local

Al contrastar de manera aislada los dos modos de reordenamiento imple-
mentados (polar y cartesiano) frente al algoritmo de b´usqueda con inserciones
secuenciales, las m´etricas reflejan diferencias muy sutiles. Esto se debe a que
existe un beneficio extra en que se inserten una serie de puntos consecutivos,
como s´ı pasa al usar neighborsStruct. A pesar de ello, la introducci´on de la
poda logra amortizar con ´exito su propia sobrecarga algor´ıtmica, manteniendo
registros competitivos.

Te´oricamente, cabr´ıa esperar una divergencia de rendimiento m´as acusada
entre ambas reordenaciones al cruzar sus kernels respectivos, dado que cada
una minimiza la sobreestimaci´on del volumen envolvente en su propio dominio
geom´etrico. Sin embargo, la mejora temporal respecto a la versi´on secuencial
es tan peque˜na que el ahorro funcional apenas sobresale respecto al coste fi-
jo de computar los l´ımites de la hoja. Hay casos concretos en los que, tanto
el modo polar como el cartesiano, ganan rendimiento al ser acoplados sobre
neighborsPrune, pero no de forma consistente ni con una tendencia clara.

En este contexto en el que la sobrecarga pr´acticamente alcanza al ahorro
conseguido mediante la poda, los escenarios cruzados (modo cartesiano sobre
kernels esf´ericos y modo polar sobre kernels c´ubicos) operan como un esta-
do intermedio que tampoco degrada apenas el rendimiento. En estos casos, la
sobreestimaci´on geom´etrica del volumen impide que la mayor´ıa de las hojas ca-
lifiquen para un descarte. No obstante, estas hojas no aptas evitan la ejecuci´on
de la b´usqueda binaria y se desv´ıan de forma inmediata hacia el recorrido se-
cuencial ordinario. El software no penaliza de forma cr´ıtica estas configuraciones
sub´optimas de entrada, garantizando la estabilidad del rendimiento.

6.4. Calibraci´on del Espacio de Hiperpar´ametros

Los ensayos demuestran que el rendimiento de las consultas espaciales es
altamente sensible al ecosistema de par´ametros de control, condicionado de
manera simult´anea por la densidad de la nube, la magnitud del radio y la
posici´on espacial del centro de consulta.

A pesar de esta volatilidad, es posible extraer una regla de dise˜no genera-
lizada: la configuraci´on de tama˜nos de nodo hoja superiores a maxLeaf = 256
introduce un principio de degradaci´on en la eficiencia temporal. Esta p´erdida de
rendimiento, cabe destacar, es significativamente atenuada en comparaci´on con
la bajada de rendimiento en los algoritmos cl´asicos. A´un as´ı, delimita el umbral

48

CAP´ITULO 6. DISCUSI ´ON DE LOS RESULTADOS

que no se deber´ıa superar a menos que se est´e trabajando sobre un entorno ya
conocido y estudiado.

En lo que respecta al umbral m´ınimo de puntos para la activaci´on de la
poda (threshold), el valor de 64 puntos se consolida como el punto de equili-
brio ´optimo. L´ımites inferiores introducen un overhead ineficiente en la CPU al
forzar el c´alculo de selectores de rango para subconjuntos de puntos min´usculos
cuya evaluaci´on secuencial directa en memoria ser´ıa m´as veloz. Por el contrario,
l´ımites excesivamente elevados clausuran ventanas de oportunidad de la poda
en hojas medianas que albergan buen potencial de descarte.

6.5. Din´amicas en B´usquedas de Cobertura Comple-

ta

El escenario de evaluaci´on sobre b´usquedas completas y secuenciales (Full
Searches) proporcion´o un escenario de resultados muy dispar a los discutidos
hasta ahora. En este modo de exploraci´on, la tasa de fallos de cach´e se reduce
al m´ınimo debido a la asignaci´on por bloques de puntos a cada hilo OpenMP.
Al procesarse n´ucleos de consulta vecinos de forma consecutiva, hay una gran
tasa de reutilizaci´on de puntos en los niveles superiores de la memoria cach´e
de la CPU tras las iteraciones previas. Este fen´omeno se amplifica teniendo en
cuenta que los puntos fueron reordenados de forma global mediante las curvas de
Hilbert para mejorar su localidad. Como consecuencia directa de esta localidad
hardware, el coste temporal de evaluar secuencialmente un conjunto de puntos
que potencialmente se iban a descartar es extremadamente bajo.

Por ende, dado que el coste de c´alculo del overhead para la selecci´on de ran-
gos geom´etricos se mantiene constante, el balance de rentabilidad de la poda es
negativo. Los tiempos de ejecuci´on totales, aunque no suponen una degradaci´on
notable, se sit´uan ligeramente por detr´as que las b´usquedas con los algoritmos
originales. En este contexto de coberturas completas, no se considera ´util la
implementaci´on actual de reordenamientos locales, ya que no produce un ren-
dimiento consistentemente mejor al c´odigo de partida a la vez que otorga a los
Octrees un recorrido de b´usqueda m´as complejo y produce un mayor gasto en
memoria.

Cap´ıtulo 7

Conclusi´ons e posibles
ampliaci´ons

7.1. Principales aportaciones y limitaciones

Tras el an´alisis cr´ıtico de los experimentos ejecutados, se presenta a con-
tinuaci´on una s´ıntesis de las principales conclusiones y aportaciones t´ecnicas
derivadas del dise˜no e implementaci´on del sistema:

Sinergia de los reordenamientos con las estructuras de almace-
namiento: La poda local act´ua de forma dependiente con la estrategia
de gesti´on de memoria. Manifest´o una ganancia cr´ıtica al acoplarse al al-
goritmo orientado a bloques (neighborsStruct), ya que la discriminaci´on
geom´etrica se especializa en agrupar puntos vecinos y reduce las bifur-
caciones condicionales y las operaciones de inserci´on. Esto provoca que
el reordenamiento cartesiano no produce resultados favorables al no po-
der acoplarse naturalmente a este sistema de inserciones en bloque, al
contrario que el reordenamiento propuesto mediante el ´angulo φ.

Especializaci´on y din´amica dimensional de los kernels: Se com-
prueba que la efectividad te´orica del descarte est´a ligada a la morfolog´ıa
de la consulta. El modo polar maximiza su rendimiento en kernels esf´eri-
cos (especialmente a radios bajos debido al estrechamiento de la secci´on
angular), mientras que el modo cartesiano destaca en kernels c´ubicos (par-
ticularmente a radios mayores, donde los cortes unidimensionales con las
caras del nodo permiten una separaci´on perfecta entre puntos vecinos y
no vecinos).

Delimitaci´on del espacio de hiperpar´ametros: A pesar de la varia-
bilidad y la naturaleza ´unica de cada nube LiDAR, se consolida un umbral
operativo ´optimo fijando el threshold en 64 puntos, cota que evita intro-
ducir un overhead ineficiente en la CPU ante subconjuntos min´usculos.
Asimismo, se establece que valores superiores a maxLeaf = 256 marcan el
inicio de la degradaci´on de la eficiencia, partiendo el espacio en octantes
no lo suficientemente concretos. A´un as´ı, esta implementaci´on supone una

49

50

CAP´ITULO 7. CONCLUSI ´ONS E POSIBLES AMPLIACI ´ONS

ganancia en flexibilidad a la hora de dise˜nar el tama˜no de los octantes del
Octree.

Inversi´on de rentabilidad en coberturas completas: Se identifica
una limitaci´on f´ısica del algoritmo en las b´usquedas completas. La or-
denaci´on global por curvas de Hilbert combinada con la asignaci´on por
bloques de OpenMP maximiza de forma nativa la reutilizaci´on de datos
en la memoria cach´e de la CPU y el tiempo ahorrado con los descartes de
puntos no llega a equilibrar la sobrecarga introducida.

A partir de estas premisas, se puede confirmar la mejora real introducida en
este trabajo mediante la propuesta del reordenamiento basado en coordenadas
esf´ericas integrado sobre el algoritmo neighborsStruct. Para poner en el contexto
de la literatura t´ecnica esta implementaci´on, y tal como se abord´o en el estado
del arte, se tomaron como referencia de rendimiento los resultados del Octree
lineal publicados por Vi˜nambres et al. [17]. Dicha estructura base minimiza-
ba los tiempos de ejecuci´on en relaci´on con las principales implementaciones
abiertas de Octrees, tales como PCL, PicoTree, NanoFLANN y unibnOctree.

Por consiguiente, la integraci´on original de la t´ecnica de poda a nivel de
hoja desarrollada en este proyecto sobre la arquitectura de Vi˜nambres et al.
supone, hasta donde se conoce y bajo el entorno operativo adecuado (consul-
tas individuales/aleatorias con kernels esf´ericos), la soluci´on m´as eficiente en la
actualidad para detectar y almacenar vecinos sobre nubes de puntos tridimen-
sionales LiDAR.

7.2. V´ıas de Mejora y Trabajo Futuro

Por ´ultimo, como una ampliaci´on a futuro, se propone de desarrollo el dise˜no
de un mecanismo que permita acoplar la b´usqueda de rangos cartesiana directa-
mente sobre la infraestructura orientada a datos contiguos de neighborsStruct.
Esta integraci´on tiene como objetivo desbloquear el gran potencial de la
poda por ejes cartesianos sobre geometr´ıa c´ubicas, visible en los porcentajes de
puntos podados conseguidos, cuyo impacto no pudo transformarse en eficien-
cia temporal real. El motivo es la incompatibilidad matem´atica del algoritmo
neighborsStruct base con los m´ultiples reordenamientos simult´aneos en memo-
ria. En caso de consolidar esta integraci´on estructural, se elevar´ıa dr´asticamente
la rapidez temporal de las consultas sobre kernels c´ubicos.

Te´oricamente, esta eficiencia ser´ıa incluso de mayor magnitud que la encon-
trada para kernels esf´ericos mediante la poda angular. Esto es debido a que la
efectividad geom´etrica del descarte cartesiano es sustancialmente m´as alta: tal
y como se constat´o en las m´etricas de depuraci´on, apenas existen puntos evalua-
dos por el algoritmo que no terminen cumpliendo el criterio de vecindad. Esto
implica que el sistema har´ıa todav´ıa menos operaciones de c´omputo de vecinos
por hoja. Esto constituye, sin duda, la l´ınea de investigaci´on m´as prometedora
para la continuidad de este trabajo.

Ap´endice A

Manuais t´ecnicos

En funci´on do tipo de Traballo e metodolox´ıa empregada, o contido poderase
dividir en varios documentos. En todo caso, neles incluirase toda a informaci´on
precisa para aquelas persoas que desexen repetir o experimento (por exemplo
c´odigo fonte, recursos necesarios, operaci´ons necesarias para modificaci´ons e
probas, posibles problemas, etc.). O c´odigo fonte poderase entregar en soporte
inform´atico en formatos PDF ou postscript.

51

52

AP ´ENDICE A. MANUAIS T ´ECNICOS

Ap´endice B

Manuais de usuario

Incluir´an toda a informaci´on precisa para aquelas persoas que repliquen o
experimento: instalaci´on, utilizaci´on, configuraci´on, mensaxes de erro, etc. A
documentaci´on do usuario debe ser autocontida, ´e dicir, para o seu entende-
mento o usuario final non debe precisar da lectura doutro manual t´ecnico.

53

54

AP ´ENDICE B. MANUAIS DE USUARIO

Bibliograf´ıa

[1] Tetsuo Asano, Desh Ranjan, Thomas Roos, Emo Welzl, and Peter Wid-
mayer. Space-filling curves and their use in the design of geometric data
structures. Theoretical Computer Science, 181(1):3–15, 1997.

[2] Jens Behley, Volker Steinhage, and Armin B. Cremers. Efficient radius
neighbor search in three-dimensional point clouds. In 2015 IEEE Interna-
tional Conference on Robotics and Automation (ICRA), pages 3625–3630.
IEEE, 2015.

[3] Jose Luis Blanco and Pranjal Kumar Rai. nanoflann: a C++ header-only
fork of FLANN, a library for nearest neighbor (NN) with kd-trees. https:
//github.com/jlblancoc/nanoflann. Accedido: 2 de junio de 2026.

[4] Jonathan Broere. Picotree: A C++ header-only library for nearest
neighbor search and multidimensional KD-trees. https://github.com/
Jaybro/pico_tree. Accedido: 2 de junio de 2026.

[5] Centro de Supercomputaci´on de Galicia (CESGA). Finisterrae III user gui-
de: Overview of node groups description. https://cesga-docs.gitlab.
io/ft3-user-guide/overview.html#node-groups-description. Acce-
dido: 8 de junio de 2026.

[6] cppreference.com.

https://es.
cppreference.com/cpp/chrono/high_resolution_clock. Accedido: 20
de mayo de 2026.

std::chrono::high resolution clock.

[7] cppreference.com.

std::sort.

https://en.cppreference.com/cpp/

algorithm/sort. Accedido: 18 de mayo de 2026.

[8] Timo Hackel, N. Savinov, L. Ladicky, Jan D. Wegner, K. Schindler, and
M. Pollefeys. SEMANTIC3D.NET: A new large-scale point cloud clas-
sification benchmark. In ISPRS Annals of the Photogrammetry, Remote
Sensing and Spatial Information Sciences, volume IV-1-W1, pages 91–98,
2017.

[9] Instituto Geogr´afico Nacional (IGN). Presentaci´on del proyecto PNOA-
LiDAR para el escaneo del territorio nacional. https://pnoa.ign.es/
web/portal/pnoa-lidar/presentacion. Accedido: 4 de mayo de 2026.

55

56

BIBLIOGRAF´IA

[10] Michael K¨olle, Dominik Laupheimer, Stefan Schmohl, Norbert Haala,
Franz Rottensteiner, Jan Dirk Wegner, and Hugo Ledoux. The hessigheim
3d (h3d) benchmark on semantic segmentation of high-resolution 3d point
clouds and textured meshes from uav lidar and multi-view-stereo. ISPRS
Open Journal of Photogrammetry and Remote Sensing, 1:11, 2021.

[11] Linux Manual Page. numactl(8) - control NUMA policy for processes or
shared memory. https://linux.die.net/man/8/numactl. Accedido: 3
de junio de 2026.

[12] Marius Muja and David G. Lowe. Flann: A library for performing fast
approximate nearest neighbor searches in high dimensional spaces. IEEE
Transactions on Pattern Analysis and Machine Intelligence, 36(11):2229–
2240, 2014. Software disponible en https://www.cs.ubc.ca/research/
flann/.

[13] OpenMP Architecture Review Board. The OpenMP API specifications
for parallel programming. https://www.openmp.org/specifications/.
Accedido: 7 de junio de 2026.

[14] Xavier Roynard, Jean-Emmanuel Deschaud, and Fran¸cois Goulette. Paris-
lille-3d: A large and high-quality ground-truth urban point cloud dataset
for automatic segmentation and classification. The International Journal
of Robotics Research, 37(6):545–557, 2018.

[15] Radu Bogdan Rusu and Steve Cousins. 3D is here: Point Cloud Library
In IEEE International Conference on Robotics and Automation

(PCL).
(ICRA), Shanghai, China, May 9-13 2011. IEEE.

[16] Nina Varney, Vijayan K. Asari, and Quinn Graehling. DALES: A large-
scale aerial lidar data set for semantic segmentation.
In Proceedings of
the IEEE/CVF Conference on Computer Vision and Pattern Recognition
Workshops, pages 186–187, 2020.

[17] Pablo D. Vi˜nambres, Miguel Yermo, Silvia R. Alcaraz, Oscar G. Lorenzo,
Francisco F. Rivera, and Jos´e C. Cabaleiro. Efficient neighbourhood search
in 3d point clouds through space-filling curves and linear octrees. arXiv
preprint arXiv:2603.06771, 2026.

[18] Wikipedia. Coordenadas esf´ericas. https://es.wikipedia.org/wiki/

Coordenadas_esf%C3%A9ricas. Accedido: 30 de mayo de 2026.

[19] Wikipedia. Curva de llenado del espacio. https://es.wikipedia.org/

wiki/Curva_de_llenado_del_espacio. Accedido: 30 de mayo de 2026.

[20] Qian-Yi Zhou, Jaesik Park, and Vladlen Koltun. Open3D: A modern li-

brary for 3D data processing. arXiv:1801.09847, 2018.

