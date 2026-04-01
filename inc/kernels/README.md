# `/inc/kernels/` - Kernels de Búsqueda Espacial

Módulo que implementa la lógica de búsqueda de vecinos (dentro de radio y k-NN) para diferentes dimensiones y tipos de estructuras.

## Concepto

Un **kernel de búsqueda** encapsula la lógica de validación de puntos durante búsquedas:
- ¿Está este punto dentro del radio de búsqueda?
- ¿Es este punto un mejor candidato para kNN?
- ¿Puede descartarse esta rama completa del árbol?

## Archivos Principales

### 🔍 `kernel_abstract.hpp`
Clase base abstracta `Kernel` que define la interfaz general.

**Métodos principales:**
- `is_valid(point)` - ¿Punto satisface criterio de búsqueda?
- `should_prune(box)` - ¿Puede descartarse esta caja completa?
- `can_contain(box)` - ¿Puede esta caja contener puntos válidos?
- `set_query(point)` - Establecer punto de consulta
- `set_radius(radius)` - Establecer radio de búsqueda (para búsqueda por radio)
- `set_k(k)` - Establecer k (para kNN)

**Parámetros:**
```cpp
struct SearchParams {
    Point query_point;       // Centro de búsqueda
    float radius;            // Para búsqueda por radio
    unsigned int k;          // Para búsqueda kNN
    ReorderMode reorder;     // Modo de reordenamiento
};
```

---

### 🟦 `kernel_2d.hpp`
Implementación especializada para búsquedas **2D**.

**Particularidades:**
- Búsquedas circulares (en lugar de esféricas)
- Cálculos más simples (sin coordenada z)
- Más rápidas que 3D
- Útil para datos proyectados a 2D

**Métodos especializados:**
- `is_valid_2d(x, y)` - Validación 2D rápida
- `distance_to_box_2d(box)` - Distancia a caja 2D
- `point_in_circle(point)` - Punto en círculo

---

### 🟦 `kernel_3d.hpp`
Implementación especializada para búsquedas **3D**.

**Particularidades:**
- Búsquedas esféricas completas
- Validación 3D con coordenada z
- Más lento que 2D pero más flexible
- Estándar para datos LiDAR completos

**Métodos especializados:**
- `is_valid_3d(x, y, z)` - Validación 3D
- `distance_to_box_3d(box)` - Distancia 3D
- `sphere_box_intersection(box)` - Intersección esfera-caja

**Optimizaciones clave:**
```cpp
// Prueba rápida: distancia al punto más cercano de la caja
float dx = max(0.0f, |query.x - box.center.x| - box.half_size.x);
float dy = max(0.0f, |query.y - box.center.y| - box.half_size.y);
float dz = max(0.0f, |query.z - box.center.z| - box.half_size.z);
float dist_sq = dx*dx + dy*dy + dz*dz;

if (dist_sq > radius*radius) {
    // Caja completamente fuera del radio: descartar rama
    return PRUNE;
}
```

---

### 🏭 `kernel_factory.hpp`
Factory para crear kernels apropiados.

**Responsabilidades:**
- Instanciar kernel correcto según dimensión
- Validar parámetros
- Singleton o pool de kernels

**Uso típico:**
```cpp
auto kernel = KernelFactory::create(KernelType::KERNEL_3D);
kernel.set_query(query_point);
kernel.set_radius(search_radius);

// Usar en búsqueda
if (kernel.is_valid(point)) {
    results.add(point);
}
```

---

## Tipos de Búsquedas Soportadas

### 1. **Búsqueda por Radio (Radius Search)**
Encuentra todos los puntos dentro de una distancia fija.

**Algoritmo:**
```cpp
neighbors = [];
for each node in octree:
    if node.bbox.intersects(sphere with radius):
        if node is leaf:
            for each point in node:
                if distance(point, query) <= radius:
                    neighbors.add(point)
        else:
            recurse(node.children)
```

