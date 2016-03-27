/**
* Just a small experimental program to test
* json-c lib with github API
*
* Copyright (C) 2016 Bhavyanshu Parasher <mail@bhavyanshu.me>
*
* This program is free software. You are free to use, modify, distribute this code
* for commercial non-commercial purposes. This program is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY.
*/
#include "helper.h"

fetch_all(char * res) {
  struct json_object *initobj = json_tokener_parse(res);
  enum json_type type;
  json_object *jarray = initobj;

  int arraylen = json_object_array_length(jarray); /*Getting the length of the array*/
  int i;
  json_object * jvalue;

  for (i=0; i< arraylen; i++){
    jvalue = json_object_array_get_idx(jarray, i);
    type = json_object_get_type(jvalue);
    if (type == json_type_array) {
      //future use
    }
    else if (type != json_type_object) {
      //future use
    }
    else {
      json_object *sha_obj, *message_obj, *commit_obj, *author_obj;
      if(json_object_object_get_ex(jvalue,"sha",&sha_obj)) {
        const char *sha;
        sha = json_object_get_string(sha_obj);
        printf("\n\n" ANSI_COLOR_GREEN "[SHA] " ANSI_COLOR_RESET " %s",sha);
      }
      if(json_object_object_get_ex(jvalue,"commit",&commit_obj)) {
        if(json_object_object_get_ex(commit_obj,"author",&author_obj)) {
          json_parse(author_obj);
        }
        if(json_object_object_get_ex(commit_obj,"message",&message_obj)) {
          char *message;
          char *pos;
          message = (char *) json_object_get_string(message_obj);
          message[60] = 0;
          if ((pos=strchr(message, '\n')) != NULL)
          *pos = '\0';
          printf("\n - commit message : %s\n",message);
        }
      }
    }
  }
  return 0;
}

fetch_single(char * res) {
  struct json_object *initobj = json_tokener_parse(res);

  json_object *message_obj, *commit_obj, *author_obj, *files_obj, *patch_obj;
  if(json_object_object_get_ex(initobj,"commit",&commit_obj)) {
    if(json_object_object_get_ex(commit_obj,"author",&author_obj)) {
      //const char *author;
      //author = json_object_get_string(author_obj);
      printf("\n\n **************** Author ****************");
      json_parse(author_obj);
    }
    if(json_object_object_get_ex(commit_obj,"message",&message_obj)) {
      char *message;
      message = (char *) json_object_get_string(message_obj);
      message[60] = 0;
      printf("\n\n **************** Message ****************\n %s\n",message);
    }
    printf("\n **************** Files ****************\n");
    json_parse_array(initobj, "files");
  }
  return 0;
}

main()
{
  printf("*** Github API in C (using json-c) ***\n");

  char username[200] = "torvalds";
  char reponame[200] = "linux";

  char endpoint_all_commits[200] = "/commits?page=";
  char endpoint_single_commit[200] = "/commits/";

  char finalurl_all_commits[200] = "";
  strcat(finalurl_all_commits,strcat(strcat(strcat(strcat(getbaseurl(), username),"/"),reponame),endpoint_all_commits));

  char finalurl_single_commit[200] = "";
  strcat(finalurl_single_commit,strcat(strcat(strcat(strcat(getbaseurl(), username),"/"),reponame),endpoint_single_commit));

  struct string s;
  int input;

  while(1) {
    printf("\n 1. All Commits \n 2. Single Commit \n 3. Exit \n Choose an option:");
    scanf("%d", &input);
    switch(input) {
      case 1:
      {
        int pageval = 1;
        char pagestr[10];
        char geturl[200];

        sprintf(pagestr,"%d",pageval);
        strcpy(geturl,finalurl_all_commits);
        strcat(geturl,pagestr);
        printf("%s",geturl);
        char *result = setupcurl(s,geturl);
        printf("\n\n" ANSI_COLOR_YELLOW  "\t******** Page %d ********" ANSI_COLOR_RESET "\n", pageval);
        fetch_all(result);
        int check = 1;
        while(check == 1) {
          printf("\n What would you like to do? \n1. << Previous Page \t 2. Next Page >> \n3. Go back to main menu:");
          int newinput;
          scanf(" %d",&newinput);
          switch(newinput) {
            case 1:
            {
              int tmp_pageval = pageval;
              tmp_pageval--;
              pageval = tmp_pageval;
              if(pageval < 0) {
                pageval = 0;
              }
              sprintf(pagestr,"%d",pageval);
              strcpy(geturl,finalurl_all_commits);
              strcat(geturl,pagestr);
              char *result = setupcurl(s,geturl);
              printf("\n\n" ANSI_COLOR_YELLOW  "\t******** Page %d ********" ANSI_COLOR_RESET "\n", pageval);
              fetch_all(result);
              break;
            }
            case 2:
            {
              int tmp_pageval = pageval;
              tmp_pageval++;
              pageval = tmp_pageval;
              sprintf(pagestr,"%d",pageval);
              strcpy(geturl,finalurl_all_commits);
              strcat(geturl,pagestr);
              char *result = setupcurl(s,geturl);
              printf("\n\n" ANSI_COLOR_YELLOW  "\t******** Page %d ********" ANSI_COLOR_RESET "\n", pageval);
              fetch_all(result);
              break;
            }
            case 3:
              check = 0;
              break;
          }
        }
        break;
      }
      case 2:
      {
        char geturln[200];
        strcpy(geturln,finalurl_single_commit);

        char sha1[100];
        printf("\nEnter the SHA1 for the commit :");
        scanf("%s", sha1);
        strcat(geturln,sha1);

        char *result = setupcurl(s,geturln);

        fetch_single(result);
        break;
      }
      default:
        curl_global_cleanup();
        release_pointer(s);
        exit(0);
    }
  }
  return 0;
}
