# Create a table on master and slave possibly with imcompatible attributes;
# INSERT into the table and replicate;
# print out inserted values.
--connection master
--eval DROP TABLE IF EXISTS $table;
sync_slave_with_master slave;

--connection master
--eval CREATE TABLE $table ($attr_m)
--replace_result default DEFAULT
--eval SHOW CREATE TABLE $table

sync_slave_with_master slave;
--eval ALTER TABLE $table MODIFY $attr_s
--replace_result default DEFAULT
--eval SHOW CREATE TABLE $table

--connection master
--eval INSERT INTO t1 VALUES $values2
--eval SELECT $check_m from $table

sync_slave_with_master slave;
--eval SELECT $check_s from $table
