#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include    "feature.h"
#define SERVER_FEATURE_DISTRIBUTE_PORT 60721
#define SERVER_FEATURE_DISTRIBUTE_IPV4 "192.168.182.133"


typedef void (*feature_handler)(const unsigned char *, struct feature*);

int init_receive_feature_service();
int dispatch();
#endif