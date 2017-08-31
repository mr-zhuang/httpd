/*
*	Copyright © 2017 Joe Xu <joe.0x01@gmail.com>
*	This work is free. You can redistribute it and/or modify it under the
*	terms of the Do What The Fuck You Want To Public License, Version 2,
*	as published by Sam Hocevar. See the COPYING file for more details.
*
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "httpd.h"

#define LISTEN_ADDR "127.0.0.1"
#define LISTEN_PORT 1234
#define STATIC_FILE_DIR "./static"
#define CGI_DIR "./cgi-bin"

int main(void) {
  int listenFd = setupListener();

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);

  pthread_t thread;
  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

  while (1) {
    int clientFd =
        accept(listenFd, (struct sockaddr*)&clientAddr, &clientAddrLen);

    printf("hi %d\n", clientFd);
    if (pthread_create(&thread, &threadAttr, handleRequest, &clientFd) != 0) {
      printf("create thread fail\n");
    }
    // handleRequest(&clientFd);
  }
  pthread_attr_destroy(&threadAttr);
  close(listenFd);

  return 0;
}

int setupListener() {
  int listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in listenAddr;
  memset(&listenAddr, 0, sizeof(listenAddr));
  listenAddr.sin_family = AF_INET;
  listenAddr.sin_addr.s_addr = inet_addr(LISTEN_ADDR);
  listenAddr.sin_port = htons(LISTEN_PORT);

  bind(listenFd, (struct sockaddr*)&listenAddr, sizeof(listenAddr));

  listen(listenFd, 20);
  return listenFd;
}

void* handleRequest(void* clientFd) {
  struct Context* ctx = newContext(*(int*)clientFd);

  struct Respone* resp;

  HandleFuncPtr handler = staticFile;
  resp = handler(ctx);

  encodeRespone(resp);

  sendRespone(resp, ctx->clientFd);

  close(ctx->clientFd);
  cleanRespone(resp);
  printf("bye %d\n", ctx->clientFd);
  cleanContext(ctx);
}

struct Respone* staticFile(struct Context* ctx) {
  struct Respone* resp = newRespone(STAT_OK);

  char filePath[32];

  readRequest(ctx);
  // printf("header:\n%s\n", ctx->rawHeader->data);
  // printf("body:\n%s\n", ctx->rawBody->data);
  // printf("====end====\n");
  parseHeader(ctx);

  printf("request url:%s\n", ctx->url);
  // printMap(ctx->header);

  resp->protocol = ctx->protocol;
  resp->message = "OK";
  setMap(resp->header, "Content-type", strdup("text/html"));
  setMap(resp->header, "mheader", strdup("mval"));

  if (!ctx->url) {
    printf("[warn] NULL url\n");
  } else if (strcmp("/", ctx->url) == 0) {
    printf("index\n");
  } else {
    sprintf(filePath, "%s%s", STATIC_FILE_DIR, ctx->url);
    if (readFile(filePath, resp->body) < 0) {
      resp->statusCode = STAT_NOT_FOUND;
    }
  }

  return resp;
}
