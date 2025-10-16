#include "text_style.h"
#include "../../utils/utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * Internal TextStyle structure
 */
struct TextStyle {
  char *font_family;
  char font_weight;
  int font_size;
};

void *text_style_create(const char *font_family, char font_weight,
                        int font_size) {
  if (!font_family) {
    return NULL;
  }

  struct TextStyle *text_style = malloc(sizeof(struct TextStyle));
  if (!text_style) {
    return NULL;
  }

  text_style->font_weight = font_weight;
  text_style->font_size = font_size;

  text_style->font_family = duplicate_string(font_family);
  if (!text_style->font_family) {
    free(text_style);
    return NULL;
  }

  return text_style;
}

void text_style_destroy(void *text_style) {
  if (!text_style)
    return;

  struct TextStyle *ts = (struct TextStyle *)text_style;
  free(ts->font_family);
  free(ts);
}

const char *text_style_get_font_family(void *text_style) {
  if (!text_style)
    return NULL;
  return ((struct TextStyle *)text_style)->font_family;
}

char text_style_get_font_weight(void *text_style) {
  if (!text_style)
    return '\0';
  return ((struct TextStyle *)text_style)->font_weight;
}

int text_style_get_font_size(void *text_style) {
  if (!text_style)
    return 0;
  return ((struct TextStyle *)text_style)->font_size;
}