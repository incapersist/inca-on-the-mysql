#include "DbResults.h"
#include <ctime>
#include <sstream>
#include "DbException.h"
#include "DbTransaction.h"

using namespace mysqlx;

/**
 * The <code>DbResults</code> class converts a PERSiST result object to database storage.
 *
 * @param[in]	session is the active MySQL connection
 * @param[in]	modelRun is the current model run meta data
 * @param[in]	isDebug says whether to write out progress to the console
 */
DbResults::DbResults(const std::string& schema,
						std::shared_ptr<Session> session,
						DbModelRun& modelRun) : schema_(schema),
												session_(session), 
												modelRun_(modelRun), 
												logger_(Logger::GetInstance())
{
}

DbResults::~DbResults()
{
}

/**
 * Accept a PERSiST result object and INSERT that data into the MySQL database
 *
 * @param[in]	indexers is the complete set of indexes used by the model run
 * @param[in]	results is the PERSiST model results object
 * @param[in]	output is the PERSiST CLI-compatible output level flag
 * @param[in]	forceDelete says whether or not to delete results from tables
*/
void DbResults::storeResults(DbModelIndexers& indexers, CResultsGroup& results, int output, bool forceDelete)
{
	try
	{
		if (forceDelete)
		{
			clear(modelRun_.getId());
		}

		/// Get the schema which will be used by the results INSERT query building functions
		/// It's like this because there's no default constructor for mysqlx::Schema, so it can't be a member variable
		auto schema = session_->getSchema(schema_);

		logger_.log("Begin INSERTing results into database");

		/// Start a timer, to time the overall INSERT process
		double duration;
		std::clock_t timer = std::clock();

		/// Try to INSERT reach results. These are always INSERTed, regardless of output level
		insertReachResults(schema, indexers, results);
		/// Copy reach results from the TEMPORARY table
	//	updateResultsTable("result_set_value_reach");


		if (output > 0)
		{
			insertLandHydrologyResults(schema, indexers, results, output);
//			updateResultsTable("result_set_value_reach_land_hydrology");
		}

		if (output > 1)
		{
			insertLandDirectResults(schema, indexers, results, output);
//			updateResultsTable("result_set_value_reach_land_direct");

			insertLandSoilResults(schema, indexers, results, output);
//			updateResultsTable("result_set_value_reach_land_soil");

			insertLandGroundwaterResults(schema, indexers, results, output);
//			updateResultsTable("result_set_value_reach_land_groundwater");
		}

		/// If we're in debug mode, write timing info to the console
		duration = (std::clock() - timer) / (double)CLOCKS_PER_SEC;
		std::stringstream str;
		str << "TOTAL TIME: " << duration << " seconds" << std::endl;
		logger_.log(str.str());
	}
	catch (DbException& err)
	{
		throw err;
	}
}

/**
 * Inserts reach results into the result_set_value_reach table for this model run
 *
 * @param[in]	schema is the MySQL schema to use;
 * @param[in]	indexers is the complete set of indexes used by the model run
 * @param[in]	results is the PERSiST model results object
 */
