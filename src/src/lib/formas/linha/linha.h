/**
 * Line ADT - Represents a line geometric element
 *
 * This module provides an abstract data type for lines with start/end points
 * and color attributes.
 */
#ifndef LINE_H
#define LINE_H

typedef void *Line;

/**
 * Creates a new line instance
 * @param id Line identifier
 * @param x1 X coordinate of start point
 * @param y1 Y coordinate of start point
 * @param x2 X coordinate of end point
 * @param y2 Y coordinate of end point
 * @param color Line color string
 * @return Pointer to new line or NULL on error
 */
Line line_create(int id, double x1, double y1, double x2, double y2,
                 const char *color);

/**
 * Destroys a line instance and frees all memory
 * @param line Line instance to destroy
 */
void line_destroy(Line line);

/**
 * Gets the line identifier
 * @param line Line instance
 * @return Line identifier
 */
int line_get_id(Line line);

/**
 * Gets the X coordinate of start point
 * @param line Line instance
 * @return X coordinate of start point
 */
double line_get_x1(Line line);

/**
 * Gets the Y coordinate of start point
 * @param line Line instance
 * @return Y coordinate of start point
 */
double line_get_y1(Line line);

/**
 * Gets the X coordinate of end point
 * @param line Line instance
 * @return X coordinate of end point
 */
double line_get_x2(Line line);

/**
 * Gets the Y coordinate of end point
 * @param line Line instance
 * @return Y coordinate of end point
 */
double line_get_y2(Line line);

/**
 * Gets the line color string
 * @param line Line instance
 * @return Line color string (do not free)
 */
const char *line_get_color(Line line);

#endif // LINE_H