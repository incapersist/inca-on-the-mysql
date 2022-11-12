#include "DbEffSeries.h"
#include <iostream>
#include <sstream>
#include "DbException.h"

using namespace mysqlx;

DbEffSeries::DbEffSeries(std::shared_ptr<mysqlx::Session>& session, const std::string& tableName) : session_(session), tableName_(tableName)
{
}

DbEffSeries::~DbEffSeries()
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
std::vector<CEffSeries*> DbEffSeries::fetch(DbModelIndexers& modelIndexers, DbModelRun& dbModelRun)
{
	const unsigned int fileId = dbModelRun.getEffluentFileId();

	/// Check how many reaches are represented in the driving data for this model run
	std::vector<DbModelIndex> indexes;

	std::vector<CEffSeries*> effSeriesList;

	try
	{
		indexes = getFileReachIndexes(fileId);

		/// For every reach in the system...
		for (auto reach : indexes)
		{
			/// Set up a model container object to store the retrieved data.
			/// A copy of this object will be returned by if this functions if the retrieval is successful.
			CEffSeries* eff = new CEffSeries(dbModelRun.getTimeSteps(), 4);
			fetchReachData(*eff, reach, fileId, dbModelRun);
			effSeriesList.push_back(eff);
		}

		return effSeriesList;
	}
	catch(DbException& err)
	{
		throw err;
	}
}

/**
 * Retrieve from the database the effluent data for the specified reach, and add the data to the
 * supplied  container object
 *
 * @param[in/out]	eff is the effluent data container object
 * @param[in]		reach identifies the reach to get data for
 * @param[in]		inputFileId identifies the correct input file to grab data for
 * @param[in]		dbModelRun is the model run meta data
 */
void DbEffSeries::fetchReachData(CEffSeries& eff, DbModelIndex& reach, unsigned int inputFileId, DbModelRun& dbModelRun)
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
	eff.Effluent[reach.getName()] = a;
}

/**
 * Retrieve from the database the number of reaches used by the effluent data, as
 * identified by inputFileId
 *
 * @param[in]	inputFileId is the database ID of the file, passed from the fetch function
 * @throw		A simple, generic exception if anything goes wrong with the retrieval
 */
std::vector<DbModelIndex> DbEffSeries::getFileReachIndexes(unsigned int fileId)
{
	/// MySQL SELECT statement to get reach indexes described in the effluent data
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
		throw DbException(err, ExitCode::effReachIndexSqlError);
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
		throw DbException(Error("No effluent data reach index found"), ExitCode::noEffReachIndex);
	}
}

/**
 * Retrieve from the database the driving data for the specified reach, and add the data to the
 * supplied input container object
 *
 * @param[in/out]	eff is the inputs data container object
 * @param[in]		reach is the ID of the reach for which to retrieve data
 * @param[in]		inputFileId identifies the correct input file to grab data for
 * @param[in]		dbModelRun is the model run meta data
 */
CBaseSeries* DbEffSeries::fetchData(unsigned int reachId, unsigned int fileId, DbModelRun& dbModelRun)
{
	/// MySQL query to get drivng data as for reachId, modelRun
	std::stringstream sqlStmt;
	sqlStmt << "SELECT timestep, flow, nitrate, ammonium, organic_nitrogen FROM " << tableName_ << " WHERE reach_id = " << reachId << " AND input_file_id = " << fileId << " LIMIT " << dbModelRun.getOffset() << ", " << dbModelRun.getTimeSteps();

	SqlStatement sql = session_->sql(sqlStmt.str().c_str());

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.execute();
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::effDataSqlError);
	}

	/// If the query produced data...
	if (result.hasData())
	{
		/// Check that we have enough data
		if (result.count() < dbModelRun.getTimeSteps())
		{
			throw DbException(Error("Not enough effluent data available"), ExitCode::tooFewTimesteps);
		}

		/// Create a new model input object to store the retrieved data
		CBaseSeries *eff = new CBaseSeries(1, result.count());

		/// Get all the query data
		auto rows = result.fetchAll();

		/// Fill the object with query data
		for (auto row : rows)
		{
			const unsigned int index = (int)row[0] - 1;
			eff->Data[0][index] = (double)row[1];
			eff->Data[1][index] = (double)row[2];
			eff->Data[2][index] = (double)row[3];
			eff->Data[3][index] = (double)row[4];
		}

		return eff;
	}
	/// Otherwise throw an exception
	else
	{
		throw DbException(Error("No input data for reach"), ExitCode::noEffData);
	}
}