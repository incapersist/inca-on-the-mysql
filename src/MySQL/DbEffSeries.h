/**
 * The <code>DbEffSeries</code> fetches model driving data from a MySQL database, via a supplied session.
 *
 * @param[in]	session is the MySQL connection
 */
#pragma once
#include <string>
#include <mysqlx/xdevapi.h>
#include "DbModelIndexers.h"
#include "DbModelRun.h"
#include "CEffSeries.h"

class DbEffSeries
{
public:
	/// Store the supplied session pointer as a member variable
	DbEffSeries(std::shared_ptr<mysqlx::Session>& session, const std::string& tableName);
	~DbEffSeries();

	/**
	 * Fetches driving data, from the input file identified by inputFileId,
	 * from the MySQL database identified by the session_ member.
	 *
	 * @param[in]	modelIndexers contains indexers for the model run
	 * @param[in]	dbModelRun is the model run meta data
	 * @return		A model object containing data retrieved from the database
	 * @throw		A simple, generic exception if anything goes wrong with the retrieval
	 */
	std::vector<CEffSeries*> fetch(DbModelIndexers& modelIndexers, DbModelRun& dbModelRun);

private:
	std::shared_ptr<mysqlx::Session> session_;	/// A local copy of a pointer to the active database session
	std::string tableName_;		/// The name of the MySQL table from which to fetch data

	/**
	 * Retrieve from the database the number of reaches used by the file data, as
	 * identified by fileId
	 *
	 * @param[in]	fileId is the database ID of the file, passed from the fetch function
	 * @throw		A simple, generic exception if anything goes wrong with the retrieval
	 */
	std::vector<DbModelIndex> getFileReachIndexes(unsigned int fileId);

	/**
	 * Retrieve from the database the data for a single reach (identified by
	 * reachId), for the input file identified by inputFileId
	 *
	 * @param[in]	reachId is the database id of the reach for which to fetch data
	 * @param[in]	inputFileId is the database ID of the file, passed from the fetch function
	 * @param[in]	dbModelRun is the model run meta data
	 * @throw		A simple, generic exception if anything goes wrong with the retrieval
	 */
	CBaseSeries* fetchData(unsigned int reachId, unsigned int inputFileId, DbModelRun& dbModelRun);

	/**
	 * Retrieve from the database the driving data for the specified reach, and add the data to the
	 * supplied input container object
	 *
	 * @param[in/out]	eff is the inputs data container object
	 * @param[in]		reach is the ID of the reach for which to retrieve data
	 * @param[in]		inputFileId identifies the correct input file to grab data for
	 * @param[in]		dbModelRun is the model run meta data
	 */
	void fetchReachData(CEffSeries& eff, DbModelIndex& reach, unsigned int inputFileId, DbModelRun& dbModelRun);
};

