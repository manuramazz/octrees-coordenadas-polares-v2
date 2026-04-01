# `/tests/geometry/` - Tests de Primitivas Geométricas

Directorio con pruebas unitarias para tipos geométricos básicos (Point, Box, operaciones espaciales).

## Archivos

```
geometry/
├── test_points.cpp     # Tests de clase Point
└── test_geometry.cpp   # Tests de Box e intersecciones
```

---

## 📋 `test_points.cpp`
Tests de la clase `Point` y operaciones vectoriales.

**Pruebas:**

```cpp
TEST(PointTest, Construction) {
    Point3f p1;
    EXPECT_EQ(p1.x, 0.0f);
    EXPECT_EQ(p1.y, 0.0f);
    EXPECT_EQ(p1.z, 0.0f);
    
    Point3f p2(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(p2.x, 1.0f);
    EXPECT_EQ(p2.y, 2.0f);
    EXPECT_EQ(p2.z, 3.0f);
}

TEST(PointTest, Distance) {
    Point3f origin(0, 0, 0);
    Point3f p1(3, 4, 0);
    
    // Distancia 3-4-5
    EXPECT_FLOAT_EQ(origin.distance(p1), 5.0f);
    
    // Simetría
    EXPECT_FLOAT_EQ(origin.distance(p1), p1.distance(origin));
}

TEST(PointTest, SquaredDistance) {
    Point3f p1(0, 0, 0);
    Point3f p2(3, 4, 0);
    
    // dist² = 9 + 16 = 25
    EXPECT_FLOAT_EQ(p1.squared_distance(p2), 25.0f);
    
    // Más rápido que distance() para comparaciones
    EXPECT_LT(p1.squared_distance(p2), 36.0f);
}

TEST(PointTest, Addition) {
    Point3f p1(1, 2, 3);
    Point3f p2(4, 5, 6);
    Point3f result = p1 + p2;
    
    EXPECT_EQ(result.x, 5);
    EXPECT_EQ(result.y, 7);
    EXPECT_EQ(result.z, 9);
}

TEST(PointTest, Subtraction) {
    Point3f p1(5, 7, 9);
    Point3f p2(1, 2, 3);
    Point3f diff = p1 - p2;
    
    EXPECT_EQ(diff.x, 4);
    EXPECT_EQ(diff.y, 5);
    EXPECT_EQ(diff.z, 6);
}

TEST(PointTest, ScalarMultiplication) {
    Point3f p(1, 2, 3);
    Point3f scaled = p * 2.0f;
    
    EXPECT_EQ(scaled.x, 2);
    EXPECT_EQ(scaled.y, 4);
    EXPECT_EQ(scaled.z, 6);
}

TEST(PointTest, DotProduct) {
    Point3f p1(1, 0, 0);
    Point3f p2(2, 0, 0);
    
    // dot product
    float dot = p1.dot(p2);
    EXPECT_EQ(dot, 2.0f);
    
    // Perpendicular -> dot = 0
    Point3f p3(0, 1, 0);
    EXPECT_EQ(p1.dot(p3), 0.0f);
}

TEST(PointTest, CrossProduct) {
    Point3f x_axis(1, 0, 0);
    Point3f y_axis(0, 1, 0);
    Point3f z_axis = x_axis.cross(y_axis);
    
    EXPECT_FLOAT_EQ(z_axis.x, 0.0f);
    EXPECT_FLOAT_EQ(z_axis.y, 0.0f);
    EXPECT_FLOAT_EQ(z_axis.z, 1.0f);
}

TEST(PointTest, Normalization) {
    Point3f p(3, 4, 0);
    Point3f normalized = p.normalize();
    
    // Longitud debe ser 1
    EXPECT_NEAR(normalized.distance(Point3f(0, 0, 0)), 1.0f, 1e-6);
    
    // Dirección preservada
    EXPECT_NEAR(p.dot(normalized), 5.0f, 1e-6);  // magnitud original
}

TEST(PointTest, Magnitude) {
    Point3f p(3, 4, 0);
    EXPECT_FLOAT_EQ(p.magnitude(), 5.0f);
}

TEST(PointTest, Equality) {
    Point3f p1(1, 2, 3);
    Point3f p2(1, 2, 3);
    Point3f p3(1, 2, 4);
    
    EXPECT_EQ(p1, p2);
    EXPECT_NE(p1, p3);
}

TEST(PointTest, Indexing) {
    Point3f p(1, 2, 3);
    
    EXPECT_EQ(p[0], 1);    // x
    EXPECT_EQ(p[1], 2);    // y
    EXPECT_EQ(p[2], 3);    // z
}
```

---

## 📋 `test_geometry.cpp`
Tests de caja 3D (`Box`) e intersecciones.

**Pruebas:**

