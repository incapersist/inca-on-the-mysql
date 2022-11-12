#include "DbFertSeries.h"
#include <iostream>
#include <sstream>
#include "DbException.h"

using namespace mysqlx;

DbFertSeries::DbFertSeries(std::shared_ptr<mysqlx::Session>& session, const std::string& tableName) : session_(session), tableName_(tableName)
{
}

DbFertSeries::~DbFertSeries()
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
CFertSeries* DbFertSeries::fetch(DbModelIndexers& modelIndexers, DbModelRun& dbModelRun, unsigned int fileId)
{
	/// Check how many reaches are represented in the driving data for this model run
	std::vector<DbModelIndex> indexes;

	try
	{
		indexes = getFileReachIndexes(fileId);
		unsigned int landIndexOffset = getLandIndexOffset(dbModelRun.getParameterSetId());
		unsigned int landCount = (unsigned int)modelIndexers["Landscape units"].size();

		/// Set up a model container object to store the retrieved data.
		/// A copy of this object will be returned by if this functions if the retrieval is successful.
		CFertSeries* fert = new CFertSeries(dbModelRun.getTimeSteps(), landCount);

		/// For every reach in the system...
		for (auto reach : indexes)
		{
			fetchReachData(*fert, reach, fileId, dbModelRun, landCount, landIndexOffset);
		}

		return fert;
	}
	catch(DbException& err)
	{
		throw err;
	}
}

/**
 * Retrieve from the database the fertilizer data for the specified reach, and add the data to the
 * supplied container object
 *
 * @param[in/out]	fert is the fertilizer data container object
 * @param[in]		reach identifies the reach to get data for
 * @param[in]		inputFileId identifies the correct input file to grab data for
 * @param[in]		dbModelRun is the model run meta data
 */
void DbFertSeries::fetchReachData(CFertSeries& fert, DbModelIndex& reach, unsigned int inputFileId, DbModelRun& dbModelRun, unsigned int landCount, unsigned int landIndexOffset)
{
	/// Create a new input data object to store driving data for a single reach
	CBaseSeries* a;

	/// Try to fetch driving data for a single reach
	try
	{
		a = fetchData(reach.getId(), inputFileId, dbModelRun, landCount, landIndexOffset);
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::data);
	}

	/// Add the new CBaseSeries object to the container
	fert.Fert[reach.getName()] = a;
}

/**
 * Retrieve from the database the number of reaches used by the fertilizer data, as
 * identified by inputFileId
 *
 * @param[in]	inputFileId is the database ID of the file, passed from the fetch function
 * @throw		A simple, generic exception if anything goes wrong with the retrieval
 */
std::vector<DbModelIndex> DbFertSeries::getFileReachIndexes(unsigned int fileId)
{
	/// MySQL SELECT statement to get reach indexes described in the fertilizer data
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
		throw DbException(err, ExitCode::fertReachIndexSqlError);
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
		throw DbException(Error("No fertilizer data reach index found"), ExitCode::noFertReachIndex);
	}
}

unsigned int DbFertSeries::getLandIndexOffset(unsigned int parameterSetId)
{
	/// MySQL SELECT statement to get reach indexes described in the fertilizer data
	const char * stmt = R"(	SELECT
								MIN(psix.id)
							FROM
								parameter_set_index_xref psix
								JOIN indexer_index ON psix.indexer_index_id = indexer_index.id
							WHERE
								indexer_index.indexer_id = 2
								AND psix.parameter_set_id = ?)";

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(stmt);

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(parameterSetId).execute();
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::fertReachIndexSqlError);
	}
	unsigned int count = 0;

	/// If the query produced data...
	if (result.hasData())
	{
		/// Get all the query data
		auto rows = result.fetchAll();

		/// Fill the object with query data
		for (auto row : rows)
		{
			count = (unsigned int)row[0];
		}
	}
	/// Otherwise throw an exception
	else
	{
		throw DbException(Error("No fertilizer data reach index found"), ExitCode::noFertReachIndex);
	}

	return count;
}

/**
 * Retrieve from the database the driving data for the specified reach, and add the data to the
 * supplied input container object
 *
 * @param[in/out]	fert is the inputs data container object
 * @param[in]		reach is the ID of the reach for which to retrieve data
 * @param[in]		inputFileId identifies the correct input file to grab data for
 * @param[in]		dbModelRun is the model run meta data
 */
CBaseSeries* DbFertSeries::fetchData(unsigned int reachId, unsigned int fileId, DbModelRun& dbModelRun, unsigned int landCount, unsigned int landIndexOffset)
{
	/// MySQL query to get drivng data as for reachId, modelRun
	std::stringstream sqlStmt;
	sqlStmt << "SELECT timestep, land_id, value";
	sqlStmt << " FROM " << tableName_;
	sqlStmt << " WHERE reach_id = " << reachId;
	sqlStmt << " AND input_file_id = " << fileId;
	sqlStmt << " AND timestep > " << dbModelRun.getOffset() << " and timestep <= " << dbModelRun.getTimeSteps();

	SqlStatement sql = session_->sql(sqlStmt.str().c_str());

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.execute();
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::fertDataSqlError);
	}

	/// If the query produced data...
	if (result.hasData())
	{
		/// Check that we have enough data
		if (result.count() < dbModelRun.getTimeSteps())
		{
			throw DbException(Error("Not enough fertilizer data available"), ExitCode::tooFewTimesteps);
		}

		/// Create a new model input object to store the retrieved data
		CBaseSeries *fert = new CBaseSeries(landCount, dbModelRun.getTimeSteps());

		/// Get all the query data
		auto rows = result.fetchAll();

		/// Fill the object with query data
		for (auto row : rows)
		{
			const unsigned int timestepIndex = (int)row[0] - 1;
			const unsigned int landIndex = (int)row[1] - landIndexOffset;
			fert->Data[landIndex][timestepIndex] = (double)row[2];
		}

		return fert;
	}
	/// Otherwise throw an exception
	else
	{
		throw DbException(Error("No input data for reach"), ExitCode::noFertData);
	}
}