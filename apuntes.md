RESPUESTAS:
DONE Assert incorrecto que estaba frenando la ejecución sin decir nada
DONE El problema tiene que ser en range selector creo que en el método K0 (basicamente porque los otros no recortan apenas)
DONE Spherical y pointer van igual (otro motivo para pensar que es el método K0)
DONE Linear y pointer van igual (las funciones que exponen y la diferenciación está bien)


 No estoy gestionando los kernels 2D - no puedo cierto?

6- Optimizaciones: 
- intersecar rangos de los 3 métodos
- ¿hacer vectores de puntos en vez de perms para mantener localidad caché? -> mandatory probarlo porque ahora mismo aún con la poda el tiempo de bucle es mayor
- comentar tamaño hojas

Añadir a las gráficas: 
- relación L-R
- tamaño de las hojas

Luego hacer experimento modificando los parámetros hablados
