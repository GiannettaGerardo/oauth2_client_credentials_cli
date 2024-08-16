#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

String* String_new(const char* str, unsigned long capacity)
{
  String* string = NULL;
  unsigned long str_len = strlen(str);

  if (! str || capacity <= 0 || str_len >= capacity) {
    return NULL;
  }

  if (! (string = malloc(sizeof(String)))) {
    return NULL;
  }
  string->capacity = capacity;
  string->length = str_len;

  if (! (string->buffer = calloc(capacity, sizeof(char)))) {
    String_free(string);
    return NULL;
  }
  strncpy(string->buffer, str, string->length + 1);

  return string;
}

String* String_new_2(const char* str)
{
  if (! str) {
    return NULL;
  }
  return String_new(str, strlen(str) + 1);
}

bool String_free(String* string)
{
  bool res = false;
  if (string)
  {
    if (string->buffer)
    {
      free(string->buffer);
      string->buffer = NULL;
    }
    free(string);
    res = true;
  }
  return res;
}

void String_zero(String* string)
{
  memset(string->buffer, 0, string->capacity);
  string->length = 0;
}
