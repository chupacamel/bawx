/**********************************************************************
 * Copyright (c) 2004, Leo Seib, Hannover
 *
 * Project:SQLiteDataset C++ Dynamic Library
 * Module: SQLiteDataset class realisation file
 * Author: Leo Seib      E-Mail: leoseib@web.de 
 * Begin: 5/04/2002
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **********************************************************************/

#include <iostream>
#include <string>

#include "sqlitedataset.h"
#include "utils/log.h"
#include "utils/CriticalSection.h"
#include "system.h" // for Sleep(), OutputDebugString() and GetLastError()

using namespace std;

static CCriticalSection g_lock;

namespace dbiplus {
//************* Callback function ***************************

int callback(void* res_ptr,int ncol, char** reslt,char** cols)
{
  result_set* r = (result_set*)res_ptr;

  if (!r->record_header.size())
  {
    r->record_header.reserve(ncol);
    for (int i=0; i < ncol; i++) {
      field_prop header;
      header.name = cols[i];
      r->record_header.push_back(header);
    }
  }

  if (reslt != NULL)
  {
    sql_record *rec = new sql_record;
    rec->resize(ncol);
    for (int i=0; i<ncol; i++)
    { 
      field_value &v = rec->at(i);
      if (reslt[i] == NULL)
      {
        v.set_asString("");
        v.set_isNull();
      }
      else
      {
        v.set_asString(reslt[i]);
      }
    }
    r->records.push_back(rec);
  }
  return 0;  
}

//************* SqliteDatabase implementation ***************

SqliteDatabase::SqliteDatabase() {

  active = false;	
  _in_transaction = false;		// for transaction

  error = "Unknown database error";//S_NO_CONNECTION;
  host = "localhost";
  port = "";
  db = "sqlite.db";
  login = "root";
  passwd = "";
}

SqliteDatabase::~SqliteDatabase() {
  disconnect();
}


Dataset* SqliteDatabase::CreateDataset() const {
	return new SqliteDataset((SqliteDatabase*)this); 
}

int SqliteDatabase::status(void) {
  if (active == false) return DB_CONNECTION_NONE;
  return DB_CONNECTION_OK;
}

int SqliteDatabase::setErr(int err_code, const char * qry){
  switch (err_code) {
  case SQLITE_OK: error ="Successful result";
    break;
  case SQLITE_ERROR: error = "SQL error or missing database";
    break;
  case SQLITE_INTERNAL: error = "An internal logic error in SQLite";
    break;
  case SQLITE_PERM: error ="Access permission denied";
    break;
  case SQLITE_ABORT: error = "Callback routine requested an abort";
    break;
  case SQLITE_BUSY: error = "The database file is locked";
    break;
  case SQLITE_LOCKED: error = "A table in the database is locked";
    break;
  case SQLITE_NOMEM: error = "A malloc() failed";
    break;
  case SQLITE_READONLY: error = "Attempt to write a readonly database";
    break;
  case SQLITE_INTERRUPT: error = "Operation terminated by sqlite_interrupt()";
    break;
  case  SQLITE_IOERR: error = "Some kind of disk I/O error occurred";
    break;
  case  SQLITE_CORRUPT: error = "The database disk image is malformed";
    break;
  case SQLITE_NOTFOUND: error = "(Internal Only) Table or record not found";
    break;
  case SQLITE_FULL: error = "Insertion failed because database is full";
    break;
  case SQLITE_CANTOPEN: error = "Unable to open the database file";
    break;
  case SQLITE_PROTOCOL: error = "Database lock protocol error";
    break;
  case SQLITE_EMPTY:  error = "(Internal Only) Database table is empty";
    break;
  case SQLITE_SCHEMA: error = "The database schema changed";
    break;
  case SQLITE_TOOBIG: error = "Too much data for one row of a table";
    break;
  case SQLITE_CONSTRAINT: error = "Abort due to constraint violation";
    break;
  case SQLITE_MISMATCH:  error = "Data type mismatch";
    break;
  default : error = "Undefined SQLite error";
  }
  error += "\nQuery: ";
  error += qry;
  error += "\n";
  return err_code;
}

const char *SqliteDatabase::getErrorMsg() {
   return error.c_str();
}

int SqliteDatabase::connect() {
  try {
    disconnect();
    if (sqlite3_open(db.c_str(),&conn/*,NULL*/)==SQLITE_OK) {
      //cout << "Connected!\n";
      sqlite3_busy_timeout(conn, 3000);
      char* err=NULL;
      if (setErr(sqlite3_exec(getHandle(),"PRAGMA empty_result_callbacks=ON",NULL,NULL,&err),"PRAGMA empty_result_callbacks=ON") != SQLITE_OK) {
        throw DbErrors("%s", getErrorMsg());
      }
      active = true;
      return DB_CONNECTION_OK;
    }
    CLog::Log(LOGERROR, "unable to open database:%s (%u)",
              db.c_str(), GetLastError());
    return DB_CONNECTION_NONE;
  }
  catch(...)
  {
    CLog::Log(LOGERROR, "unable to open database:%s (%u)",
              db.c_str(), GetLastError());
  }
  return DB_CONNECTION_NONE;
}

void SqliteDatabase::disconnect(void) {
  if (active == false) return;
  sqlite3_close(conn);
  active = false;
}

int SqliteDatabase::create() {
  return connect();
}

int SqliteDatabase::drop() {
  if (active == false) throw DbErrors("Can't drop database: no active connection...");
  disconnect();
  if (!unlink(db.c_str())) {
     throw DbErrors("Can't drop database: can't unlink the file %s,\nError: %s",db.c_str(),strerror(errno));
     }
  return DB_COMMAND_OK;
}


long SqliteDatabase::nextid(const char* sname) {
  if (!active) return DB_UNEXPECTED_RESULT;
  int id;/*,nrow,ncol;*/
  result_set res;
  char sqlcmd[512];
  sprintf(sqlcmd,"select nextid from %s where seq_name = '%s'",sequence_table.c_str(), sname);
  if ((last_err = sqlite3_exec(getHandle(),sqlcmd,&callback,&res,NULL)) != SQLITE_OK) {
    return DB_UNEXPECTED_RESULT;
    }
  if (res.records.size() == 0) {
    id = 1;
    sprintf(sqlcmd,"insert into %s (nextid,seq_name) values (%d,'%s')",sequence_table.c_str(),id,sname);
    if ((last_err = sqlite3_exec(conn,sqlcmd,NULL,NULL,NULL)) != SQLITE_OK) return DB_UNEXPECTED_RESULT;
    return id;
  }
  else {
    id = res.records[0]->at(0).get_asInt()+1;
    sprintf(sqlcmd,"update %s set nextid=%d where seq_name = '%s'",sequence_table.c_str(),id,sname);
    if ((last_err = sqlite3_exec(conn,sqlcmd,NULL,NULL,NULL) != SQLITE_OK)) return DB_UNEXPECTED_RESULT;
    return id;
  }
  return DB_UNEXPECTED_RESULT;
}


// methods for transactions
// ---------------------------------------------
void SqliteDatabase::start_transaction() {
  if (active) {
    sqlite3_exec(conn,"begin IMMEDIATE",NULL,NULL,NULL);
    _in_transaction = true;
  }
}

void SqliteDatabase::commit_transaction() {
  if (active) {
    sqlite3_exec(conn,"commit",NULL,NULL,NULL);
    _in_transaction = false;
  }
}

void SqliteDatabase::rollback_transaction() {
  if (active) {
    sqlite3_exec(conn,"rollback",NULL,NULL,NULL);
    _in_transaction = false;
  }  
}



//************* SqliteDataset implementation ***************

SqliteDataset::SqliteDataset():Dataset() {
  haveError = false;
  db = NULL;
  errmsg = NULL;
  autorefresh = false;
}


SqliteDataset::SqliteDataset(SqliteDatabase *newDb):Dataset(newDb) {
  haveError = false;
  db = newDb;
  errmsg = NULL;
  autorefresh = false;
}

