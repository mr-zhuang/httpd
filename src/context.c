#include <stdlib.h>
#include <string.h>

#include "context.h"

#define RAW_DATA_INIT_SIZE 512

struct context* newContext(int clientFd) {
  struct context* ctx = (struct context*)malloc(sizeof(struct context));
  ctx->clientFd = clientFd;
  ctx->header = newMap();

  ctx->rawHeader = newGrowData(RAW_DATA_INIT_SIZE);
  ctx->rawBody = newGrowData(RAW_DATA_INIT_SIZE);

  return ctx;
}

int cleanContext(struct context* ctx) {
  cleanMap(ctx->header);

  free(ctx->method);
  free(ctx->path);
  free(ctx->protocol);

  cleanGrowData(ctx->rawHeader);
  cleanGrowData(ctx->rawBody);

  free(ctx);

  return 0;
}
