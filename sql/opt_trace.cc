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

#include "mariadb.h"
#include "sql_array.h"
#include "sql_string.h"
#include "sql_class.h"
#include "sql_show.h"
#include "field.h"
#include "table.h"
#include "opt_trace.h"
#include "sql_parse.h"

const char I_S_table_name[] = "OPTIMIZER_TRACE";

/*

Take a decision on this one

template <class T>
T *new_nothrow_w_my_error() {
  T *const t = new (std::nothrow) T();
  if (unlikely(t == NULL))
    my_error(ER_OUTOFMEMORY, MYF(ME_FATAL), static_cast<int>(sizeof(T)));
  return t;
}
template <class T, class Arg>
T *new_nothrow_w_my_error(Arg a) {
  T *const t = new (std::nothrow) T(a);
  if (unlikely(t == NULL))
    my_error(ER_OUTOFMEMORY, MYF(ME_FATAL), static_cast<int>(sizeof(T)));
  return t;
}
*/

/**
   Whether a list of tables contains information_schema.OPTIMIZER_TRACE.
   @param  tbl  list of tables
   @note this does not catch that a stored routine or view accesses
   the OPTIMIZER_TRACE table. So using a stored routine or view to read
   OPTIMIZER_TRACE will overwrite OPTIMIZER_TRACE as it runs and provide
   uninteresting info.
*/
bool list_has_optimizer_trace_table(const TABLE_LIST *tbl) {
  for (; tbl; tbl = tbl->next_global) {
    if (tbl->schema_table &&
        0 == strcmp(tbl->schema_table->table_name, I_S_table_name))
      return true;
  }
  return false;
}

ST_FIELD_INFO optimizer_trace_info[] = {
    /* name, length, type, value, maybe_null, old_name, open_method */
    {"QUERY", 65535, MYSQL_TYPE_STRING, 0, false, NULL, SKIP_OPEN_TABLE},
    {"TRACE", 65535, MYSQL_TYPE_STRING, 0, false, NULL, SKIP_OPEN_TABLE},
    {"MISSING_BYTES_BEYOND_MAX_MEM_SIZE", 20, MYSQL_TYPE_LONG, 0, false, NULL,
     SKIP_OPEN_TABLE},
    {"INSUFFICIENT_PRIVILEGES", 1, MYSQL_TYPE_TINY, 0, false, NULL,
     SKIP_OPEN_TABLE},
    {NULL, 0, MYSQL_TYPE_STRING, 0, true, NULL, 0}};

const char *Opt_trace_context::flag_names[] = {"enabled", "one_line", "default",
                                               NullS};

/*
  To check which commands to be traced
*/

inline bool sql_command_can_be_traced(enum enum_sql_command sql_command)
{
  return (sql_command_flags[sql_command] & CF_OPTIMIZER_TRACE);
}

class Opt_trace_stmt {
 public:
  /**
     Constructor, starts a trace for information_schema and dbug.
     @param  ctx_arg          context
  */
  Opt_trace_stmt(Opt_trace_context *ctx_arg)
  {
    ctx= ctx_arg;
  }
  ~Opt_trace_stmt()
  {
    query.free();
    trace.free();
  }
  void set_query(const char *query_ptr, size_t length, const CHARSET_INFO *charset);
  void fill_info(Opt_trace_info* info);
private:
  Opt_trace_context *ctx;
  String query;
  String trace;
};


void Opt_trace_stmt::set_query(const char *query_ptr, size_t length, const CHARSET_INFO *charset)
{
  query.append(query_ptr, length, charset);
  trace.append("hello world", 11, system_charset_info);
}

Opt_trace_context::Opt_trace_context()
{
  current_trace= NULL;
  prev_trace= NULL;
  inited= FALSE;
  traces= NULL;
}
Opt_trace_context::~Opt_trace_context()
{
  inited= FALSE;
  delete current_trace;
  current_trace= NULL;
  if (traces)
    delete traces;
}

void Opt_trace_context::set_query(const char *query, size_t length, const CHARSET_INFO *charset)
{
  current_trace->set_query(query, length, charset);
}

void Opt_trace_context::start(THD *thd, TABLE_LIST *tbl,
                  enum enum_sql_command sql_command,
                  const char *query,
                  size_t query_length,
                  const CHARSET_INFO *query_charset)
{
  current_trace= new Opt_trace_stmt(this);
  if (!traces)
    traces= new Dynamic_array<Opt_trace_stmt*>();
  traces->push(current_trace);
}

void Opt_trace_context::end()
{
  if (!traces->elements())
    return;
  if (traces->elements() > 1)
  {
    Opt_trace_stmt *prev= traces->at(0);
    delete prev;
    traces->del(0);
  }
}

Opt_trace_start::Opt_trace_start(THD *thd, TABLE_LIST *tbl,
                  enum enum_sql_command sql_command,
                  const char *query,
                  size_t query_length,
                  const CHARSET_INFO *query_charset):ctx(&thd->opt_trace)
{
  /*
    if optimizer trace is enabled and the statment we have is traceable,
    then we start the context.
  */
  const ulonglong var = thd->variables.optimizer_trace;
  error= TRUE;
  if (unlikely(var & Opt_trace_context::FLAG_ENABLED) && sql_command_can_be_traced(sql_command) && 
      !list_has_optimizer_trace_table(tbl))
  {
    ctx->start(thd, tbl, sql_command, query, query_length, query_charset);
    ctx->set_query(query, query_length, query_charset);
    error= FALSE;
  }
}

Opt_trace_start::~Opt_trace_start()
{
  if (!error)
    ctx->end();
}

void Opt_trace_stmt::fill_info(Opt_trace_info* info)
{
  info->trace_ptr = trace.ptr();
  info->trace_length = trace.length();
  info->query_ptr = query.ptr();
  info->query_length = query.length();
  info->query_charset = query.charset();
  info->missing_bytes = 0;
  info->missing_priv= 0;
}


void get_info(THD *thd, Opt_trace_info* info)
{
  Opt_trace_context* ctx= &thd->opt_trace;
  Opt_trace_stmt *stmt= ctx->get_current_trace();
  stmt->fill_info(info);
}


int fill_optimizer_trace_info(THD *thd, TABLE_LIST *tables, Item *)
{
  TABLE *table = tables->table;
  Opt_trace_info info;  

  //  get_values of trace, query , missing bytes and missing_priv  
  get_info(thd, &info);

  table->field[0]->store(info.query_ptr, static_cast<uint>(info.query_length),
                           info.query_charset);
  table->field[1]->store(info.trace_ptr, static_cast<uint>(info.trace_length),
                           system_charset_info);
  table->field[2]->store(info.missing_bytes, true);
  table->field[3]->store(info.missing_priv, true);
  
  //  Store in IS
  if (schema_table_store_record(thd, table)) return 1;
  return 0;
}