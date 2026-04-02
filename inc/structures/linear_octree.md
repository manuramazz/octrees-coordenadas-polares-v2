# LinearOctree: estructura interna y atributos clave

Este documento describe la estructura de datos `LinearOctree` definida en `linear_octree.hpp`, con foco en:

- `LeafPart`
- `InternalPart`
- Atributos globales usados en búsqueda:
  - `offsets`
  - `internalRanges`
  - `centers`
  - `precomputedRadii`

## 1. Vision general del LinearOctree

`LinearOctree` implementa un octree lineal basado en codificacion espacial (Morton/Hilbert) y en el enfoque de Cornerstone.

Idea principal:

1. Los puntos ya codificados (`codes`) se asumen ordenables por su clave espacial.
2. Se construye una particion lineal de hojas (`leaf.leaves`) en espacio de claves.
3. Se genera una representacion interna compacta de nodos (prefijos en formato Warren-Salmon + relaciones padre/hijo).
4. Se transforman esos datos temporales en cuatro arrays persistentes que aceleran consultas de vecinos.

## 2. Parte temporal de hojas: `LeafPart`

`LeafPart` existe durante la construccion y se descarta al terminar.

### 2.1 `std::vector<key_t> leaves`

Representa los limites de hojas en formato cornerstone.

Interpretacion:

- Su tamaño es `nLeaf + 1`.
- Cada hoja `i` corresponde al intervalo de claves:
  - `[leaves[i], leaves[i+1])`
- Esta ordenado en creciente.
- Se inicializa con dos extremos (`0` y `upperBound`) y se rebalancea con split/merge.

Funcion: definir la particion espacial en espacio de codigos, no en memoria de punteros.

### 2.2 `std::vector<size_t> counts`

`counts[i]` es el numero de puntos cuyo codigo cae en la hoja `i`.

Se recalcula tras cada iteracion de rebalanceo (`computeNodeCounts`).

Funcion: guiar decisiones de rebalanceo (split/merge/keep) y preparar el layout de puntos por hoja.

### 2.3 `std::vector<size_t> layout`

Es el scan exclusivo de `counts`.

Interpretacion:

- `layout[i]` = indice inicial de puntos de la hoja `i` en el array ordenado de puntos.
- `layout[i+1]` = indice final exclusivo.

Funcion: puente entre hoja logica y rango contiguo en el contenedor de puntos.

## 3. Parte temporal interna: `InternalPart`

`InternalPart` tambien es temporal y se usa para construir el arbol interno enlazado por indices.

### 3.1 `std::vector<key_t> prefixes`

Clave Warren-Salmon (placeholder bit) de cada nodo.

Funcion:

- Codifica posicion del nodo en el arbol (prefijo + profundidad).
- Permite ordenar nodos y buscar hijos por rango de nivel con `lower_bound`.

### 3.2 `std::vector<uint32_t> parents`

Guarda el padre por grupos de 8 hermanos.

Notas:

- Se rellena en `linkTree`.
- Es auxiliar y no se usa en consultas de vecinos del camino principal.

### 3.3 `std::vector<size_t> levelRange`

`levelRange[L]` marca el primer indice de nodos del nivel `L` dentro de `prefixes` ordenado.

Funcion:

- Delimitar rapidamente ventanas de busqueda por nivel:
  - `[levelRange[L], levelRange[L+1])`

### 3.4 `std::vector<int32_t> internalToLeaf`

Mapeo del indice interno ordenado de nodo hacia el indice de hoja en layout cornerstone.

Uso clave:

- Cuando un nodo es hoja, se usa para transformar nodo -> hoja y asi recuperar rango de puntos via `leaf.layout`.

### 3.5 `std::vector<int32_t> leafToInternal`

Mapeo inverso de `internalToLeaf`.

Uso clave:

- Traducir indices durante el enlazado de nodos (`linkTree`) y ordenar representaciones.

## 4. Atributos globales persistentes para busquedas

Estos arrays viven en el objeto final y son los mas importantes en tiempo de consulta.

## 4.1 `offsets`

Tipo: `std::vector<uint32_t>`

Significado por nodo `n`:

- `offsets[n] == 0` => `n` es hoja.
- `offsets[n] != 0` => `offsets[n]` es el indice del primer hijo; los 8 hijos son contiguos:
  - `offsets[n] + 0` ... `offsets[n] + 7`

Rol en algoritmos:

- Es la estructura de conectividad para DFS/BFS en `singleTraversal`.
- Permite navegar el octree sin punteros dinamicos.

## 4.2 `internalRanges`

Tipo: `std::vector<std::pair<size_t, size_t>>`

Significado por nodo `n`:

- `internalRanges[n].first` = inicio de puntos
- `internalRanges[n].second` = fin exclusivo

Interpretacion:

- Para una hoja: rango exacto de puntos de esa hoja.
- Para un interno: union contigua de los rangos de sus hijos.

Rol en algoritmos:

- Si un nodo cae completamente dentro del kernel, se añade todo su rango directamente (pruning positivo).
- Evita revisar punto a punto cuando no es necesario.

## 4.3 `centers`

Tipo: `std::vector<Point>`

Significado:

- `centers[n]` es el centro geometrico del cubo asociado al nodo `n`.

Construccion:

- Se deriva de `prefixes` y nivel del nodo con el encoder.

Rol en algoritmos:

- Pruebas de interseccion kernel-octante.
- Distancia punto-octante para kNN.

## 4.4 `precomputedRadii`

Tipo: `std::vector<Vector>` indexado por profundidad

Significado:

- `precomputedRadii[d]` guarda el semieje (x,y,z) de cualquier nodo a profundidad `d`.

Construccion:

- A partir del bounding box global (`box`) y la resolucion del encoder.

Rol en algoritmos:

- Evita recomputar tamanos de octantes en cada visita.
- Se combina con `centers[n]` para construir AABB del nodo en tiempo O(1).

## 5. Flujo de construccion resumido

1. `buildOctreeLeaves(leaf)`
- Construye y rebalancea `leaf.leaves`.
- Calcula `leaf.counts` y `leaf.layout`.

2. `resize(inter)`
- Determina `nLeaf`, `nInternal`, `nTotal`.
- Reserva memoria global y temporal.

3. `buildOctreeInternal(leaf, inter)`
- Construye `prefixes` y mapeos.
- Ordena por prefijo.
- Calcula `levelRange`.
- Enlaza hijos (`offsets`).
- Propaga rangos (`internalRanges`).

4. `computeGeometry(inter)`
- Rellena `centers` para cada nodo.
- Usa `precomputedRadii` por nivel.

Tras esto, `LeafPart` e `InternalPart` se pueden descartar: las busquedas se apoyan en `offsets`, `internalRanges`, `centers` y `precomputedRadii`.

## 6. Relacion directa con busquedas

- `neighborsStruct`, `neighborsPrune`, `neighbors` y `neighborsApprox`:
  - Navegan con `offsets`.
  - Deciden poda/interseccion con `centers` + `precomputedRadii`.
  - Materializan resultados con `internalRanges`.

- `knn`:
  - Usa `offsets` para expandir octantes.
  - Usa `centers` + `precomputedRadii` en distancia minima punto-octante.
  - Usa `internalRanges` para expandir puntos de hojas.

## 7. Notas practicas

- El diseno evita punteros por nodo y favorece memoria contigua.
- Los arrays persistentes estan optimizados para consulta; los temporales para construccion.
- Si se quiere depuracion detallada del proceso de build, deben loguearse `LeafPart` e `InternalPart` antes de salir de `buildOctree`.
