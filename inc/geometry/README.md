# `/inc/geometry/` - Primitivas Geométricas

Módulo que define los tipos de datos geométricos básicos utilizados en todo el proyecto para representar puntos y volúmenes de búsqueda.

## Archivos Principales

### 🔹 `point.hpp`
Definición de la clase `Point` para representar puntos en 3D.

**Características principales:**
- Representa un punto en el espacio 3D
- Almacena coordenadas (x, y, z) como float o double según compilación
- Métodos para operaciones geométricas
  - `distance(other)` - Distancia Euclidiana
  - `squared_distance(other)` - Distancia al cuadrado (más rápido)
  - `direction_to(other)` - Vector dirección
  - `normalize()` - Normalización
  - `dot(other)` - Producto escalar
  - `cross(other)` - Producto vectorial
- Operadores (`+`, `-`, `*`, `/`, `==`, etc.)
- Acceso a coordenadas por índice (p[0], p[1], p[2])

**Tipos soportados:**
```cpp
using Point3f = Point<float>;   // Precisión simple (4 bytes/coordenada)
using Point3d = Point<double>;  // Precisión doble (8 bytes/coordenada)
```

**Optimizaciones:**
- Inline para operaciones simples
- SIMD donde sea posible
- Métodos const para operaciones no-mutantes

---

### 📦 `point_containers.hpp`
Contenedores eficientes para almacenar múltiples puntos.

**Dos modelos de almacenamiento:**

#### 1. **Array of Structures (AoS)** - Por defecto
```
[x0, y0, z0] [x1, y1, z1] [x2, y2, z2] ...
```
- Todo un punto junto en memoria
- Acceso natural al punto individual
- Útil para procesamiento por punto

#### 2. **Structure of Arrays (SoA)** - Alternativo
```
[x0, x1, x2, ...] [y0, y1, y2, ...] [z0, z1, z2, ...]
```
- Coordenadas homogéneas juntas
- Mejor para compilación vectorizada SIMD
- Mejor cache behavior en algunos casos
- Útil para procesamiento por coordenada

**Clase principal:**
```cpp
template<typename Point_t>
class PointContainer {
    // AoS o SoA según template parameter
    std::vector<Point_t> data;  // Para AoS
    // O
    struct {
        std::vector<float> x, y, z;  // Para SoA
    };
};
```

**Métodos:**
- `add(point)` - Agregar punto
- `get(i)` - Obtener punto i
- `set(i, point)` - Establecer punto i
- `size()` - Número de puntos
- `reorder(indices)` - Reordenar según índices

---

### 🏷️ `point_metadata.hpp`
Información adicional asociada a cada punto.

**Contenido:**
- `PointMetadata` - Estructura para metadatos
  - ID original del punto
  - Clasificación (terreno, vegetación, agua, etc.)
  - Intensidad/Reflectancia
  - Número de retorno
  - Intensidad RGB
  - Información de digitalización

**Vinculación:**
```cpp
std::vector<Point> points;        // Coordenadas
std::vector<PointMetadata> meta;  // Metadatos asociados
// Mismo índice i conecta points[i] con meta[i]
```

**Uso:**
- Preservar información LAS original
- Filtrado y clasificación
- Validación de resultados
- Exportación de resultados

---

### 📦 `box.hpp`
Representación de cajas 3D (**Axis-Aligned Bounding Box - AABB**).

**Características:**
- Caja allineada con ejes (no rotada)
- Definida por:
  - Esquina mínima (x_min, y_min, z_min)
  - Esquina máxima (x_max, y_max, z_max)
  - O centro + media-tamaño (half_size)

**Métodos principales:**
- `contains(point)` - ¿Punto dentro de la caja?
- `intersects(sphere)` - ¿Caja interseca esfera?
- `distance_to(point)` - Distancia más cercana
- `volume()` - Volumen
- `expand(point)` - Expandir para incluir punto
- `split()` - Dividir en 8 cajas (para octante)

