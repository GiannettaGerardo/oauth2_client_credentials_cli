#include "oauth2login.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include "parser.h"

#define URL_MAX 100
#define CLIENT_ID_MAX 37
#define CLIENT_SECRET_MAX 33
#define ACCESS_TOKEN_MAX 2049

typedef struct {

  CURL* curl_session;
  pthread_t b_thread;
  pthread_mutex_t lock;
  char url[URL_MAX];
  char client_id[CLIENT_ID_MAX];
  char client_secret[CLIENT_SECRET_MAX];
  Parser* access_token_parser;

} OAuth2ClientCredential;

static bool curl_client_init(void);
static bool oauth2_login(void);
static void* background_renew_login(void* args);
static size_t ParseBufferCallback(void *contents, size_t size, size_t nmemb, void *userp);
static void cleanup_handler(void *arg);

static OAuth2ClientCredential* self = NULL;

bool OAuth2ClientCredential_create(const char* url, const char* client_id, const char* client_secret) 
{
  if (self) {
    fprintf(stderr, "OAuth2ClientCredential already initialized");
    return false;
  }

  size_t url_len = strlen(url);
  size_t client_id_len = strlen(client_id);
  size_t client_secret_len = strlen(client_secret);

  if (url_len <= 0 || url_len >= URL_MAX) {
    return false;
  }
  if (client_id_len <= 0 || client_id_len >= CLIENT_ID_MAX) {
    return false;
  }
  if (client_secret_len <= 0 || client_secret_len >= CLIENT_SECRET_MAX) {
    return false;
  }

  if (! (self = malloc(sizeof(OAuth2ClientCredential)))) {
    self = NULL;
    return false;
  }

  strncpy(self->url, url, url_len+1);
  strncpy(self->client_id, client_id, client_id_len+1);
  strncpy(self->client_secret, client_secret, client_secret_len+1);

  if (! (self->access_token_parser = Parser_create("\"access_token\":\"", ACCESS_TOKEN_MAX))) {
    OAuth2ClientCredential_destroy();
    return false;
  }

  if (! curl_client_init()) {
    OAuth2ClientCredential_destroy();
    return false;
  }

  if (pthread_mutex_init(&self->lock, NULL) != 0) { 
    OAuth2ClientCredential_destroy();
    return false;
  }

  if (! oauth2_login()) {
    OAuth2ClientCredential_destroy();
    return false;
  }

  if (pthread_create(&self->b_thread, NULL, background_renew_login, NULL) != 0) {
    OAuth2ClientCredential_destroy();
    return false;
  }

  return true;
}

static bool curl_client_init(void)
{
  /* get a curl handle */
  if (! (self->curl_session = curl_easy_init())) {
    return false;
  }
  curl_easy_setopt(self->curl_session, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(self->curl_session, CURLOPT_URL, self->url);
  /* send all data to this function  */
  curl_easy_setopt(self->curl_session, CURLOPT_WRITEFUNCTION, ParseBufferCallback);
  /* we pass our 'parser' struct to the callback function */
  curl_easy_setopt(self->curl_session, CURLOPT_WRITEDATA, (void *)self->access_token_parser);
  
  return true;
}

bool OAuth2ClientCredential_destroy(void)
{
  bool res = false;
  if (self) 
  {
    if (self->curl_session)
    {
      curl_easy_cleanup(self->curl_session);
      self->curl_session = NULL;
    }
    memset(self->url, 0, strlen(self->url));
    memset(self->client_id, 0, strlen(self->client_id));
    memset(self->client_secret, 0, strlen(self->client_secret));
    
    if (self->access_token_parser)
    {
      Parser_destroy(self->access_token_parser);
      self->access_token_parser = NULL;
    }
    
    int s = pthread_cancel(self->b_thread);
    if (s != 0)
      fprintf(stderr, "pthread_cancel\n");

    pthread_join(self->b_thread, NULL);
    pthread_mutex_destroy(&self->lock);

    free(self);
    self = NULL;
    res = true;
  }
  self = NULL;
  return res;
}

static bool oauth2_login(void)
{
  CURLcode res;
  char post_data[CLIENT_ID_MAX +  CLIENT_SECRET_MAX + 54] = "grant_type=client_credentials&";
  strcat(post_data, "client_id=");
  strcat(post_data, self->client_id);
  strcat(post_data, "&client_secret=");
  strcat(post_data, self->client_secret);
  
  curl_easy_setopt(self->curl_session, CURLOPT_POSTFIELDS, post_data);

  Parser_reset(self->access_token_parser);

  if ((res = curl_easy_perform(self->curl_session)) != CURLE_OK)
  {
    if (res == CURLE_HTTP_RETURNED_ERROR)
    {
      long http_code = 0;
      curl_easy_getinfo (self->curl_session, CURLINFO_RESPONSE_CODE, &http_code);
      fprintf(stderr, "HTTP request returned %ld\n", http_code);
    }
    else 
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    return false;
  }

  return true;
}

static size_t ParseBufferCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  const size_t realsize = size * nmemb;
  Parser* parser = (Parser*) userp;
  char* buffer = (char*) contents;
  bool loop = true;

  if (Parser_is_extraction_started(parser))
  {
    Parser_extract(parser, buffer, realsize);
    return realsize;
  }

  while (loop)
  {
    if (Parser_parse(parser, buffer, realsize)) 
    {
      if (Parser_extract(parser, buffer, realsize)) {
        loop = false;
      }
    } 
    else {
      loop = false;
    }
  }

  return realsize;
}

static void* background_renew_login(void* args)
{
  (void)args;
  bool login_result;

  pthread_cleanup_push(cleanup_handler, NULL);

  while (true)
  {
    pthread_testcancel(); // A cancelation point
    
    sleep(270); // 270 = 4 minutes and 30 seconds
    pthread_mutex_lock(&self->lock);

    printf("New login started\n");
    login_result = oauth2_login();
    printf("New login completed successfully\n");

    pthread_mutex_unlock(&self->lock);

    if (! login_result)
    {
      pthread_exit(NULL);
    }
  }

  pthread_cleanup_pop(1);
  pthread_exit(0);
}

static void cleanup_handler(void *arg)
{
  (void)arg;
  printf("Called clean-up handler\n");

  if (pthread_mutex_trylock(&self->lock) == 0)
  {
    printf("Clean-up handler: try lock\n");
  }
  pthread_mutex_unlock(&self->lock);
}

void get_access_token_copy(char access_token_copy[])
{
  pthread_mutex_lock(&self->lock);

  Parser_copy_extracted_data(self->access_token_parser, access_token_copy);

  pthread_mutex_unlock(&self->lock);
}

unsigned long get_access_token_len(void)
{
  return Parser_get_extracted_data_len(self->access_token_parser);
}
