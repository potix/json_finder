#include <mysql/mysql.h>

my_bool
jfget_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  if (args->arg_count != 2) {
    strcpy(message,"XXX() requires two arguments");
    return 1;
  }

}

void
jfget_deinit(UDF_INIT *initid)
{

}
 