void DbResults::insertReachResults(Schema& schema, DbModelIndexers& indexers, CResultsGroup& res)
{
//	std::string tempTable = createTemporaryTable("result_set_value_reach");

	/// Get a table object for the correct table from the schema. 
	/// It's like this because explicitly creating a TableInsert object appeared to be the
	/// best/most performant way to convert a PERSiST results object to database rows.
//	auto t = schema.getTable(tempTable.c_str());
	auto t = schema.getTable("result_set_value_reach");

	DbTransaction dbt(session_);

	/// Start a timer so we can check performance for this results group
	double duration;
	std::clock_t timer = std::clock();

	logger_.log("Inserting REACH results");

	/// For every reach in the system...
	for (auto reach : indexers["Reaches"])
	{
		/// Temporarily store the current reach name and ID.
		std::string reachKey = reach.getName();
		unsigned int reachId = reach.getId();

		std::stringstream str;
		str << "Reach " << reachKey;
		logger_.log(str.str());

		/// Create a new mysqlx::TableInsert object. This seemed the best way to INSERT PERSiST data
		TableInsert ti = t.insert("timestep","reach_id","model_run_id","flow", "nitrate", "ammonium", "organic_nitrogen","volume","depth","residence_time","width","area","perimeter","radius","velocity");

		/// Try to start a new MySQL transaction
		try
		{
			dbt.start();
		}
		catch (DbException& err)
		{
			throw err;
		}

		/// For each row of data in the PERSiST results set (for this reach)...
		for (unsigned int i = 0; i < res.SubCatchRes[reachKey]->Count; ++i)
		{
			/// Build a mysqlx::Row object from PERSiST results object data
			ti.values(Row(
				i+1, 
				reachId, 
				modelRun_.getId(), 
				res.SubCatchRes[reachKey]->Daily[0].Data[0][i],		// Flow
				res.SubCatchRes[reachKey]->Daily[0].Data[1][i],		// Nitrate
				res.SubCatchRes[reachKey]->Daily[0].Data[2][i],		// Ammonium
				res.SubCatchRes[reachKey]->Daily[0].Data[4][i],		// Organic N
				res.SubCatchRes[reachKey]->Daily[0].Data[3][i],		// Volume
				res.SubCatchRes[reachKey]->Daily[0].Data[7][i],		// Depth
				res.SubCatchRes[reachKey]->Daily[0].Data[11][i],	// Residence time
				res.SubCatchRes[reachKey]->Daily[0].Data[6][i],		// Width
				res.SubCatchRes[reachKey]->Daily[0].Data[8][i],		// Area
				res.SubCatchRes[reachKey]->Daily[0].Data[9][i],		// Perimeter
				res.SubCatchRes[reachKey]->Daily[0].Data[10][i],	// Radius
				res.SubCatchRes[reachKey]->Daily[0].Data[5][i]		// Velocity
				));
		}

		/// We should have built INSERTs for all of the data for this reach,
		/// so try to COMMIT it
		try
		{
			dbt.end(ti);
		}
		catch (DbException& err)
		{
			throw DbException("Failed to INSERT reach results", ExitCode::insertReachResults);
		}
	}

	/// If we're in debug mode, write timing info to the console.
	logger_.log("Finished inserting REACH results");

	duration = (std::clock() - timer) / (double)CLOCKS_PER_SEC;
	std::stringstream str;
	str << "INSERT COMPLETE IN: " << duration << " seconds" << std::endl;
	logger_.log(str.str());
}

/**
 * Inserts land results into the result_set_value_reach_land_soil table for this model run
 *
 * @param[in]	schema is the MySQL schema to use 
 * @param[in]	indexers is the complete set of indexes used by the model run
 * @param[in]	results is the PERSiST model results object
 * @param[in]	output is the PERSiST CLI-compatible output level flag
 */
