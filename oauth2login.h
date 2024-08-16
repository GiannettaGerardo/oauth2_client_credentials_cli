#ifndef _OAUTH2LOGIN_H_
#define _OAUTH2LOGIN_H_

#include <stdbool.h>

bool OAuth2ClientCredential_create(const char* url, const char* client_id, const char* client_secret);
bool OAuth2ClientCredential_destroy(void);

void get_access_token_copy(char access_token_copy[]);
unsigned long get_access_token_len(void);

#endif
