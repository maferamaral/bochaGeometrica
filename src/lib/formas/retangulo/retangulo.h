/**
 * Rectangle ADT - Represents a rectangle geometric element
 *
 * This module provides an abstract data type for rectangles with position,
 * dimensions, and color attributes.
 */
// Tradução para português
#ifndef RETANGULO_H
#define RETANGULO_H

typedef void *Rectangle;
typedef void *Retangulo;

/**
 * Creates a new rectangle instance
 * @param id Rectangle identifier
 * @param x X coordinate of anchor point
 * @param y Y coordinate of anchor point
 * @param width Rectangle width
 * @param height Rectangle height
 * @param border_color Border color string
 * @param fill_color Fill color string
 * @return Pointer to new rectangle or NULL on error
 */
Rectangle rectangle_create(int id, double x, double y, double width, double height,
                           const char *border_color, const char *fill_color);
Retangulo retangulo_criar(int id, double x, double y, double largura, double altura,
                          const char *cor_borda, const char *cor_preenchimento);

/**
 * Destroys a rectangle instance and frees all memory
 * @param rectangle Rectangle instance to destroy
 */
void rectangle_destroy(Rectangle rectangle);
void retangulo_destruir(Retangulo retangulo);

/**
 * Gets the rectangle identifier
 * @param rectangle Rectangle instance
 * @return Rectangle identifier
 */
int rectangle_get_id(Rectangle rectangle);
int retangulo_get_id(Retangulo retangulo);

/**
 * Gets the X coordinate of rectangle anchor
 * @param rectangle Rectangle instance
 * @return X coordinate
 */
double rectangle_get_x(Rectangle rectangle);
double retangulo_get_x(Retangulo retangulo);

/**
 * Gets the Y coordinate of rectangle anchor
 * @param rectangle Rectangle instance
 * @return Y coordinate
 */
double rectangle_get_y(Rectangle rectangle);
double retangulo_get_y(Retangulo retangulo);

/**
 * Gets the rectangle width
 * @param rectangle Rectangle instance
 * @return Rectangle width
 */
double rectangle_get_width(Rectangle rectangle);
double retangulo_get_largura(Retangulo retangulo);

/**
 * Gets the rectangle height
 * @param rectangle Rectangle instance
 * @return Rectangle height
 */
double rectangle_get_height(Rectangle rectangle);
double retangulo_get_altura(Retangulo retangulo);

/**
 * Gets the border color string
 * @param rectangle Rectangle instance
 * @return Border color string (do not free)
 */
const char *rectangle_get_border_color(Rectangle rectangle);
const char *retangulo_get_cor_borda(Retangulo retangulo);

/**
 * Gets the fill color string
 * @param rectangle Rectangle instance
 * @return Fill color string (do not free)
 */
const char *rectangle_get_fill_color(Rectangle rectangle);
const char *retangulo_get_cor_preenchimento(Retangulo retangulo);

#endif // RETANGULO_H