void DbResults::insertLandSoilResults(Schema& schema, DbModelIndexers& indexers, CResultsGroup& res, int output)
{
//	std::string tempTable = createTemporaryTable("result_set_value_reach_land");

	/// Get a table object for the correct table from the schema. 
	/// It's like this because explicitly creating a TableInsert object appeared to be the
	/// best/most performant way to convert a PERSiST results object to database rows.
//	auto t = schema.getTable(tempTable.c_str());
	auto t = schema.getTable("result_set_value_reach_land_soil");

	/// Start a timer so we can check performance for this results group
	double duration;
	std::clock_t timer = std::clock();

	logger_.log("Inserting SOIL WATER results");

	unsigned int landCounter = 0;

	DbTransaction dbt(session_);

	/// For every landscape unit in the system...
	for (auto land : indexers["Landscape units"])
	{
		std::stringstream str;
		str << "Land " << land.getName();
		logger_.log(str.str());

		/// Temporarily store the current landscape unit ID.
		unsigned int landId = land.getId();

		/// For every reach in the system...
		for (auto reach : indexers["Reaches"])
		{
			/// Temporarily store the current reach name and ID.
			std::string reachKey = reach.getName();
			unsigned int reachId = reach.getId();

			std::stringstream str;
			str << "Reach " << reachKey;
			logger_.log(str.str());

			/// Create a new mysqlx::TableInsert object. This seemed the best way to INSERT PERSiST data
			TableInsert ti = t.insert("timestep","reach_id","land_id","model_run_id","flow", "nitrate", "ammonium", "organic_nitrogen", "solid_organic_nitrogen", "drainage_volume", "retention_volume", "total_volume", "infiltration", "water_depth", "smd", "soil_temperature");

			/// Try to start a new MySQL transaction
			try
			{
				dbt.start();
			}
			catch (DbException& err)
			{
				throw err;
			}

			/// For each row of data in the PERSiST results set (for this landscape unit and reach)...
			for (unsigned int i = 0; i < res.SubCatchRes[reachKey]->Count; ++i)
			{
				ti.values(Row(
					i+1,
					reachId, 
					landId, 
					modelRun_.getId(), 
					res.SubCatchRes[reachKey]->Soil[0].Data[landCounter][i],	// Flow
					res.SubCatchRes[reachKey]->Soil[1].Data[landCounter][i],	// Nitrate
					res.SubCatchRes[reachKey]->Soil[2].Data[landCounter][i],	// Ammonium
					res.SubCatchRes[reachKey]->Soil[4].Data[landCounter][i],	// Organic N
					res.SubCatchRes[reachKey]->Soil[5].Data[landCounter][i],	// Soil solid Organic N
					res.SubCatchRes[reachKey]->Soil[6].Data[landCounter][i],	// Drainage volume
					res.SubCatchRes[reachKey]->Soil[7].Data[landCounter][i],	// Retention volume
					res.SubCatchRes[reachKey]->Soil[3].Data[landCounter][i],	// Total volume
					res.SubCatchRes[reachKey]->Soil[9].Data[landCounter][i],	// Infiltration
					res.SubCatchRes[reachKey]->Soil[10].Data[landCounter][i],	// Water depth
					res.SubCatchRes[reachKey]->Soil[11].Data[landCounter][i],	// SMD
					res.SubCatchRes[reachKey]->Soil[8].Data[landCounter][i]		// Soil temperature
					));
			}

			/// Try to COMMIT
			/// Note that this will INSERT data for ALL time steps, ONE landscape unit and ONE reach
			try
			{
				dbt.end(ti);
			}
			catch (DbException& err)
			{
				throw DbException("Failed to INSERT soil water results", ExitCode::insertLandResults);
			}
		}

		++landCounter;
	}

	/// If we're in debug mode, write timing info to the console.
	logger_.log("Finished inserting SOIL WATER results");

	duration = (std::clock() - timer) / (double)CLOCKS_PER_SEC;
	std::stringstream str;
	str << "INSERT COMPLETE IN: " << duration << " seconds" << std::endl;
	logger_.log(str.str());
}

/**
 * Inserts land results into the result_set_value_reach_land_direct table for this model run
 *
 * @param[in]	schema is the MySQL schema to use
 * @param[in]	indexers is the complete set of indexes used by the model run
 * @param[in]	results is the PERSiST model results object
 * @param[in]	output is the PERSiST CLI-compatible output level flag
 */