 SqliteDataset::~SqliteDataset(){
   if (errmsg) sqlite3_free(errmsg);
 }


void SqliteDataset::set_autorefresh(bool val){
    autorefresh = val;
}



//--------- protected functions implementation -----------------//

sqlite3* SqliteDataset::handle(){
  if (db != NULL){
    return static_cast<SqliteDatabase*>(db)->getHandle();
      }
  else return NULL;
}

void SqliteDataset::make_query(StringList &_sql) {
  string query;

 try {

  if (db == NULL) throw DbErrors("No Database Connection");
  if (autocommit) db->start_transaction();


  for (list<string>::iterator i =_sql.begin(); i!=_sql.end(); i++) {
	query = *i;
	char* err=NULL; 
	Dataset::parse_sql(query);
	if (db->setErr(sqlite3_exec(this->handle(),query.c_str(),NULL,NULL,&err),query.c_str())!=SQLITE_OK) {
	  throw DbErrors("%s", db->getErrorMsg());
	}
  } // end of for


  if (db->in_transaction() && autocommit) db->commit_transaction();

  active = true;
  ds_state = dsSelect;		
  if (autorefresh)
    refresh();

 } // end of try
 catch(...) {
  if (db->in_transaction()) db->rollback_transaction();
  throw;
 }

}


void SqliteDataset::make_insert() {
  make_query(insert_sql);
  last();
}


void SqliteDataset::make_edit() {
  make_query(update_sql);
}


void SqliteDataset::make_deletion() {
  make_query(delete_sql);
}


void SqliteDataset::fill_fields() {
  //cout <<"rr "<<result.records.size()<<"|" << frecno <<"\n";
  if ((db == NULL) || (result.record_header.size() == 0) || (result.records.size() < (unsigned int)frecno)) return;

  if (fields_object->size() == 0) // Filling columns name
  {
    const unsigned int ncols = result.record_header.size();
    fields_object->resize(ncols);
    for (unsigned int i = 0; i < ncols; i++)
      (*fields_object)[i].props = result.record_header[i];
    }

  //Filling result
  if (result.records.size() != 0)
  {
    const sql_record *row = result.records[frecno];
    if (row)
    {
      const unsigned int ncols = row->size();
      fields_object->resize(ncols);
      for (unsigned int i = 0; i < ncols; i++)
        (*fields_object)[i].val = row->at(i);
      return;
    }
  }
  const unsigned int ncols = result.record_header.size();
  fields_object->resize(ncols);
  for (unsigned int i = 0; i < ncols; i++)
    (*fields_object)[i].val = "";
  }    


//------------- public functions implementation -----------------//

int SqliteDataset::exec(const string &sql) {
  if (!handle()) throw DbErrors("No Database Connection");
  int res;
  exec_res.clear();
  
  ::EnterCriticalSection(g_lock);
  try
  {
    res = db->setErr(sqlite3_exec(handle(),sql.c_str(),&callback,&exec_res,&errmsg),sql.c_str());
  }
  catch(...)
  {
    res = SQLITE_ERROR;
  }
  ::LeaveCriticalSection(g_lock);
   if(res == SQLITE_OK)
    return res;
    
      throw DbErrors("%s", db->getErrorMsg());
    }

int SqliteDataset::exec() {
	return exec(sql);
}

const void* SqliteDataset::getExecRes() {
  return &exec_res;
}


bool SqliteDataset::query(const char *query) {
    if(!handle()) throw DbErrors("No Database Connection");
    std::string qry = query;
    int fs = qry.find("select");
    int fS = qry.find("SELECT");
    if (!( fs >= 0 || fS >=0))                                 
         throw DbErrors("MUST be select SQL!"); 

  close();

  sqlite3_stmt *stmt = NULL;
  #ifdef __APPLE__
  if (db->setErr(sqlite3_prepare(handle(),query,-1,&stmt, NULL),query) != SQLITE_OK)
  #else
  if (db->setErr(sqlite3_prepare_v2(handle(),query,-1,&stmt, NULL),query) != SQLITE_OK)
  #endif
    throw DbErrors("%s", db->getErrorMsg());

  // column headers
  const unsigned int numColumns = sqlite3_column_count(stmt);
  result.record_header.resize(numColumns);
  for (unsigned int i = 0; i < numColumns; i++)
    result.record_header[i].name = sqlite3_column_name(stmt, i);

  // returned rows
  while (sqlite3_step(stmt) == SQLITE_ROW)
  { // have a row of data
    sql_record *res = new sql_record;
    res->resize(numColumns);
    for (unsigned int i = 0; i < numColumns; i++)
    {
      field_value &v = res->at(i);
      switch (sqlite3_column_type(stmt, i))
      {
      case SQLITE_INTEGER:
        v.set_asInt64(sqlite3_column_int64(stmt, i));
        break;
      case SQLITE_FLOAT:
        v.set_asDouble(sqlite3_column_double(stmt, i));
        break;
      case SQLITE_TEXT:
        v.set_asString((const char *)sqlite3_column_text(stmt, i));
        break;
      case SQLITE_BLOB:
        v.set_asString((const char *)sqlite3_column_text(stmt, i));
        break;
      case SQLITE_NULL:
      default:
        v.set_asString("");
        v.set_isNull();
        break;
      }
    }
    result.records.push_back(res);
  }
  if (db->setErr(sqlite3_finalize(stmt),query) == SQLITE_OK)
  {
    active = true;
    ds_state = dsSelect;
    this->first();
    return true;
  }
  else
  {
    throw DbErrors("%s", db->getErrorMsg());
  }  
}

bool SqliteDataset::query(const string &q){
  return query(q.c_str());
}

void SqliteDataset::open(const string &sql) {
	set_select_sql(sql);
	open();
}

void SqliteDataset::open() {
  if (select_sql.size()) {
    query(select_sql.c_str()); 
  }
  else {
    ds_state = dsInactive;
  }
}


void SqliteDataset::close() {
  Dataset::close();
  result.clear();
  edit_object->clear();
  fields_object->clear();
  ds_state = dsInactive;
  active = false;
}


void SqliteDataset::cancel() {
  if ((ds_state == dsInsert) || (ds_state==dsEdit)) {
    if (result.record_header.size())
      ds_state = dsSelect;
    else
      ds_state = dsInactive;
  }
}


int SqliteDataset::num_rows() {
  return result.records.size();
}


bool SqliteDataset::eof() {
  return feof;
}


bool SqliteDataset::bof() {
  return fbof;
}


void SqliteDataset::first() {
  Dataset::first();
  this->fill_fields();
}

void SqliteDataset::last() {
  Dataset::last();
  fill_fields();
}

void SqliteDataset::prev(void) {
  Dataset::prev();
  fill_fields();
}

void SqliteDataset::next(void) {
#ifdef _XBOX
  free_row();
#endif
  Dataset::next();
  if (!eof()) 
      fill_fields();
}

void SqliteDataset::free_row(void)
{
  if (frecno < 0 || (unsigned int)frecno >= result.records.size())
    return;

  sql_record *row = result.records[frecno];
  if (row)
  {
    delete row;
    result.records[frecno] = NULL;
  }
}

bool SqliteDataset::seek(int pos) {
  if (ds_state == dsSelect) {
    Dataset::seek(pos);
    fill_fields();
    return true;	
    }
  return false;
}



long SqliteDataset::nextid(const char *seq_name) {
  if (handle()) return db->nextid(seq_name);
  else return DB_UNEXPECTED_RESULT;
}

void SqliteDataset::interrupt() {
  sqlite3_interrupt(handle());
}
}//namespace
