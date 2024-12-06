
#pragma once

#ifdef _WIN32
#define socklen_t int
#else

#endif

#define FALSE	0

#define TRUE	1

#ifndef INADDR_ANY
#define INADDR_ANY 0
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#define OBJECT_USERNAME	"root"
#define OBJECT_PASSWORD	"pass"



