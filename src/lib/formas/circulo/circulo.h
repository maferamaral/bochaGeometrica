/**
 * Circle ADT - Represents a circle geometric element
 *
 * This module provides an abstract data type for circles with position,
 * radius, and color attributes.
 */
// Tradução para português
#ifndef CIRCULO_H
#define CIRCULO_H

typedef void *Circulo;

/**
 * Creates a new circle instance
 * @param id Circle identifier
 * @param x X coordinate of center
 * @param y Y coordinate of center
 * @param radius Circle radius
 * @param border_color Border color string
 * @param fill_color Fill color string
 * @return Pointer to new circle or NULL on error
 */
Circulo circulo_criar(int id, double x, double y, double raio,
                      const char *cor_borda, const char *cor_preenchimento);

/**
 * Destroys a circle instance and frees all memory
 * @param circle Circle instance to destroy
 */
void circulo_destruir(Circulo circulo);

/**
 * Gets the circle identifier
 * @param circle Circle instance
 * @return Circle identifier
 */
int circulo_get_id(Circulo circulo);

/**
 * Gets the X coordinate of circle center
 * @param circle Circle instance
 * @return X coordinate
 */
double circulo_get_x(Circulo circulo);

/**
 * Gets the Y coordinate of circle center
 * @param circle Circle instance
 * @return Y coordinate
 */
double circulo_get_y(Circulo circulo);

/**
 * Gets the circle radius
 * @param circle Circle instance
 * @return Circle radius
 */
double circulo_get_raio(Circulo circulo);

/**
 * Gets the border color string
 * @param circle Circle instance
 * @return Border color string (do not free)
 */
const char *circulo_get_cor_borda(Circulo circulo);

/**
 * Gets the fill color string
 * @param circle Circle instance
 * @return Fill color string (do not free)
 */
const char *circulo_get_cor_preenchimento(Circulo circulo);

#endif // CIRCULO_H