#include "DbDepSeries.h"
#include <iostream>
#include <sstream>
#include "DbException.h"

using namespace mysqlx;

DbDepSeries::DbDepSeries(std::shared_ptr<mysqlx::Session>& session, const std::string& tableName) : session_(session), tableName_(tableName)
{
}

DbDepSeries::~DbDepSeries()
{
}

/**
 * Fetches abtraction data, from the input file identified by fileId,
 * from the MySQL database identified by the session_ member.
 *
 * @param[in]	modelIndexers contains indexers for the model run
 * @param[in]	dbModelRun is the model run meta data
 * @return		An input data object containing data retrieved from the database
 * @throw		A simple, generic exception if anything goes wrong with the retrieval
 */
CDepSeries* DbDepSeries::fetch(DbModelIndexers& modelIndexers, DbModelRun& dbModelRun)
{
	const unsigned int fileId = dbModelRun.getDepositionFileId();

	/// Check how many reaches are represented in the driving data for this model run
	std::vector<DbModelIndex> indexes;


	try
	{
		indexes = getFileReachIndexes(fileId);

		CDepSeries* dep = new CDepSeries(dbModelRun.getTimeSteps(), 4);

		/// For every reach in the system...
		for (auto reach : indexes)
		{
			/// Set up a model container object to store the retrieved data.
			/// A copy of this object will be returned by if this functions if the retrieval is successful.
			fetchReachData(*dep, reach, fileId, dbModelRun);
		}

		return dep;
	}
	catch(DbException& err)
	{
		throw err;
	}
}

/**
 * Retrieve from the database the deposition data for the specified reach, and add the data to the
 * supplied  container object
 *
 * @param[in/out]	dep is the deposition data container object
 * @param[in]		reach identifies the reach to get data for
 * @param[in]		inputFileId identifies the correct input file to grab data for
 * @param[in]		dbModelRun is the model run meta data
 */
void DbDepSeries::fetchReachData(CDepSeries& dep, DbModelIndex& reach, unsigned int inputFileId, DbModelRun& dbModelRun)
{
	/// Create a new input data object to store driving data for a single reach
	CBaseSeries* a;

	/// Try to fetch driving data for a single reach
	try
	{
		a = fetchData(reach.getId(), inputFileId, dbModelRun);
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::data);
	}

	/// Add the new CBaseSeries object to the container
	dep.Deposition[reach.getName()] = a;
}

/**
 * Retrieve from the database the number of reaches used by the deposition data, as
 * identified by inputFileId
 *
 * @param[in]	inputFileId is the database ID of the file, passed from the fetch function
 * @throw		A simple, generic exception if anything goes wrong with the retrieval
 */
std::vector<DbModelIndex> DbDepSeries::getFileReachIndexes(unsigned int fileId)
{
	/// MySQL SELECT statement to get reach indexes described in the deposition data
	std::stringstream sqlStmt;
	sqlStmt << "SELECT id, reference FROM indexer_index WHERE id IN (SELECT DISTINCT reach_id FROM " << tableName_ << " WHERE input_file_id = ";
	sqlStmt << fileId << ")";

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(sqlStmt.str().c_str());

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.execute();
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::depReachIndexSqlError);
	}

	/// If the query produced data...
	if (result.hasData())
	{
		/// Get all the query data
		auto rows = result.fetchAll();

		std::vector<DbModelIndex> indexes;

		/// Fill the object with query data
		for (auto row : rows)
		{
			indexes.push_back(DbModelIndex((int)row[0], std::string(row[1])));
		}

		return indexes;
	}
	/// Otherwise throw an exception
	else
	{
		throw DbException(Error("No deposition data reach index found"), ExitCode::noDepReachIndex);
	}
}

/**
 * Retrieve from the database the driving data for the specified reach, and add the data to the
 * supplied input container object
 *
 * @param[in/out]	dep is the inputs data container object
 * @param[in]		reach is the ID of the reach for which to retrieve data
 * @param[in]		inputFileId identifies the correct input file to grab data for
 * @param[in]		dbModelRun is the model run meta data
 */
CBaseSeries* DbDepSeries::fetchData(unsigned int reachId, unsigned int fileId, DbModelRun& dbModelRun)
{
	/// MySQL query to get drivng data as for reachId, modelRun
	std::stringstream sqlStmt;
	sqlStmt << "SELECT timestep, nitrate_dry, nitrate_wet, ammonium_dry, ammonium_wet FROM " << tableName_ << " WHERE reach_id = " << reachId << " AND input_file_id = " << fileId << " LIMIT " << dbModelRun.getOffset() << ", " << dbModelRun.getTimeSteps();

	SqlStatement sql = session_->sql(sqlStmt.str().c_str());

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.execute();
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::depDataSqlError);
	}

	/// If the query produced data...
	if (result.hasData())
	{
		/// Check that we have enough data
		if (result.count() < dbModelRun.getTimeSteps())
		{
			throw DbException(Error("Not enough deposition data available"), ExitCode::tooFewTimesteps);
		}

		/// Create a new model input object to store the retrieved data
		CBaseSeries *dep = new CBaseSeries(4, result.count());

		/// Get all the query data
		auto rows = result.fetchAll();

		/// Fill the object with query data
		for (auto row : rows)
		{
			const unsigned int index = (int)row[0] - 1;
			dep->Data[0][index] = (double)row[1];
			dep->Data[1][index] = (double)row[2];
			dep->Data[2][index] = (double)row[3];
			dep->Data[3][index] = (double)row[4];
		}

		return dep;
	}
	/// Otherwise throw an exception
	else
	{
		throw DbException(Error("No input data for reach"), ExitCode::noDepData);
	}
}