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

PLAN 15/05
- DONE early exit cartesian
- DONE optimizar binary search
- DONE salida de emergencia
- mandar correo a tutores
- (local done) a futuro: comparar max-leaf y umbral-poda en tiempos totales

PLAN SEMANA 18-24/05
-
- Pasar el pnoa por drive ya al cesga o ir a la etse
- Reunión MARTES 17:00:
    - Enseñar funcionamiento actual:
        - Modo polar (ángulo azimutal) y cartesiano (3 ejes)
        - Vector contiguo reordenado localmente (SortedDataFlat)
        - Parámetro personalizable max-leaf umbral-poda
        - Optimizaciones selector:
            - (polar) Si no hay poda, recorre el bucle original sobre points con máxima localidad.
            - (cartesian) Si un eje poda > 50% se admite.
            - (cartesian) Se comprueba si el mínimo/máximo se sale de los límites de la box -> skip búsqueda binaria
    - Preguntar experimento cesga:
        - Si mandar full searches, parallel searches
        - Si variar algoritmos (si sí implementar integración)
        - Si variar encodings
        - Si incluir nuevos datasets (me tendrían que pasar el de dales y decirles que el speulderbos es too big)
    - Enseñar gráficas debugs y preguntar por mejoras
    - Preguntar por gráficas de tiempos completos:
        - tiempo total (si debería quitar los debugs para medir esto) 
        - heatmap con max-leaf/radio/umbral ??
        - Comentar añadir visualizaciones de como funcionan los selectores de rangos
        - Que más puedo visualizar
    - Preguntar por memoria: si enfocarla solo en la versión final o incluir desarrollo, pruebas, optimizaciones anteriores etc.
- Limpiar código
- Mandar todo
- Estructurar memoria

confío :) -> confié demasiado

ESTRATEGIA REFACTORIZACIÓN CÓDIGO
-
- Funciones separadas para cada modo
- vectores de points separados para cada clave y almacenada la referencia en linear octree
- Estrategia diferente para cada modo
    - POLAR: con el vector contiguo idéntico (incluyendo nodos internos, hojas vacías etc) se puede tener literalmente el mismo bucle que la versión base (si la poda funciona el modo funciona)
    - CARTESIANO: no es posible almacenar los índices haciendo push_back(i) porque no se sabe a que reordenación hace referencia cada uno. Habría que almacenar un vector aparte o algo similar que en todo caso no sería más eficiente que lo que se está haciendo ahora -> push_back(p.id()). 
    Por lo tanto la estrategia es: hacer 3 vectores contiguos idénticos a points en tamaño e índices de inicio de cada hoja. Dependiendo del valor de order se usa uno u otro pero no hay que referenciar ningún puntero y se ahorra el coste de leafData (llamada a función + 2 multiplicaciones, sumas + 2 llamadas a memoria). Así sabes siempre de donde buscar y puedes recuperar los vecinos sin problema. Cuando se lanza el modo polar siempre se hace con pointsPolar y cuando se hace con cartesian son los índices del vector points original.
    - Optimización extra para el modo CARTESIAN: después de calcular el rango de cada hoja, se sabe el número de puntos en ese rango se puede hacer reserve con ese número -> Creo que no lo voy a impelmentar porque push_back ya hacer realocaciones generosas, y el número de resizes acaba siendo menor (investigar como son exactamente los reallocs de push_back)
    - Limpieza: implementar funciones diferentes para debug ranges y debug leafs: los ifs los hago en neighbors_benchmark: si hay un debug activado llamo a la función indicada, dentro ya no hay nada que comprobar.


SITUACIÓN ACTUAL
- Faltan las funciones de debug leaves en linear octree
- Luego integrar todo en neighborsbenchmark
- Probarlo ya
    