void DbResults::insertLandDirectResults(Schema& schema, DbModelIndexers& indexers, CResultsGroup& res, int output)
{
	//	std::string tempTable = createTemporaryTable("result_set_value_reach_land");

		/// Get a table object for the correct table from the schema. 
		/// It's like this because explicitly creating a TableInsert object appeared to be the
		/// best/most performant way to convert a PERSiST results object to database rows.
	//	auto t = schema.getTable(tempTable.c_str());
	auto t = schema.getTable("result_set_value_reach_land_direct");

	/// Start a timer so we can check performance for this results group
	double duration;
	std::clock_t timer = std::clock();

	logger_.log("Inserting DIRECT RUNOFF results");

	unsigned int landCounter = 0;

	DbTransaction dbt(session_);

	/// For every landscape unit in the system...
	for (auto land : indexers["Landscape units"])
	{
		std::stringstream str;
		str << "Land " << land.getName();
		logger_.log(str.str());

		/// Temporarily store the current landscape unit ID.
		unsigned int landId = land.getId();

		/// For every reach in the system...
		for (auto reach : indexers["Reaches"])
		{
			/// Temporarily store the current reach name and ID.
			std::string reachKey = reach.getName();
			unsigned int reachId = reach.getId();

			std::stringstream str;
			str << "Reach " << reachKey;
			logger_.log(str.str());

			/// Create a new mysqlx::TableInsert object. This seemed the best way to INSERT PERSiST data
			TableInsert ti = t.insert("timestep", "reach_id", "land_id", "model_run_id", "flow", "nitrate", "ammonium", "organic_nitrogen", "volume");

			/// Try to start a new MySQL transaction
			try
			{
				dbt.start();
			}
			catch (DbException& err)
			{
				throw err;
			}

			/// For each row of data in the PERSiST results set (for this landscape unit and reach)...
			for (unsigned int i = 0; i < res.SubCatchRes[reachKey]->Count; ++i)
			{
				ti.values(Row(
					i + 1,
					reachId,
					landId,
					modelRun_.getId(),
					res.SubCatchRes[reachKey]->Soil[0].Data[landCounter][i],	// Flow
					res.SubCatchRes[reachKey]->Soil[1].Data[landCounter][i],	// Nitrate
					res.SubCatchRes[reachKey]->Soil[2].Data[landCounter][i],	// Ammonium
					res.SubCatchRes[reachKey]->Soil[4].Data[landCounter][i],	// Organic N
					res.SubCatchRes[reachKey]->Soil[3].Data[landCounter][i]		// Volume
				));
			}

			/// Try to COMMIT
			/// Note that this will INSERT data for ALL time steps, ONE landscape unit and ONE reach
			try
			{
				dbt.end(ti);
			}
			catch (DbException& err)
			{
				throw DbException("Failed to INSERT direct runoff results", ExitCode::insertLandResults);
			}
		}

		++landCounter;
	}

	/// If we're in debug mode, write timing info to the console.
	logger_.log("Finished inserting DIRECT RUNOFF results");

	duration = (std::clock() - timer) / (double)CLOCKS_PER_SEC;
	std::stringstream str;
	str << "INSERT COMPLETE IN: " << duration << " seconds" << std::endl;
	logger_.log(str.str());
}

/**
 * Inserts land results into the result_set_value_reach_land_groundwater table for this model run
 *
 * @param[in]	schema is the MySQL schema to use
 * @param[in]	indexers is the complete set of indexes used by the model run
 * @param[in]	results is the PERSiST model results object
 * @param[in]	output is the PERSiST CLI-compatible output level flag
 */
void DbResults::insertLandGroundwaterResults(Schema& schema, DbModelIndexers& indexers, CResultsGroup& res, int output)
{
	//	std::string tempTable = createTemporaryTable("result_set_value_reach_land");

		/// Get a table object for the correct table from the schema. 
		/// It's like this because explicitly creating a TableInsert object appeared to be the
		/// best/most performant way to convert a PERSiST results object to database rows.
	//	auto t = schema.getTable(tempTable.c_str());
	auto t = schema.getTable("result_set_value_reach_land_groundwater");

	/// Start a timer so we can check performance for this results group
	double duration;
	std::clock_t timer = std::clock();

	logger_.log("Inserting GROUNDWATER results");

	unsigned int landCounter = 0;

	DbTransaction dbt(session_);

	/// For every landscape unit in the system...
	for (auto land : indexers["Landscape units"])
	{
		std::stringstream str;
		str << "Land " << land.getName();
		logger_.log(str.str());

		/// Temporarily store the current landscape unit ID.
		unsigned int landId = land.getId();

		/// For every reach in the system...
		for (auto reach : indexers["Reaches"])
		{
			/// Temporarily store the current reach name and ID.
			std::string reachKey = reach.getName();
			unsigned int reachId = reach.getId();

			std::stringstream str;
			str << "Reach " << reachKey;
			logger_.log(str.str());

			/// Create a new mysqlx::TableInsert object. This seemed the best way to INSERT PERSiST data
			TableInsert ti = t.insert("timestep", "reach_id", "land_id", "model_run_id", "flow", "nitrate", "ammonium", "organic_nitrogen", "volume");

			/// Try to start a new MySQL transaction
			try
			{
				dbt.start();
			}
			catch (DbException& err)
			{
				throw err;
			}

			/// For each row of data in the PERSiST results set (for this landscape unit and reach)...
			for (unsigned int i = 0; i < res.SubCatchRes[reachKey]->Count; ++i)
			{
				ti.values(Row(
					i + 1,
					reachId,
					landId,
					modelRun_.getId(),
					res.SubCatchRes[reachKey]->Ground[0].Data[landCounter][i],	// Flow
					res.SubCatchRes[reachKey]->Ground[1].Data[landCounter][i],	// Nitrate
					res.SubCatchRes[reachKey]->Ground[2].Data[landCounter][i],	// Ammonium
					res.SubCatchRes[reachKey]->Ground[4].Data[landCounter][i],	// Organic N
					res.SubCatchRes[reachKey]->Ground[3].Data[landCounter][i]	// Volume
				));
			}

			/// Try to COMMIT
			/// Note that this will INSERT data for ALL time steps, ONE landscape unit and ONE reach
			try
			{
				dbt.end(ti);
			}
			catch (DbException& err)
			{
				throw DbException("Failed to INSERT groundwater results", ExitCode::insertLandResults);
			}
		}

		++landCounter;
	}

	/// If we're in debug mode, write timing info to the console.
	logger_.log("Finished inserting GROUNDWATER results");

	duration = (std::clock() - timer) / (double)CLOCKS_PER_SEC;
	std::stringstream str;
	str << "INSERT COMPLETE IN: " << duration << " seconds" << std::endl;
	logger_.log(str.str());
}

