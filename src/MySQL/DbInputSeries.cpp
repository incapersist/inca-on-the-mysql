#include "DbInputSeries.h"
#include <iostream>
#include <ctime>
#include "DbException.h"
#include "DbTransaction.h"

using namespace mysqlx;

DbInputSeries::DbInputSeries(const std::string& schema,
								std::shared_ptr<mysqlx::Session>& session, 
								bool forceSolarCalc)						:	schema_(schema),
																				session_(session),
																				logger_(Logger::GetInstance()),
																				forceSolarCalc_(forceSolarCalc)
{
}

DbInputSeries::~DbInputSeries()
{
}

/**
 * Fetches driving data, from the input file identified by inputFileId,
 * from the MySQL database identified by the session_ member.
 *
 * @param[in]	inputFileId is the database ID of the file for which to get reach counts
 * @param[in]	modelIndexers contains indexers for the model run
 * @param[in]	timesteps is the number of timesteps used in this model run
 * @param[in]	parset is the parameter set used in this model run
 * @return		A PERSiST input data object containing data retrieved from the database
 * @throw		A simple, generic exception if anything goes wrong with the retrieval
 */
void DbInputSeries::fetch(CInputSeriesContainer& inputs, DbModelIndexers& modelIndexers, DbModelRun& dbModelRun, CParSet& parset)
{
	const unsigned int inputFileId = dbModelRun.getInputFileId();

	/// Check how many reaches are represented in the driving data for this model run
	std::vector<DbModelIndex> indexes;

	try
	{
		indexes = getInputReachIndexes(inputFileId);

		/// Set up a PERSiST model container object to store the retrieved driving data.
		/// A copy of this object will be returned by if this functions if the retrieval is successful.
//		CInputSeriesContainer inputs;
//		inputs.inputCount = indexes.size();

		/// If the driving data in the database only contains data for a single reach...
		if (indexes.size() == 1)
		{
			fetchReachInputs(inputs, indexes[0], inputFileId, dbModelRun, parset);
		}
		/// If the driving data in the database contains data for more than one reach
		else
		{
			/// Check the the number of reaches described by the driving data for this model run
			/// matches the number of reaches in the parameter set
			if (indexes.size() == modelIndexers["Reaches"].size())
			{
				/// For every reach in the system...
				for (auto reach : modelIndexers["Reaches"])
				{
					fetchReachInputs(inputs, reach, inputFileId, dbModelRun, parset);
				}
			}
			/// If there is more than one reach represented in the driving data, but the number of
			/// reaches does not match the reach count in the parameter set, there's no way to know
			/// how to assign the data to reaches. so throw an exception
			else
			{
				throw DbException(Error("Couldn't match reaches"), ExitCode::inputReachAlignment);
			}
		}

	//	return inputs;
	}
	catch(DbException& err)
	{
		throw err;
	}
}

/**
 * Retrieve from the database the driving data for the specified reach, and add the data to the
 * supplied PERSiST input container object
 *
 * @param[in/out]	inputs is the PERSiST inputs data container object
 * @param[in]		reach identifies the reach to get data for
 * @param[in]		inputFileId identifies the correct input file to grab data for
 * @param[in]		timesteps is the number of input data rows to retreve
 * @param[in]		parset is the parameter set used in this model run
 */
void DbInputSeries::fetchReachInputs(CInputSeriesContainer& inputs, DbModelIndex& reach, unsigned int inputFileId, DbModelRun& dbModelRun, CParSet& parset)
{
	/// Create a new PERSiST input data object to store driving data for a single reach
	CInputSeries* input;

	/// Try to fetch driving data for a single reach
	try
	{
		input = fetchData(reach, inputFileId, dbModelRun, parset);
		inputs.Add(reach.getName(), input);
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::data);
	}

	/// Set the number of timesteps in the input object to the size of the object. It's like this
	/// to avoid modifying the PERSiST CInput class
//	inputs.TimeSteps = input.size();

	/// Add the new CInput object to the inputs container
//	inputs.inputs.insert(inputValueType(reach.getName(), input));
}

/**
 * Retrieve from the database the distinct reach IDs used by the input data, as
 * identified by inputFileId
 *
 * @param[in]	inputFileId is the database ID of the file, passed from the fetch function
 * @throw		A simple, generic exception if anything goes wrong with the retrieval
 */
