#include "DbStructure.h"
#include <iostream>
#include "DbException.h"

using namespace mysqlx;

DbStructure::DbStructure(std::shared_ptr<mysqlx::Session> session) : session_(session)
{
}

DbStructure::~DbStructure()
{
}

/**
 * Retrieve stream/reach data from the MySQL database and build a PERSiST
 * structure object, which is stored as a member variable
 *
 * @param[in]	modelRun defines the model run to use to grab the correct data
 * @param[in]	indexers describes the model run indexers/indexes
 * @return		a new PERSiST CStructure object
 */
CStructure DbStructure::fetch(DbModelRun& modelRun, DbModelIndexers &indexers)
{
	const unsigned int parameterSetId = modelRun.getParameterSetId();

	/// Set whether the PERSiST reach structure has been generated (i.e. no user-supplied
	/// structure file has been stored in the database) or is user-defined (a structure 'file'
	/// is present in the database).
	structure_.Generated(modelRun.isGeneratedStructure());

	/// Get a list of all the stream indexes from the indexers object
	auto streams = indexers["Stream"];

	/// Try to get all the streams for this model run
	std::vector<DbStream> dbStreams;
	try
	{
		dbStreams = fetchStreams(modelRun);
	}
	catch (DbException& err)
	{
		throw err;
	}

	/// For each stream that was fetched...
	for (auto dbStream : dbStreams)
	{
		/// Create a new container of PERSiST reach objects
		std::vector<CReach> dbReaches;

		/// Try to get all the reaches for the stream
		try
		{
			dbReaches = fetchReaches(dbStream.getId(), parameterSetId, modelRun.getStreamNetworkId());
		}
		catch (DbException& err)
		{
			throw err;
		}

		/// Create a PERSiST container object for this stream, to store the fetched reaches
		CStream stream(dbStream.getName(), dbStream.getOrder(), dbStream.getName());

		/// Add all the fetched reaches to this stream
		for (auto reach : dbReaches)
		{
			stream.AddReach(reach);
		}

		/// Add this new stream to the PERSiST structure object
		structure_.AddStream(stream);
	}

	/// Return a copy of the PERSiST strcuture object
	return structure_;
}

/**
 * Retrieves stream data from the persist.stream table, to build the PERSiST strucutre object
 *
 * @param[in]	modelRun tells the function which data to grab
 * @return		a vector of streams, being all the streams in the model run
 *
 * TODO			Connect streams together
 */
std::vector<DbStream> DbStructure::fetchStreams(DbModelRun& modelRun)
{
	/// MySQL query to get all the streams for a given model run
	const char * stmt = R"(	SELECT 
								DISTINCT(stream.id),
								stream.strahler_order,
								indexer_index.reference
							FROM 
								stream
                                LEFT JOIN reach_structure rs ON rs.parent_stream_id = stream.id 
                                LEFT JOIN parameter_set ON rs.parameter_set_id = parameter_set.id
                                LEFT JOIN model_run on parameter_set.id = model_run.parameter_set_id
                                LEFT JOIN indexer_index ON stream.index_id = indexer_index.id
							WHERE
								model_run.id = ?
                                AND rs.stream_network_id = COALESCE(
									model_run.stream_network_id,
                                    (SELECT id FROM stream_network WHERE parameter_set_id = rs.parameter_set_id AND is_default = 1)
								)
							)";
	
	/// Build a query object from the query string
	SqlStatement sql = session_->sql(stmt);
	SqlResult result;

	/// Try to execute the query
	try
	{
		result = sql.bind(modelRun.getId()).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::structureSqlError);
	}

	/// If valid results were returned...
	if (result.hasData())
	{
		/// Create storage for the fetched streams
		std::vector<DbStream> streams;

		/// Get all of the results from the query
		auto rows = result.fetchAll();

		/// For each row in the results set...
		for (auto row : rows)
		{
			/// Create a new stream from the data in the row
			DbStream stream((unsigned int)row[0], (unsigned int)row[1], (std::string)row[2]);

			/// Add the new stream to the stream container
			streams.push_back(stream);
		}

		/// Return a copy of the stream container
		return streams;
	}
	else
	{
		throw DbException(Error("No reach structure available"), ExitCode::noReachStructure);
	}
}