/**
 * Inserts land results into the result_set_value_reach_land_groundwater table for this model run
 *
 * @param[in]	schema is the MySQL schema to use
 * @param[in]	indexers is the complete set of indexes used by the model run
 * @param[in]	results is the PERSiST model results object
 * @param[in]	output is the PERSiST CLI-compatible output level flag
 */
void DbResults::insertLandHydrologyResults(Schema& schema, DbModelIndexers& indexers, CResultsGroup& res, int output)
{
	//	std::string tempTable = createTemporaryTable("result_set_value_reach_land");

		/// Get a table object for the correct table from the schema. 
		/// It's like this because explicitly creating a TableInsert object appeared to be the
		/// best/most performant way to convert a PERSiST results object to database rows.
	//	auto t = schema.getTable(tempTable.c_str());
	auto t = schema.getTable("result_set_value_reach_land_hydrology");

	/// Start a timer so we can check performance for this results group
	double duration;
	std::clock_t timer = std::clock();

	logger_.log("Inserting HYDROLOGY results");

	unsigned int landCounter = 0;

	DbTransaction dbt(session_);

	/// For every landscape unit in the system...
	for (auto land : indexers["Landscape units"])
	{
		std::stringstream str;
		str << "Land " << land.getName();
		logger_.log(str.str());

		/// Temporarily store the current landscape unit ID.
		unsigned int landId = land.getId();

		/// For every reach in the system...
		for (auto reach : indexers["Reaches"])
		{
			/// Temporarily store the current reach name and ID.
			std::string reachKey = reach.getName();
			unsigned int reachId = reach.getId();

			std::stringstream str;
			str << "Reach " << reachKey;
			logger_.log(str.str());

			/// Create a new mysqlx::TableInsert object. This seemed the best way to INSERT PERSiST data
			TableInsert ti = t.insert("timestep", "reach_id", "land_id", "model_run_id", "rain", "snow_fall", "snow_melt", "snow_depth", "precipitation_depth", "potential_evapotranspiration", "actual_evapotranspiration", "precipitation_depth_added_to_land", "inputs9", "snow_water");

			/// Try to start a new MySQL transaction
			try
			{
				dbt.start();
			}
			catch (DbException& err)
			{
				throw err;
			}

			/// For each row of data in the PERSiST results set (for this landscape unit and reach)...
			for (unsigned int i = 0; i < res.SubCatchRes[reachKey]->Count; ++i)
			{
				ti.values(Row(
					i + 1,
					reachId,
					landId,
					modelRun_.getId(),
					res.SubCatchRes[reachKey]->Hydrology[0].Data[landCounter][i],	// Rain
					res.SubCatchRes[reachKey]->Hydrology[1].Data[landCounter][i],	// Snow fall
					res.SubCatchRes[reachKey]->Hydrology[2].Data[landCounter][i],	// Snow melt
					res.SubCatchRes[reachKey]->Hydrology[3].Data[landCounter][i],	// Snow depth
					res.SubCatchRes[reachKey]->Hydrology[4].Data[landCounter][i],	// Precipitation depth
					res.SubCatchRes[reachKey]->Hydrology[5].Data[landCounter][i],	// Potential evapotranspiration
					res.SubCatchRes[reachKey]->Hydrology[6].Data[landCounter][i],	// Actual evapotranspiration
					res.SubCatchRes[reachKey]->Hydrology[7].Data[landCounter][i],	// Precipitation depth added to land
					res.SubCatchRes[reachKey]->Hydrology[8].Data[landCounter][i],	// Inputs[9]
					res.SubCatchRes[reachKey]->Hydrology[9].Data[landCounter][i]	// Snow water
				));
			}

			/// Try to COMMIT
			/// Note that this will INSERT data for ALL time steps, ONE landscape unit and ONE reach
			try
			{
				dbt.end(ti);
			}
			catch (DbException& err)
			{
				throw DbException("Failed to INSERT hydrology results", ExitCode::insertLandResults);
			}
		}

		++landCounter;
	}

	/// If we're in debug mode, write timing info to the console.
	logger_.log("Finished inserting HYDROLOGY results");

	duration = (std::clock() - timer) / (double)CLOCKS_PER_SEC;
	std::stringstream str;
	str << "INSERT COMPLETE IN: " << duration << " seconds" << std::endl;
	logger_.log(str.str());
}

