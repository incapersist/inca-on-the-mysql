/**
 * The <code>DbInputSeries</code> class fetches PERSiST driving data from a MySQL database, via a supplied session.
 * Where fetched input data does not include data in the solar_radiation column, Solar Radiation is calculated and
 * added back to the input_hydrology_data table to use in the next model run
 *
 * @param[in]	session is the MySQL connection
 */
#pragma once
#include <mysqlx/xdevapi.h>
#include "DbModelIndexers.h"
#include "DbModelRun.h"
#include "Logger.h"
#include "CInputSeriesContainer.h"
#include "CParSet.h"

class DbInputSeries
{
public:
	/// Store the supplied session pointer as a member variable
	DbInputSeries(const std::string& schema, std::shared_ptr<mysqlx::Session>& session, bool forceSolarCalc);
	~DbInputSeries();

	/**
	 * Fetches driving data, from the input file identified by inputFileId,
	 * from the MySQL database identified by the session_ member.
	 *
	 * @param[in]	inputFileId is the database ID of the file for which to get reach counts
	 * @param[in]	modelIndexers contains indexers for the model run
	 * @param[in]	dbModelRun is the model run meta data
	 * @param[in]	parset is the parameter set used in this model run
	 * @return		A PERSiST input data object containing data retrieved from the database
	 * @throw		A simple, generic exception if anything goes wrong with the retrieval
	 */
	void fetch(CInputSeriesContainer& inputs, DbModelIndexers& modelIndexers, DbModelRun& dbModelRun, CParSet& parset);

	bool isSolarRadiationPresent() { return isSolarRadiationPresent_;  }

private:
	std::string schema_;						/// The MySQL schema containing model data
	std::shared_ptr<mysqlx::Session> session_;	/// A local copy of a pointer to the active database session
	Logger& logger_;							/// Logging object;
	bool forceSolarCalc_;						/// Whether to always calculate solar radiation
	bool isSolarRadiationPresent_;				/// Indicates whether the inputs data includes solar radiation

	/**
	 * Retrieve from the database the distinct reach IDs used by the input data, as
	 * identified by inputFileId
	 *
	 * @param[in]	inputFileId is the database ID of the file, passed from the fetch function
	 * @throw		A simple, generic exception if anything goes wrong with the retrieval
	 */
	std::vector<DbModelIndex> getInputReachIndexes(unsigned int inputFileId);

	/**
	 * Retrieve from the database the data for a single reach (identified by
	 * reachId), for the input file identified by inputFileId
	 *
	 * @param[in]	reach is the database index pair of the reach for which to fetch data
	 * @param[in]	inputFileId is the database ID of the file, passed from the fetch function
	 * @param[in]	dbModelRun is the model run meta data
	 * @param[in]	parset is the parameter set used in this model run
	 * @throw		A simple, generic exception if anything goes wrong with the retrieval
	 */
	CInputSeries* fetchData(DbModelIndex& reach, unsigned int inputFileId, DbModelRun& dbModelRun, CParSet& parset);

	/**
	 * Retrieve from the database the driving data for the specified reach, and add the data to the
	 * supplied PERSiST input container object
	 *
	 * @param[in/out]	inputs is the PERSiST inputs data container object
	 * @param[in]		inputFileId identifies the correct input file to grab data for
	 * @param[in]		dbModelRun is the model run meta data
	 * @param[in]		timesteps is the number of input data rows to retrieve
	 * @param[in]		parset is the parameter set used in this model run
	 */
	void fetchReachInputs(CInputSeriesContainer& inputs, DbModelIndex& reach, unsigned int inputFileId, DbModelRun& dbModelRun, CParSet& parset);

	/**
	 * Calculate solar radiation and add the calculated data back to the input_hydrology_data table
	 *
	 * @param[in]	input is the PERSiST input data container object
	 * @param[in]	parset is the parameter set used in this model run
	 * @param[in]	reach is index object for the current reach
	 * @param[in]	dbModelRun is the model run meta data
	 * @throw		A simple, generic exception if anything goes wrong with the update
	 */
	void insertSolar(CInputSeries& input, CParSet& parset, DbModelIndex& reach, DbModelRun& dbModelRun);

	/**
	 * DROP the temporary table used to UPDATE input_hydrology_data.solar_radiation
	 */
	void dropTemporaryTable();

	/**
	 * CREATE the temporary table used to UPDATE input_hydrology_data.solar_radiation
	 */
	void createTemporaryTable();

	/**
	 * UPDATE `input_hydrology_data`.`solar_radiation` from `temp_input_hydrology_data`
	 *
	 * @param[in]	reachId is the ID of the reach to be updated (used in WHERE clause)
	 * @param[in]	inputFileId is the file ID of the data to be updated (used in WHERE clause)
	 */
	void updateSolarRadiation(unsigned int reachId, unsigned int inputFileId);

};