/**
 * Retrieves all the reaches for a stream, from persist.reach_structure
 *
 * @param[in]	streamId is the database ID of the stream for which to fetch reaches
 * @param[in]	parameterSetId is the database ID of the parameter set for which to fetch reaches
 * @param[in]	streamNetworkId is the database ID of the stream network for which to fetch reach inputs
 * @return		a vector of PERSiST CReach objects, being all the reaches for the stream
 */
std::vector<CReach> DbStructure::fetchReaches(unsigned int streamId, unsigned int parameterSetId, unsigned int streamNetworkId)
{
	/// MySQL query statement to fetch all the reaches for a given stream
	const char * stmt = R"(	SELECT 
								indexer_index.reference,
								reach_structure.reach_id
							FROM 
								reach_structure
								JOIN indexer_index ON indexer_index.id = reach_structure.reach_id
							WHERE
								reach_structure.parent_stream_id = ?
                                AND reach_structure.parameter_set_id = ?
							ORDER BY
								reach_structure.reach_id)";

	/// Build a query object from the query string
	SqlStatement sql = session_->sql(stmt);
	
	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(streamId).bind(parameterSetId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::reachSqlError);
	}

	/// If valid results were returned...
	if (result.hasData())
	{
		/// Create storage for the fetched reaches
		std::vector<CReach> reaches;

		/// Get all of the results from the query
		auto rows = result.fetchAll();

		/// For each row in the results set...
		for (auto row : rows)
		{
			/// Create a new PERSiST reach object. For PERSiST, id === name
			CReach reach((std::string)row[0], (std::string)row[0]);
			std::vector<std::string> inputs = fetchReachInputs((unsigned int)row[1], streamNetworkId);

			for (auto input : inputs)
			{
				reach.AddInput(input);
			}

			/// Add the new reach to the reach container
			reaches.push_back(reach);
		}

		/// Return a copy of the reach container
		return reaches;
	}
	else
	{
		throw DbException(Error("No reaches available for stream"), ExitCode::noReaches);
	}
}

/**
 * Retrieves all the inputs for a reach, from persist.reach_structure_inputs
 *
 * @param[in]	reachId is the database ID of the reach for which to fetch inputs
 * @param[in]	streamNetworkId is the database ID of the stream network for which to fetch reach inputs
 * @return		a vector of string, being the references for all the inputs to the reach
 */
std::vector<std::string> DbStructure::fetchReachInputs(unsigned int reachId, unsigned int streamNetworkId)
{
	/// MySQL query statement to fetch all the inputs for a given reach
	const char * stmt = R"(	SELECT 
								indexer_index.reference
							FROM 
								reach_structure_inputs rsi
								LEFT JOIN indexer_index ON rsi.input_reach_id = indexer_index.id
							WHERE
								rsi.reach_id = ?
                                AND rsi.stream_network_id = ?)";

	/// Build a query object from the query string
	SqlStatement sql = session_->sql(stmt);

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(reachId).bind(streamNetworkId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::reachSqlError);
	}

	/// If valid results were returned...
	if (result.hasData())
	{
		/// Create storage for the fetched reaches
		std::vector<std::string> reachInputs;

		/// Get all of the results from the query
		auto rows = result.fetchAll();

		/// For each row in the results set...
		for (auto row : rows)
		{
			/// Add the new reach to the reach container
			reachInputs.push_back((std::string)row[0]);
		}

		/// Return a copy of the reach container
		return reachInputs;
	}
	else
	{
		throw DbException(Error("No reach inputs available for reach"), ExitCode::noReaches);
	}
}

/**
 * Splits a string into a vector of tokens
 *
 * @param[in]	str is the string to be split
 * @param[in]	delimiter is the character to split on
 * @return		a vector of strings, being all the tokens from the original string
 */
std::vector<std::string> DbStructure::split(std::string str, char delimiter)
{
	std::stringstream ss(str);
	std::string token;
	std::vector<std::string> tokens;

	while (std::getline(ss, token, delimiter))
	{
		tokens.push_back(token);
	}

	return tokens;
}