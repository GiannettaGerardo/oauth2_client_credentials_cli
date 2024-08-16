#include "parser.h"
#include <string.h>
#include "utility.h"

#include <stdio.h>

struct parser {

  String* pattern;
  size_t idx;
  size_t loop_i;
  bool extraction_started;
  unsigned long access_token_idx;
  String* access_token;

};

Parser* Parser_create(const char* _pattern, const unsigned int extract_max)
{
  Parser* parser = NULL;

  if (! (parser = malloc(sizeof(Parser)))) {
    return NULL;
  }

  if (! (parser->access_token = String_new("", extract_max))) {
    Parser_destroy(parser);
    return NULL;
  }

  if (! (parser->pattern = String_new_2(_pattern))) {
    Parser_destroy(parser);
    return NULL;
  }

  parser->idx = 0;
  parser->loop_i = 0;
  parser->access_token_idx = 0;
  parser->extraction_started = false;

  return parser;
}

bool Parser_destroy(Parser* parser) 
{
  bool res = false;
  if (parser)
  {
    if (parser->access_token)
    {
      String_free(parser->access_token);
      parser->access_token = NULL;
    }

    if (parser->pattern)
    {
      String_free(parser->pattern);
      parser->pattern = NULL;
    }

    free(parser);
    res = true;
  }
  return res;
}

void Parser_reset(Parser* parser) 
{
  String_zero(parser->access_token);
  parser->idx = 0;
  parser->loop_i = 0;
  parser->access_token_idx = 0;
  parser->extraction_started = false;
}

bool Parser_parse(Parser* parser, char* buffer, size_t buffer_size)
{
  char* pattern = parser->pattern->buffer;
  parser->loop_i = 0;

  for (; parser->loop_i < buffer_size; ++(parser->loop_i))
  {
    if (buffer[parser->loop_i] == pattern[parser->idx])
    {
      parser->idx++;
    }
    else if (parser->idx == parser->pattern->length)
    {
      parser->extraction_started = true;
      return true;
    }
    else 
    {
      parser->idx = 0;
    }
  }
  return false;
}

// Aggiungendo un terzo valore di ritorno per quando ho davvero finito di estrarre, posso killare la funzione di callback in curl
bool Parser_extract(Parser* parser, char* buffer, size_t buffer_size)
{
  char* access_token = parser->access_token->buffer;
  unsigned long j = parser->access_token_idx;

  for (; parser->loop_i < buffer_size && buffer[parser->loop_i] != '"'; ++(parser->loop_i))
  {
    access_token[j++] = buffer[parser->loop_i];
  }
  // L'estrazione non è finita
  if (buffer[parser->loop_i] != '"')
  {
    parser->access_token_idx = j;
    return false;
  }
  // Quando l'estrazione è finita, non serve porre il byte 0 alla fine della stringa 
  // perché si suppone che la stringa sia sempre azzerata completamente.
  parser->access_token->length = j - 1;
  return true;
}

bool Parser_is_extraction_started(Parser* parser)
{
  return parser->extraction_started;
}

void Parser_set_extraction_started(Parser* parser, bool value)
{
  parser->extraction_started = value;
}

void Parser_copy_extracted_data(Parser* parser, char buffer[])
{
  strncpy(buffer, parser->access_token->buffer, parser->access_token->length + 1);
}

unsigned long Parser_get_extracted_data_len(Parser* parser)
{
  return parser->access_token->length;
}
