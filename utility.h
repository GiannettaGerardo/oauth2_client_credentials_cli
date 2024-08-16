#ifndef _API_UTILITY_H_
#define _API_UTILITY_H_

#include <stdbool.h>

typedef struct {

  char* buffer;
  unsigned long length;
  unsigned long capacity;

} String;

String* String_new(const char* s, unsigned long capacity);
String* String_new_2(const char* str);
bool String_free(String* string);
void String_zero(String* string);

#endif
