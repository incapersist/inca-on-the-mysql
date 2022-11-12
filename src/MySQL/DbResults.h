/**
 * The <code>DbResults</code> class converts a PERSiST result object to database storage.
 *
 * @param[in]	session is the active MySQL connection
 * @param[in]	modelRun is the current model run meta data
 */
#pragma once
#include <mysqlx/xdevapi.h>
#include "CResultsGroup.h"
#include "DbModelRun.h"
#include "DbModelIndexers.h"
#include "Logger.h"

class DbResults
{
public:
	DbResults(const std::string& schema, std::shared_ptr<mysqlx::Session> session, DbModelRun& modelRun);
	~DbResults();

	/**
	 * Accept a PERSiST result object and INSERT that data into the MySQL database
	 *
	 * @param[in]	indexers is the complete set of indexes used by the model run
	 * @param[in]	results is the PERSiST model results object
	 * @param[in]	output is the PERSiST CLI-compatible output level flag
	 * @param[in]	forceDelete says whether or not to delete results from tables
	 */
	void storeResults(DbModelIndexers& indexers, CResultsGroup& results, int output, bool forceDelete);

private:
	std::string schema_;	/// The MySQL schema containing model data
	std::shared_ptr<mysqlx::Session> session_;	/// A pointer to the active MySQL database connection
	DbModelRun modelRun_;	/// The current model run meta data
	Logger& logger_;		/// Logging object;

	/**
	 * Inserts reach results into the persist.result_set_value_reach table for this model run
	 *
	 * @param[in]	schema is the MySQL schema to use ('persist');
	 * @param[in]	indexers is the complete set of indexes used by the model run
	 * @param[in]	results is the PERSiST model results object
	 */
	void insertReachResults(mysqlx::Schema& schema, DbModelIndexers& indexers, CResultsGroup& results);

	/**
	 * Inserts land results into the persist.result_set_value_reach_land table for this model run
	 *
	 * @param[in]	schema is the MySQL schema to use ('persist');
	 * @param[in]	indexers is the complete set of indexes used by the model run
	 * @param[in]	results is the PERSiST model results object
	 * @param[in]	output is the PERSiST CLI-compatible output level flag
	 */
	void insertLandSoilResults(mysqlx::Schema& schema, DbModelIndexers& indexers, CResultsGroup& res, int output);

	void insertLandDirectResults(mysqlx::Schema& schema, DbModelIndexers& indexers, CResultsGroup& res, int output);
	void insertLandGroundwaterResults(mysqlx::Schema& schema, DbModelIndexers& indexers, CResultsGroup& res, int output);
	void insertLandHydrologyResults(mysqlx::Schema& schema, DbModelIndexers& indexers, CResultsGroup& res, int output);

	/**
	 * Delete all results for the specified model run
	 *
	 * @param[in]	modelRunId identifies the model run for which to delete results
	 */
	void clear(unsigned int modelRunId);

	/**
	 * CREATE a TEMPORARY table LIKE the supplied table
	 *
	 * @param[in]	fromTable: the name of the table from which to create a TEMPORARY table
	 * @param[out]	the name of the new TEMPORARY table
	 */
	std::string createTemporaryTable(std::string const& fromTable);

	/**
	 * DROP the named temporary table
	 *
	 * @param[in]	tableName is the name of the TEMPORARY table to be DROPped
	 */
	void dropTemporaryTable(std::string const& tableName);

	/**
	 * Copy results over from the TEMPORARY table
	 */
	void updateResultsTable(std::string const& targetTableName);
};

