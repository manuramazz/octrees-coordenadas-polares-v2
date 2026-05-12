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



09/05: PLAN (implementar podas x,y,z juntas)
 - DONE refactorizar nombres de reordenaciones
 - DONE modificar buildLeafPermutations para soportar nuevas reorders
 - IN modificar range selector para optimizar búsquedas binarias (si se pasan del tamaño de la hoja O(1))

11/05 PLAN
- DONE revisar y optimizar computeRange
- DONE adaptar bestRange
- DONE revisar integración benchmark y linear
- DONE compilar
- DONE probar para tantear tiempos
- DONE insertar nuevos parámetros en debugs
- DONE (ya estaba hecho) hacer parámetro NUM_POINTS_PER_LEAF 
- implementar forma de hints alternativa ?? (creo que no)
- DONE preparar experimento!!!

RESUMEN FLUJO:
1. CONSTRUCCIÓN 
 - D
 - KJ
2. Cara

