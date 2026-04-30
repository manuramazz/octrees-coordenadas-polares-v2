- DETECTAR POR QUE HAY CORE DUMPED
- HACER NUBE ENANA Y MIRAR QUE LOS RESULTADOS SE HAGAN BIEN

RESPUESTAS:
DONE Assert incorrecto que estaba frenando la ejecución sin decir nada
DONE El problema tiene que ser en range selector creo que en el método K0 (basicamente porque los otros no recortan apenas)
DONE Spherical y pointer van igual (otro motivo para pensar que es el método K0)
DONE Linear y pointer van igual (las funciones que exponen y la diferenciación está bien)

Casi no se están recortando puntos, por eso se pierde tanto tiempo, no se recupera en absoluto. 
No sé por qué pasa y no sé por qué los métodos K1 y K2 son tan irrelevantes. Mañana más...
23/04
Leaf 38: best order=0 count=12 / 17 AQUI SE PIERDEN PUNTOS: MIRAR A MANO SI LA DISTANCIA ES MENOR A 15

 No estoy gestionando los kernels 2D ARREGLAR
 precomputar cosas en las búsquedas
 k1 cylindrical muy malo

COSAS PARA HACER JUEVES 30/04
2- Pensar como quiero agrupar los datos para mostrarlos en las graficas (diferentes kernels,radios,datasets) hablarlo con gemini
3- Hacer opciones globales el degub de rangos y el debug de tiempo de loop
4- Mandar scripts al cesga 
5- Preparar notebook para visualizar resultados

6- Optimizaciones aparte: - intersecar rangos de los 3 métodos
                        - ¿hacer vectores de puntos en vez de perms para mantener localidad caché?