```cpp
TEST(BoxTest, Construction) {
    Point3f min(0, 0, 0);
    Point3f max(10, 10, 10);
    Box box(min, max);
    
    EXPECT_EQ(box.min(), min);
    EXPECT_EQ(box.max(), max);
}

TEST(BoxTest, Center) {
    Box box(Point3f(0, 0, 0), Point3f(10, 10, 10));
    Point3f center = box.center();
    
    EXPECT_EQ(center.x, 5.0f);
    EXPECT_EQ(center.y, 5.0f);
    EXPECT_EQ(center.z, 5.0f);
}

TEST(BoxTest, Size) {
    Box box(Point3f(0, 0, 0), Point3f(10, 20, 30));
    Point3f size = box.size();
    
    EXPECT_EQ(size.x, 10.0f);
    EXPECT_EQ(size.y, 20.0f);
    EXPECT_EQ(size.z, 30.0f);
}

TEST(BoxTest, Contains) {
    Box box(Point3f(0, 0, 0), Point3f(10, 10, 10));
    
    // Punto dentro
    EXPECT_TRUE(box.contains(Point3f(5, 5, 5)));
    
    // Punto fuera
    EXPECT_FALSE(box.contains(Point3f(15, 5, 5)));
    
    // Punto en borde (depende implementación)
    EXPECT_TRUE(box.contains(Point3f(0, 0, 0)));  // Esquina
    EXPECT_TRUE(box.contains(Point3f(10, 10, 10)));  // Esquina opuesta
}

TEST(BoxTest, SphereIntersection) {
    Box box(Point3f(0, 0, 0), Point3f(10, 10, 10));
    
    // Esfera dentro -> intersecta
    Sphere s1(Point3f(5, 5, 5), 2.0f);
    EXPECT_TRUE(box.intersects(s1));
    
    // Esfera fuera -> no intersecta
    Sphere s2(Point3f(20, 5, 5), 2.0f);
    EXPECT_FALSE(box.intersects(s2));
    
    // Esfera tocando boundary -> intersecta
    Sphere s3(Point3f(10, 5, 5), 2.0f);
    EXPECT_TRUE(box.intersects(s3));
}

TEST(BoxTest, DistanceToPoint) {
    Box box(Point3f(0, 0, 0), Point3f(10, 10, 10));
    
    // Punto dentro -> distancia 0
    EXPECT_EQ(box.distance_to(Point3f(5, 5, 5)), 0.0f);
    
    // Punto fuera -> distancia > 0
    float dist = box.distance_to(Point3f(15, 5, 5));
    EXPECT_GT(dist, 0.0f);
    EXPECT_NEAR(dist, 5.0f, 1e-5);  // Distancia a pared más cercana
    
    // Punto en borde -> distancia 0
    EXPECT_EQ(box.distance_to(Point3f(10, 5, 5)), 0.0f);
}

TEST(BoxTest, Split) {
    Box box(Point3f(0, 0, 0), Point3f(10, 10, 10));
    auto octants = box.split();  // Dividir en 8 octantes
    
    EXPECT_EQ(octants.size(), 8);
    
    // Cada octante debe ser menor
    for (const auto& octant : octants) {
        EXPECT_LT(octant.volume(), box.volume());
    }
    
    // Octantes no deben solaparse
    // (geometría elemental)
}

TEST(BoxTest, Union) {
    Box b1(Point3f(0, 0, 0), Point3f(5, 5, 5));
    Box b2(Point3f(3, 3, 3), Point3f(10, 10, 10));
    Box unioned = b1 | b2;  // operator|
    
    // Unión debe contener ambas
    EXPECT_TRUE(unioned.contains(Point3f(1, 1, 1)));  // De b1
    EXPECT_TRUE(unioned.contains(Point3f(8, 8, 8)));  // De b2
    
    // Min/max correctos
    EXPECT_EQ(unioned.min(), Point3f(0, 0, 0));
    EXPECT_EQ(unioned.max(), Point3f(10, 10, 10));
}

TEST(BoxTest, Intersection) {
    Box b1(Point3f(0, 0, 0), Point3f(10, 10, 10));
    Box b2(Point3f(5, 5, 5), Point3f(15, 15, 15));
    Box inter = b1 & b2;  // operator&
    
    EXPECT_EQ(inter.min(), Point3f(5, 5, 5));
    EXPECT_EQ(inter.max(), Point3f(10, 10, 10));
}

TEST(BoxTest, Disjoint) {
    Box b1(Point3f(0, 0, 0), Point3f(5, 5, 5));
    Box b2(Point3f(10, 10, 10), Point3f(15, 15, 15));
    
    // No deben intersectar
    EXPECT_FALSE(b1.intersects(b2));
}

TEST(BoxTest, Volume) {
    Box box(Point3f(0, 0, 0), Point3f(2, 3, 4));
    EXPECT_EQ(box.volume(), 24.0f);
}

TEST(BoxTest, ClosestPointOnBox) {
    Box box(Point3f(0, 0, 0), Point3f(10, 10, 10));
    
    // Punto fuera -> punto más cercano en caja
    Point3f closest = box.closest_point(Point3f(15, 5, 5));
    EXPECT_EQ(closest.x, 10.0f);  // Pared más cercana
    EXPECT_EQ(closest.y, 5.0f);
    EXPECT_EQ(closest.z, 5.0f);
}
```

---

## Ejecución

```bash
./tests/test_points
./tests/test_geometry
./tests/test_points --gtest_filter="PointTest.Distance"
./tests/test_geometry --gtest_filter="BoxTest.SphereIntersection"
```

---

## Notas

- Tests validan **correctitud matemática**
- Tolerancia para **operaciones flotantes** (1e-5 típicamente)
- Cobertura de **casos extremos**: origen, límites, etc.
- Operaciones usadas en búsqueda espacial **críticas**

