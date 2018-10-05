#ifndef OPT_TRACE_INCLUDED
#define OPT_TRACE_INCLUDED
/* This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "opt_trace_context.h"  // Opt_trace_context

class Item;
class THD;
struct TABLE_LIST;

class Opt_trace_stmt;
/*
   User-visible information about a trace.
*/
struct Opt_trace_info {
  /**
     String containing trace.
     If trace has been end()ed, this is 0-terminated, which is only to aid
     debugging or unit testing; this property is not relied upon in normal
     server usage.
     If trace has not been ended, this is not 0-terminated. That rare case can
     happen when a substatement reads OPTIMIZER_TRACE (at that stage, the top
     statement is still executing so its trace is not ended yet, but may still
     be read by the sub-statement).
  */
  const char *trace_ptr;
  size_t trace_length;  ///< length of trace string
  //// String containing original query. 0-termination: like trace_ptr.
  const char *query_ptr;
  size_t query_length;                ///< length of query string
  const CHARSET_INFO *query_charset;  ///< charset of query string
  /**
    How many bytes this trace is missing (for traces which were truncated
    because of @@@@optimizer-trace-max-mem-size).
  */
  size_t missing_bytes;
  bool missing_priv;  ///< whether user lacks privilege to see this trace
};

class Opt_trace_start {
 public:
  Opt_trace_start(THD *thd_arg, TABLE_LIST *tbl,
                  enum enum_sql_command sql_command,
                  const char *query,
                  size_t query_length,
                  const CHARSET_INFO *query_charset);
  ~Opt_trace_start();

 private:
  Opt_trace_context *const ctx;
  bool error;  ///< whether trace start() had an error
};

int fill_optimizer_trace_info(THD *thd, TABLE_LIST *tables, Item *);
#endif