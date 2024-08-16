#ifndef _CUSTOM_PARSER_H_
#define _CUSTOM_PARSER_H_

#include <stdlib.h>
#include <stdbool.h>

typedef struct parser Parser;

Parser* Parser_create(const char* pattern, const unsigned int extract_max);
bool Parser_destroy(Parser* parser);
void Parser_reset(Parser* parser);

bool Parser_parse(Parser* parser, char* buffer, size_t buffer_size);
bool Parser_is_extraction_started(Parser* parser);
void Parser_set_extraction_started(Parser* parser, bool value);
bool Parser_extract(Parser* parser, char* buffer, size_t buffer_size);
void Parser_copy_extracted_data(Parser* parser, char buffer[]);
unsigned long Parser_get_extracted_data_len(Parser* parser);

#endif
