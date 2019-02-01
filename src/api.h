#ifndef NETPLEX_API_H
#define NETPLEX_API_H

#ifdef __GNUC__
#define NETPLEX_UNUSED(type, name) __attribute__((unused)) type name
#else
#define NETPLEX_UNUSED(type, name)
#endif

#endif /* NETPLEX_API_H */