/**
 * Delete results from the specified table for the specified model run 
 *
 * @param[in]	modelRunId identifies the model run for which to delete results
 */
void DbResults::clear(unsigned int modelRunId)
{
	logger_.log("Begin DELETEing results");

	/// Start a timer, to time the overall INSERT process
	double duration;
	std::clock_t timer = std::clock();

	/// MySQL DELETE statement to remove results from the specified table for the specified model run
	std::stringstream sqlStmt;
//	sqlStmt << "CALL new_delete_results(" << modelRunId << ")";
	sqlStmt << "CALL delete_results(" << modelRunId << ")";

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
		throw DbException(err, ExitCode::deleteFailed);
	}

	/// If we're in debug mode, write timing info to the console.
	logger_.log("Finished DELETEing REACH results");

	duration = (std::clock() - timer) / (double)CLOCKS_PER_SEC;
	std::stringstream str;
	str << "DELETE COMPLETE IN: " << duration << " seconds" << std::endl;
	logger_.log(str.str());
}

/**
 * CREATE a TEMPORARY table LIKE the supplied table
 *
 * @param[in]	fromTable: the name of the table from which to create a TEMPORARY table
 * @param[out]	the name of the new TEMPORARY table
 */
std::string DbResults::createTemporaryTable(std::string const& fromTable)
{
	std::string tempTable = "temp_" + fromTable;

	/// TEMPORARY tables should be automatically DROPped when a session ends, 
	/// but may as well make sure
	dropTemporaryTable(tempTable);

	/// MySQL CREATE statement to make the temporary table
	std::stringstream stmt;
	stmt << "CREATE TEMPORARY TABLE " << tempTable << " LIKE " << fromTable;

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(stmt.str().c_str());

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::createTempTableSqlError);
	}

	return tempTable;
}


/**
 * DROP the named TEMPORARY table
 *
 * @param[in]	tableName is the name of the TEMPORARY table to be DROPped
 */
void DbResults::dropTemporaryTable(std::string const& tableName)
{
	/// MySQL SELECT statement to get meta data for the parameter set with the supplied ID
	std::stringstream stmt;
	stmt << "DROP TEMPORARY TABLE IF EXISTS " << tableName;

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(stmt.str().c_str());

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::dropTempTableSqlError);
	}
}

/**
 * Copy results over from the TEMPORARY table
 */
void DbResults::updateResultsTable(std::string const& targetTableName)
{
	const char * stmt = R"(	
							CALL update_results_table(?, ?)
						)";

	std::stringstream sourceTableName;
	sourceTableName << "temp_" << targetTableName;

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(stmt);

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(targetTableName.c_str()).bind(sourceTableName.str().c_str()).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::updateSolarRadiationSqlError);
	}
}
