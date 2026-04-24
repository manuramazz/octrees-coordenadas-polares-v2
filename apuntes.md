- DETECTAR POR QUE HAY CORE DUMPED
- HACER NUBE ENANA Y MIRAR QUE LOS RESULTADOS SE HAGAN BIEN

RESPUESTAS:
Assert incorrecto que estaba frenando la ejecución sin decir nada
No estoy gestionando los kernels 2D ARREGLAR
El problema tiene que ser en range selector creo que en el método K0 (basicamente porque los otros no recortan apenas)
Spherical y pointer van igual (otro motivo para pensar que es el método K0)
Linear y pointer van igual (las funciones que exponen y la diferenciación está bien)

Casi no se están recortando puntos, por eso se pierde tanto tiempo, no se recupera en absoluto. 
No sé por qué pasa y no sé por qué los métodos K1 y K2 son tan irrelevantes. Mañana más...
23/04
Leaf 38: best order=0 count=12 / 17 AQUI SE PIERDEN PUNTOS: MIRAR A MANO SI LA DISTANCIA ES MENOR A 15