std::vector<DbModelIndex> DbInputSeries::getInputReachIndexes(unsigned int inputFileId)
{
	/// MySQL SELECT statement to get reach indexes described in the input series data
	const char * stmt = R"(	SELECT
								id,
								reference
							FROM
								indexer_index
							WHERE id IN (SELECT DISTINCT reach_id FROM input_hydrology_data WHERE input_file_id = ?))";

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(stmt);

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(inputFileId).execute();
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::reachIndexSqlError);
	}

	/// If the query produced data...
	if (result.hasData())
	{
		/// Get all the query data
		auto rows = result.fetchAll();

		std::vector<DbModelIndex> indexes;

		/// Fill the PERSiST object with query data
		for (auto row : rows)
		{
			indexes.push_back(DbModelIndex((int)row[0], (std::string)row[1]));
		}

		return indexes;
	}
	/// Otherwise throw an exception
	else
	{
		throw DbException(Error("No input data reach index found"), ExitCode::noInputReachIndex);
	}
}

/**
 * Retrieve from the database the data for a single reach (identified by
 * reachId), for the input file identified by inputFileId
 *
 * @param[in]	reach is the database index pair of the reach for which to fetch data
 * @param[in]	inputFileId is the database ID of the file, passed from the fetch function
 * @param[in]	timesteps is the number of input data rows to retrieve
 * @param[in]	parset is the parameter set used in this model run
 * @throw		A simple, generic exception if anything goes wrong with the retrieval
 */
CInputSeries* DbInputSeries::fetchData(DbModelIndex& reach, unsigned int inputFileId, DbModelRun& dbModelRun, CParSet& parset)
{
	/// MySQL query to get drivng data as for reachId, modelRun
	SqlStatement sql = session_->sql("SELECT * FROM input_hydrology_data WHERE reach_id = ? AND input_file_id = ? LIMIT ?");

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(reach.getId()).bind(inputFileId).bind(dbModelRun.getTimeSteps()).execute();
	}
	catch (const Error &err)
	{
		throw DbException(err, ExitCode::inputDataSqlError);
	}

	/// If the query produced data...
	if (result.hasData())
	{
		/// Create a new PERSiST model input object to store the retrieved data
		CInputSeries* input = new CInputSeries(result.count(), 5, 1);

		/// Get all the query data
		auto rows = result.fetchAll();

		/// Fill the PERSiST object with query data
		for (auto row : rows)
		{
			const unsigned int index = (int)row[1] - 1;
			input->Data[0][index] = !row[6].isNull() ? (double)row[6] : 0;
			input->Data[1][index] = !row[7].isNull() ? (double)row[7] : 0;
			input->Data[2][index] = !row[3].isNull() ? (double)row[3] : 0;
			input->Data[3][index] = !row[4].isNull() ? (double)row[4] : 0;

			input->useNewHydrology(row[6].isNull());

			if (!row[5].isNull())
			{
				input->Data[4][index] = (double)row[4];
				isSolarRadiationPresent_ = true;
			}
			else
			{
				isSolarRadiationPresent_ = false;
			}
		}

		input->Missing(!isSolarRadiationPresent_);

		/// If the retrieved data is missing solar radiation, then calculate it
		/// and add the calculated data back to the input_hydrology_data table so
		/// it can be used on the next model run
		if (input->Missing() || forceSolarCalc_)
		{
			insertSolar(*input, parset, reach, dbModelRun);
		}

		return input;
	}
	/// Otherwise throw an exception
	else
	{
		throw DbException(Error("No input data for reach"), ExitCode::noInputData);
	}
}

/**
 * Calculate solar radiation and add the calculated data back to the input_hydrology_data table.
 * It's faster to fill a temporary table with all of the input data - including the newly-
 * calculated solar radiation - and then UPDATE, than UPDATE the table directly
 *
 * @param[in]	input is the PERSiST input data container object
 * @param[in]	parset is the parameter set used in this model run
 * @param[in]	reach is index object for the current reach
 * @param[in]	dbModelRun is the model run meta data
 * @throw		A simple, generic exception if anything goes wrong with the update
 */
