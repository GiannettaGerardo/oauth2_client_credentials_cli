#ifndef _API_H_
#define _API_H_

#include "oauth2login.h"

#define URL_PREFIX_MAX 51

typedef struct {

  char url_prefix[URL_PREFIX_MAX];

} Api;

Api* Api_create(const char* url_prefix);
bool Api_destroy(Api* api);
void test(Api* api);

#endif