**Validación:**
- `distance(point, query) <= radius`
- Sin orden (todos los encontrados)

### 2. **k-Nearest Neighbors (kNN)**
Encuentra los k puntos más cercanos.

**Algoritmo:**
```cpp
heap = MaxHeap(k);  // maxHeap de los k mejores
for each node in octree:
    if can_prune(node):  // Si distancia mínima > k-ésimo mejor
        continue
    if node is leaf:
        for each point in node:
            if distance < heap.max:
                heap.add(point)
    else:
        recurse(sorted_by_distance_to_center(node.children))
return sorted(heap)
```

**Validación:**
- Mantener heap de k mejores
- Actualizar radio dinámicamente

---

## Flujo de Búsqueda con Kernels

```
Query Point → SetQuery → SetRadius/SetK
                      ↓
            Traverse Octree
                      ↓
            Check Node AABB
         ↙                    ↖
  can_prune?              intersects?
    ↓ YES                    ↓ YES
  SKIP                   Recurse Children
                              ↓
                         forEach point
                              ↓
                         is_valid()?
                         ↙          ↖
                      YES           NO
                       ↓            ↓
                   Add Result      Skip
```

---

## Optimizaciones Clave

### 1. **Early Termination (Pruning)**
Descartar ramas completas sin revisar puntos:
```cpp
if (node.min_distance_to_query > radius) {
    // Ningún punto en esta rama puede estar en radio
    return PRUNE;
}
```

### 2. **Coordinatización y Caching**
```cpp
// Evitar sqrt en comparaciones
if (dist_squared <= radius_squared) {  // Válido
    // Usar sqrt solo si necesario
}
```

### 3. **Spatial Sorting**
En kNN, explorar niños en orden de proximidad:
```cpp
auto children = node.children;
std::sort(children.begin(), children.end(),
    [&query](auto a, auto b) {
        return distance(a.center, query) < 
               distance(b.center, query);
    });
// Explorar niños cerca primero → mejor poda
```

---

## Diferencias Según Estructura

Los kernels se adaptan al tipo de reordenamiento:

### Sin Reordenamiento (None)
- Búsqueda lineal simple
- Validación: `dist(p, q) <= radius`

### Reordenamiento Cilíndrico
- Optimización: eliminar puntos por ángulo φ y radio r
- Validación: primero por coordenadas cilíndricas
- Mejor para datos con búsquedas alrededor de ejes

### Reordenamiento Esférico  
- Optimización: eliminar por ángulos φ, θ y radio r
- Validación: coordenadas esféricas
- Mejor para distribuciones radiales

---

## Uso en el Proyecto

### En Octree::search()
```cpp
template<typename Kernel_t>
NeighborSet search(Point query, Kernel_t kernel) {
    NeighborSet result;
    kernel.set_query(query);
    _search_recursive(root, kernel, result);
    return result;
}

void _search_recursive(Node* node, Kernel_t& kernel, NeighborSet& results) {
    if (kernel.should_prune(node.bbox)) return;
    
    if (node.is_leaf()) {
        for (auto idx : node.points) {
            if (kernel.is_valid(points[idx])) {
                results.add(idx);
            }
        }
    } else {
        for (auto child : node.children) {
            _search_recursive(child, kernel, results);
        }
    }
}
```

### En Benchmarks
```cpp
auto kernel_2d = KernelFactory::create(KernelType::KERNEL_2D);
auto kernel_3d = KernelFactory::create(KernelType::KERNEL_3D);

// Comparar rendimiento 2D vs 3D
benchmark(octree, kernel_2d);
benchmark(octree, kernel_3d);
```

---

## Testing

Ver `tests/` para validación de kernels:
- Validación correcta de puntos dentro/fuera
- Pruining efectivo de ramas
- Exactitud de distancias
- Correctitud de kNN

---

## Notas de Implementación

- Kernels son templates sin estado (stateless)
- Métodos inline para operaciones críticas
- SIMD cuando sea posible
- Compilación selectiva según dimensión

