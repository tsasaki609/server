#ifndef OPT_TRACE_CONTEXT_INCLUDED
#define OPT_TRACE_CONTEXT_INCLUDED
class Opt_trace_stmt;

class Opt_trace_context
{
public:
   Opt_trace_context();
  ~Opt_trace_context();
void start();
void end();
private:
  Dynamic_array<Opt_trace_stmt* > traces;
  Opt_trace_stmt *current_trace;
};
#endif
