#include "DbModelIndexers.h"

#include <vector>
#include <iostream>

#include "DbException.h"

using namespace mysqlx;

DbModelIndexers::DbModelIndexers()
{
}

DbModelIndexers::~DbModelIndexers()
{
}

/**
 * Return the indexes for the provided indexer name. For exmaple, if the string 'reach'
 * is passed, then the function will return all of the reach indexes
 *
 * @param[in]	indexerName is the 'key' indexer name
 * @return		vector of indexes for indexer 'needle'
 */
std::vector<DbModelIndex>&  DbModelIndexers::operator[] (const std::string& indexerName)
{
	/// TODO:	bounds checking, exceptions
	return indexers_.find(indexerName)->second;
}

/**
 * Fetch (and store internally) all model indexers from the MySQL database for a particular model run
 *
 * @param[in]	session is the active database connection
 * @param[in]	modelRun identifies the correct model run to use
 */
void DbModelIndexers::fetch(std::shared_ptr<mysqlx::Session> session, unsigned int modelRunId)
{
	/// MySQL query to fetch all the indexers for a model run
	const char * stmt = R"(	SELECT 
								indexer.id,
								indexer.name
							FROM 
								model_run
								JOIN parameter_set ON model_run.parameter_set_id = parameter_set.id
								JOIN indexer_model_version_core_xref ON indexer_model_version_core_xref.model_version_core_id = parameter_set.model_version_core_id
								JOIN indexer ON indexer.id = indexer_model_version_core_xref.indexer_id
							WHERE
								model_run.id = ?)";

	/// Try to execute the query...
	SqlResult result;
	try
	{
		SqlStatement sql = session->sql(stmt);
		result = sql.bind(modelRunId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::indexerSqlError);
	}

	/// If the query completed and returned data...
	if (result.hasData())
	{
		/// Get all the indexers 
		auto rows = result.fetchAll();

		/// Fetch all the indexes for each indexer and add to the map
		for (auto row : rows)
		{
			try
			{
				indexers_[(std::string)row[1]] = getIndexes(session, modelRunId, int(row[0]));
			}
			catch (DbException& err)
			{
				throw err;
			}
		}
	}
	/// otherwise throw an exception
	else
	{
		throw DbException(Error("No model indexer data available"), ExitCode::noModelIndexers);
	}
}

/**
 * Retrieves all the indexes from the database for a particular indexer for a distinct model run
 *
 * @param[in]	session is the active database connection
 * @param[in]	modelRunId is the model run id for which to retrieve indexes
 * @param[in]	indexerId id the DB ID of the indexer for which to retrieve indexes
 * @return		a vector of all the indexes for the indexer
 */
std::vector<DbModelIndex> DbModelIndexers::getIndexes(std::shared_ptr<mysqlx::Session>& session, unsigned int modelRunId, unsigned int indexerId)
{
	/// MySQL query to fetch all of the indexes for an indexer
	const char * stmt = R"(	SELECT 
								indexer_index.id,
								indexer_index.reference
							FROM 
								indexer_index
                                JOIN parameter_set_index_xref psix ON psix.indexer_index_id = indexer_index.id
								JOIN model_run ON model_run.parameter_set_id = psix.parameter_set_id
							WHERE 
								model_run.id = ?
								AND indexer_index.indexer_id = ?)";

	/// Try to execute the query...
	SqlResult result;
	try
	{
		SqlStatement sql = session->sql(stmt);
		result = sql.bind(modelRunId).bind(indexerId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::indexSqlError);
	}

	/// If the query completed and returned data...
	if (result.hasData())
	{
		/// Get all the indexes and set up a new container
		auto rows = result.fetchAll();
		std::vector<DbModelIndex> indexes;

		/// Add all of the retrieved indexes to the container
		for (auto row : rows)
		{
			indexes.push_back(DbModelIndex(int(row[0]), std::string(row[1])));
		}

		/// Return the index container
		return indexes;
	}
	/// otherwise throw an exception
	else
	{
		throw DbException(Error("No indexes available for indexer"), ExitCode::noIndexes);
	}
}