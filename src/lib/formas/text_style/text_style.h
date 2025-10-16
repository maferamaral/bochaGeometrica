/**
 * TextStyle ADT - Represents text style attributes
 *
 * This module provides an abstract data type for text style with font
 * family, weight, and size attributes.
 */
#ifndef TEXT_STYLE_H
#define TEXT_STYLE_H

typedef void *TextStyle;

/**
 * Creates a new text style instance
 * @param font_family Font family string
 * @param font_weight Font weight character
 * @param font_size Font size
 * @return Pointer to new text style or NULL on error
 */
TextStyle text_style_create(const char *font_family, char font_weight,
                            int font_size);

/**
 * Destroys a text style instance and frees all memory
 * @param text_style Text style instance to destroy
 */
void text_style_destroy(TextStyle text_style);

/**
 * Gets the font family string
 * @param text_style Text style instance
 * @return Font family string (do not free)
 */
const char *text_style_get_font_family(TextStyle text_style);

/**
 * Gets the font weight character
 * @param text_style Text style instance
 * @return Font weight character
 */
char text_style_get_font_weight(TextStyle text_style);

/**
 * Gets the font size
 * @param text_style Text style instance
 * @return Font size
 */
int text_style_get_font_size(TextStyle text_style);

#endif // TEXT_STYLE_H