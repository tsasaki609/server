#ifndef OPT_TRACE_CONTEXT_INCLUDED
#define OPT_TRACE_CONTEXT_INCLUDED

#include "sql_array.h"

class Opt_trace_stmt;

class Opt_trace_context
{
public:
   Opt_trace_context();
  ~Opt_trace_context();
void start(THD *thd, TABLE_LIST *tbl,
           enum enum_sql_command sql_command,
           const char *query,
           size_t query_length,
           const CHARSET_INFO *query_charset);
void end();
void set_query(const char *query, size_t length, const CHARSET_INFO *charset);
Opt_trace_stmt* top_trace(){ return *(traces->front());}
Opt_trace_stmt* get_current_trace(){ return top_trace();}
Opt_trace_stmt* get_latest_trace(){return current_trace;}
static const char *flag_names[];
enum { FLAG_DEFAULT = 0, FLAG_ENABLED = 1 << 0, FLAG_ONE_LINE = 1 << 1 };
private:
  Dynamic_array<Opt_trace_stmt*> *traces;
  Opt_trace_stmt *current_trace;
  Opt_trace_stmt *prev_trace;
  
  bool inited;
};
#endif
