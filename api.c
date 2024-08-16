#include "api.h"
#include "oauth2login.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>

#define ACCESS_TOKEN_MAX 2048

Api* Api_create(const char* url_prefix)
{
  Api* api = NULL;
  bool auth_result;
  size_t url_prefix_len = 0;

  if (! url_prefix) {
    return NULL;
  }
  url_prefix_len = strlen(url_prefix);
  if (url_prefix_len <= 0 || url_prefix_len >= URL_PREFIX_MAX) {
    return NULL;
  }

  if (! (api = malloc(sizeof(Api)))) {
    return NULL;
  }

  /* In windows, this inits the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);
  
  auth_result = OAuth2ClientCredential_create(
    "https://auth.sso.sectigo.com/auth/realms/apiclients/protocol/openid-connect/token",
    "21e27a40-688b-42d6-ae70-fd08777cebf4",
    "CcRKW]gkArhQ[BCAGuj`lKeabRuOVLTX"
  );
  if (! auth_result) {
    free(api);
    return NULL;
  }

  strcpy(api->url_prefix, url_prefix);
  
  return api;
}

bool Api_destroy(Api* api)
{
  bool res = false;
  if (api) {
    OAuth2ClientCredential_destroy();
    free(api);

    /* we are done with libcurl, so clean it up */
    curl_global_cleanup();

    res = true;
  }
  return res;
}

void test(Api* api)
{
  char access_token[ACCESS_TOKEN_MAX + 1];
  unsigned long access_token_len = 0L;

  for (int i = 0; i < 5; ++i)
  {
    memset(access_token, 0, ACCESS_TOKEN_MAX + 1);
    sleep(2);

    access_token_len = get_access_token_len();
    if (access_token_len > ACCESS_TOKEN_MAX)
    {
      fprintf(stderr, "Access Token length is too much\n");
      return;
    }
    get_access_token_copy(access_token);
    printf("ACCESS_TOKEN: %s\n\n", access_token);
  }
}
