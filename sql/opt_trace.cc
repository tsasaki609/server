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


ST_FIELD_INFO optimizer_trace_info[] = {
    /* name, length, type, value, maybe_null, old_name, open_method */
    {"QUERY", 65535, MYSQL_TYPE_STRING, 0, false, NULL, SKIP_OPEN_TABLE},
    {"TRACE", 65535, MYSQL_TYPE_STRING, 0, false, NULL, SKIP_OPEN_TABLE},
    {"MISSING_BYTES_BEYOND_MAX_MEM_SIZE", 20, MYSQL_TYPE_LONG, 0, false, NULL,
     SKIP_OPEN_TABLE},
    {"INSUFFICIENT_PRIVILEGES", 1, MYSQL_TYPE_TINY, 0, false, NULL,
     SKIP_OPEN_TABLE},
    {NULL, 0, MYSQL_TYPE_STRING, 0, true, NULL, 0}};

class Opt_trace_stmt {
 public:
  /**
     Constructor, starts a trace for information_schema and dbug.
     @param  ctx_arg          context
  */
  Opt_trace_stmt(Opt_trace_context *ctx_arg)
  {

  }
};

Opt_trace_context::Opt_trace_context()
{

}
Opt_trace_context::~Opt_trace_context()
{
  
}

void Opt_trace_context::start()
{

}

void Opt_trace_context::end()
{

}

Opt_trace_start::Opt_trace_start(THD *thd, TABLE_LIST *tbl,
                  enum enum_sql_command sql_command,
                  const char *query,
                  size_t query_length,
                  const CHARSET_INFO *query_charset):ctx(&thd->opt_trace)
{
  return;
}

Opt_trace_start::~Opt_trace_start()
{
  return;
}


int fill_optimizer_trace_info(THD *thd, TABLE_LIST *tables, Item *)
{
  return 0;
}