void DbInputSeries::insertSolar(CInputSeries& input, CParSet& parset, DbModelIndex& reach, DbModelRun& dbModelRun)
{
	auto reachPar = parset.ReachPar[reach.getName()];

	/// Drop the temporary table, if it already exists
	dropTemporaryTable();
	/// (Re)create the temporary table
	createTemporaryTable();

	/// Calculate solar radiation using the PERSiST calculation routine, using required data
	/// from both the parameter set and information from the current model setup
	input.CalcSolar(reachPar.Par[9], reachPar.Par[10], dbModelRun.getStartDate());

	/// Get a table object for the correct table from the schema. 
	/// It's like this because explicitly creating a TableInsert object appeared to be the
	/// best/most performant way to convert a PERSiST results object to database rows.
	auto schema = session_->getSchema("inca_on_the");
	/// Note that this is INSERTing into a temporary table
	auto t = schema.getTable("temp_input_hydrology_data");

	/// Start a timer so we can check performance for this results group
	double duration;
	std::clock_t timer = std::clock();

	logger_.log("Inserting calculated solar radiation data");

	TableInsert ti = t.insert("reach_id", "timestep", "input_file_id", "air_temperature", "precipitation", "solar_radiation", "her", "smd");

	DbTransaction dbt(session_);

	/// Try to start a new MySQL transaction
	try
	{
		dbt.start();
	}
	catch (DbException& err)
	{
		throw err;
	}

	unsigned int reachId = reach.getId();
	unsigned int inputFileId = dbModelRun.getInputFileId();

	/// For each row of data in the PERSiST results set (for this reach)...
	for (unsigned int i = 0; i < dbModelRun.getTimeSteps(); ++i)
	{
		ti.values(Row(
			reachId,
			i + 1,
			inputFileId,
			input.Data[2][i],
			input.Data[3][i],
			input.Data[4][i],
			input.Data[0][i],
			input.Data[1][i]
			));
	}

	/// We should have built INSERTs for all of the data for this reach,
	/// so try to COMMIT it
	try
	{
		dbt.end(ti);
		/// Once the data are INSERTed into the temporary table, update the original
		/// table with the new values.
		updateSolarRadiation(reachId, inputFileId);
	}
	catch (DbException& err)
	{
		throw DbException("Failed to INSERT solar radiation", ExitCode::insertSolarRadiation);
	}

	/// If we're in debug mode, write timing info to the console.
	logger_.log("Finished updating solar radiation");

	duration = (std::clock() - timer) / (double)CLOCKS_PER_SEC;
	std::stringstream str;
	str << "UPDATE COMPLETE IN: " << duration << " seconds" << std::endl;
	logger_.log(str.str());
}

/**
 * DROP the temporary table used to UPDATE input_hydrology_data.solar_radiation
 */
void DbInputSeries::dropTemporaryTable()
{
	/// MySQL SELECT statement to get meta data for the parameter set with the supplied ID
	const char * stmt = R"(DROP TEMPORARY TABLE IF EXISTS `temp_input_hydrology_data`)";

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(stmt);

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
 * CREATE the temporary table used to UPDATE input_hydrology_data.solar_radiation
 */
void DbInputSeries::createTemporaryTable()
{
	/// MySQL SELECT statement to get meta data for the parameter set with the supplied ID
	const char * stmt = R"(CREATE TEMPORARY TABLE `temp_input_hydrology_data` (
							  `reach_id` int(11) NOT NULL,
							  `timestep` int(11) NOT NULL,
							  `input_file_id` int(11) NOT NULL,
							  `air_temperature` float NOT NULL,
							  `precipitation` float NOT NULL,
							  `solar_radiation` float DEFAULT NULL,
							  `her` float DEFAULT NULL,
							  `smd` float DEFAULT NULL,
							  PRIMARY KEY (`reach_id`,`timestep`,`input_file_id`))
						)";

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(stmt);

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
}

/**
 * UPDATE `input_hydrology_data`.`solar_radiation` from `temp_input_hydrology_data`
 *
 * @param[in]	reachId is the ID of the reach to be updated (used in WHERE clause)
 * @param[in]	inputFileId is the file ID of the data to be updated (used in WHERE clause)
 */
void DbInputSeries::updateSolarRadiation(unsigned int reachId, unsigned int inputFileId)
{
	/// MySQL SELECT statement to get meta data for the parameter set with the supplied ID
	const char * stmt = R"(	UPDATE 
								input_hydrology_data ihd
							JOIN temp_input_hydrology_data temp 
								ON temp.reach_id = ihd.reach_id
								AND temp.timestep = ihd.timestep
								AND temp.input_file_id = ihd.input_file_id
							SET
								ihd.solar_radiation = temp.solar_radiation
							WHERE
								temp.reach_id = ?
								AND temp.input_file_id = ?)";

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(stmt);

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(reachId).bind(inputFileId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::updateSolarRadiationSqlError);
	}

}
