/**
* helper functions for json-c and string manipulation
*
* Copyright (C) 2016 Bhavyanshu Parasher <mail@bhavyanshu.me>
*
* This program is free software. You are free to use, modify, distribute this code
* for commercial non-commercial purposes. This program is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY.
*/
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "Error malloc() \n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

void release_pointer(struct string s) {
  s.ptr = NULL;
  free(s.ptr);
}

size_t write_func(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "Error realloc()\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
  return size*nmemb;
}

char * getbaseurl() {
    char baseurl[200] = "https://api.github.com/repos/";
    char *str = (char *) malloc(200);
    return strcpy(str,baseurl);
}

char * setupcurl(struct string s, char *url) {
  init_string(&s);
  CURL *curl;
  CURLcode res;
  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Rambo/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    res = curl_easy_perform(curl);
    /* Check errors */
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    return s.ptr;
    curl_easy_cleanup(curl);
  }
}

/**
 * JSON Helper functions
 * With help from (https://linuxprograms.wordpress.com/2010/08/19/json_parser_json-c/)
 */
void print_json_value(json_object *jobj){
  enum json_type type;
  type = json_object_get_type(jobj);
  switch (type) {
    case json_type_boolean:
    printf(" %s", json_object_get_boolean(jobj)? "true": "false");
    break;
    case json_type_double:
    printf(" %lf", json_object_get_double(jobj));
    break;
    case json_type_int:
    printf(" %d", json_object_get_int(jobj));
    break;
    case json_type_string:
    {
      char * strin;
      strin = (char *) json_object_get_string(jobj);
      printf("\n %s", strin); // TODO: "patch" needs to be tokenized by "+" and "-" and highlighted with color 
      break;
    }
  }
}

void json_parse_array(json_object *jobj, char *key) {
  void json_parse(json_object * jobj); /*Forward Declaration*/
  enum json_type type;

  json_object *jarray = jobj; /*First get array*/
  if(key) {
    jarray = json_object_object_get(jobj, key); /*Get array if it is a key value pair*/
  }

  int arraylen = json_object_array_length(jarray); /*Getting the length of the array*/
  int i;

  json_object * jvalue;

  for (i=0; i< arraylen; i++){
    jvalue = json_object_array_get_idx(jarray, i); /*Getting the array element at position i*/
    type = json_object_get_type(jvalue);
    if (type == json_type_array) {
      json_parse_array(jvalue, NULL);
    }
    else if (type != json_type_object) {
      print_json_value(jvalue);
    }
    else {
      json_parse(jvalue);
    }
  }
}

/*Parsing the json object*/
void json_parse(json_object * jobj) {
  enum json_type type;
  json_object_object_foreach(jobj, key, val) { /*Passing through every array element*/
    //printf("type: ",type);
    type = json_object_get_type(val);
    switch (type) {
      case json_type_boolean:
      case json_type_double:
      case json_type_int:
      case json_type_string:
        printf("\n -- %s",key);
        print_json_value(val);
        break;
      case json_type_object:
      {
        json_object *returnObj1;
        if(json_object_object_get_ex(jobj, key, &returnObj1))
        {
          printf("\n\n ********** \n -- %s",key);
          json_parse(returnObj1);
        }
        break;
      }
      case json_type_array:
      {
        printf("\n ********** \n\n -- %s",key);
        json_parse_array(jobj, key);
        break;
      }
    }
  }
}