**Operadores:**
- `operator&` - Intersección de cajas
- `operator|` - Unión (bounding box)
- `operator==` - Comparación

**Uso principal:**
- Los nodos de octree son cajas
- Ragging y pruning rápido en búsquedas
- Descartar ramas completas que no intersectan

---

## Relaciones entre Clases

```
Point (una coordenada)
  ↓
PointContainer (almacenamiento de N puntos)
  ├─ AoS: [Point, Point, Point, ...]
  └─ SoA: [x], [y], [z]
  
PointMetadata (información asociada)
  + (indices)

Box (volumen octree)
  - contiene rangos de puntos
  - intersecta con esferas de búsqueda
```

---

## Operaciones Geométricas Importantes

### Cálculo de Distancia (búsquedas kNN)
```cpp
Point p1, p2;
float dist_sq = p1.squared_distance(p2);  // Rápido: x²+y²+z²
float dist = sqrt(dist_sq);               // Solo si necesario
```

### Contención en Caja (pruning rápido)
```cpp
Box octree_node;
Point query;
if (octree_node.contains(query)) {
    // Todos los puntos en esta caja están dentro del radio
    // Incluir toda la rama
} else if (!octree_node.intersects(sphere)) {
    // La caja no interseca la esfera de búsqueda
    // Descartar toda la rama
} else {
    // Intersección parcial: revisar puntos individuales
}
```

### Reordenamiento Polar (OctreeReordered)
```cpp
Point center;
Point p;

// Coordenadas cilíndricas
float phi = atan2(p.y - center.y, p.x - center.x);  // [0, 2π]
float r   = sqrt((p.x-center.x)² + (p.y-center.y)²);
float z   = p.z;

// Coordenadas esféricas
float theta = acos((p.z - center.z) / r);  // [0, π]
```

---

## Optimizaciones para Performance

### 1. **Precisión (float vs. double)**
- Usar `float` cuando posible (2x menos memoria)
- `double` solo cuando necesario (benchmarking detallado)
- Compilable con switches de tipo

### 2. **Evitar sqrt**
- Usar `squared_distance` para comparaciones
- Preserva orden: si dist²(a,c) < dist²(b,c), entonces dist(a,c) < dist(b,c)
- sqrt es operación lenta

### 3. **Vectorización SIMD**
- PointContainer SoA permite vectorización automática
- Compilador puede procesar puntos en lotes
- 4-8 puntos simultáneos con AVX

### 4. **Localidad de Caché**
- AoS: mejor para acceso punto por punto
- SoA: mejor para iteraciones sobre coordenadas
- Trade-off según patrón de acceso

---

## Uso en el Proyecto

El flujo típico usa estas geometrías así:

```cpp
// 1. Cargar puntos y metadatos
std::vector<Point> points;           // geometry/point.hpp
std::vector<PointMetadata> metadata; // geometry/point_metadata.hpp

// 2. Almacenarlos eficientemente
PointContainer<Point> container(AoS);  // geometry/point_containers.hpp
container.add_range(points);

// 3. Construir cajas para estructura
Box root_box = Box::from_points(points);  // geometry/box.hpp
for (auto& octant : root_box.split()) {
    // Cada octante es una caja más pequeña
}

// 4. Usarlas en búsquedas
auto results = octree.neighbors_within(query_point, radius);
// Internamente usa Box::intersects(sphere) para pruning
```

---

## Testing

Ver `tests/geometry/` para:
- `test_points.cpp` - Tests de operaciones de Point
- `test_geometry.cpp` - Tests de Box y operaciones geométricas
- Validación numérica de distancias
- Pruebas de intersección

---

## Notas Técnicas

- Todas las operaciones son altamente optimizadas (inline)
- Se usa compensación accuracy ↔ velocidad según parámetros de compilación
- Compatible con SIMD cuando sea posible
- Los tipos son templates para flexibilidad

