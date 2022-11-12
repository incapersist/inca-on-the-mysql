#include "DbModelRun.h"
#include "DbException.h"
#include <iostream>

using namespace mysqlx;

DbModelRun::DbModelRun() : logger_(Logger::GetInstance())
{
}

DbModelRun::~DbModelRun()
{
}

/**
 * Fetch a row from the persist.model_run table in MySQL
 *
 * @param[in]	session is the current active database connection
 * @param[in]	modelRunId is the id of the model run meta data to fetch
*/
void DbModelRun::fetch(std::shared_ptr<mysqlx::Session> session, unsigned int modelRunId)
{
	/// MySQL SELECT statement to get meta data for the parameter set with the supplied ID
	const char * stmt = R"(	SELECT 
								model_run.id AS id,
								model_run.parameter_set_id AS parameter_set_id,
								model_run.hydrology_input_file_id AS hydrology_input_file_id,
								model_run.observed_input_file_id AS observed_input_file_id,
								model_run.effluent_input_file_id AS effluent_input_file_id,
								model_run.abstraction_input_file_id AS abstraction_input_file_id,
								model_run.no3_fertilizer_input_file_id AS no3_fertilizer_input_file_id,
								model_run.nh4_fertilizer_input_file_id AS nh4_fertilizer_input_file_id,
								model_run.don_fertilizer_input_file_id AS don_fertilizer_input_file_id,
								model_run.deposition_input_file_id AS deposition_input_file_id,
								model_run.land_use_periods_input_file_id AS land_use_periods_input_file_id,
								COALESCE(model_run.stream_network_id, default_stream_network.id) AS stream_network_id,
								model_run.user_id AS user_id,
								model_run.reference AS reference,
								DATE_FORMAT(model_run.when_created, '%Y-%m-%d %H:%i:%S') AS when_created,
								DATE_FORMAT(model_run.start_date, '%Y-%m-%d') AS start_date,
								model_run.timesteps AS timesteps,
								start_date_offset(model_run.id) AS offset, 
								available_timesteps(model_run.id) AS available_timesteps,
                                IF (model_run.stream_network_id IS NULL, 1, 0) AS is_generated
							FROM 
								model_run
                                LEFT JOIN stream_network default_stream_network ON default_stream_network.parameter_set_id = model_run.parameter_set_id AND default_stream_network.is_default = 1
							WHERE
								model_run.id = ?)";

	/// Create a query object to select model run meta data
	SqlStatement sql = session->sql(stmt);

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(modelRunId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::connection);
	}

	/// If results are returned...
	if (result.hasData())
	{
		/// Get the first row (there should be oly one)
		auto row = result.fetchOne();

		/// Copy data into the member variables
		id_ = !row[0].isNull() ? int(row[0]) : 0;
		parameterSetId_ = !row[1].isNull() ? int(row[1]) : 0;
		hydrologyInputFileId_ = !row[2].isNull() ? int(row[2]) : 0;
		observedInputFileId_ = !row[3].isNull() ? int(row[3]) : 0;
		effluentInputFileId_ = !row[4].isNull() ? int(row[4]) : 0;
		abstractionInputFileId_ = !row[5].isNull() ? int(row[5]) : 0;
		no3FertilizerInputFileId_ = !row[6].isNull() ? int(row[6]) : 0;
		nh4FertilizerInputFileId_ = !row[7].isNull() ? int(row[7]) : 0;
		donFertilizerInputFileId_ = !row[8].isNull() ? int(row[8]) : 0;
		depositionInputFileId_ = !row[9].isNull() ? int(row[9]) : 0;
		landUsePeriodsInputFileId_ = !row[10].isNull() ? int(row[10]) : 0;
		streamNetworkId_ = !row[11].isNull() ? int(row[11]) : 0;
		userId_ = !row[12].isNull() ? int(row[12]) : 0;
		reference_ = !row[13].isNull() ? std::string(row[13]) : "";
		createDate_ = !row[14].isNull() ? std::string(row[14]) : "";
		startDate_ = !row[15].isNull() ? std::string(row[15]) : "";
		timesteps_ = !row[16].isNull() ? int(row[16]) : 0;				
		offset_ = !row[17].isNull() ? int(row[17]) : 0;
		availableTimesteps_ = !row[18].isNull() ? int(row[18]) : 0;
		isStructureGenerated_ = (row[19].isNull() || (unsigned int)row[19] == 1) ? true : false;

		if (availableTimesteps_ < timesteps_)
		{
			throw DbException(Error("Not enough input hydrology data available"), ExitCode::tooFewTimesteps);
		}
	}
	else
	{
		throw DbException(Error("No model run meta data available"), ExitCode::noModelRunMeta);
	}
}

// Check if the model run is valid
bool DbModelRun::isValid()
{
	return (hasParameterFile() && hasHydrologyFile());
}

// Check if the model run has a parameter file attached
bool DbModelRun::hasParameterFile()
{
	return (parameterSetId_ && parameterSetId_ > 0);
}

// Check if the model run has an input data file attached
bool DbModelRun::hasHydrologyFile()
{
	return (hydrologyInputFileId_ && hydrologyInputFileId_ > 0);
}

// Check if the model run has an observed data file attached
bool DbModelRun::hasObservedFile()
{
	return (observedInputFileId_ && observedInputFileId_ > 0);
}

// Check if the model run has a reach structure file attached
bool DbModelRun::hasStructureFile()
{
	return (streamNetworkId_ && streamNetworkId_ > 0);
}

// Check if the model run has an abstraction file attached
bool DbModelRun::hasAbstractionFile()
{
	return (abstractionInputFileId_ && abstractionInputFileId_ > 0);
}

// Check if the model run has an effluent file attached
bool DbModelRun::hasEffluentFile()
{
	return (effluentInputFileId_ && effluentInputFileId_ > 0);
}

// Check if the model run has a NO3 fertilizer file attached
bool DbModelRun::hasNo3FertilizerFile()
{
	return (no3FertilizerInputFileId_ && no3FertilizerInputFileId_ > 0);
}

// Check if the model run has an NH4 fertilizer file attached
bool DbModelRun::hasNh4FertilizerFile()
{
	return (nh4FertilizerInputFileId_ && nh4FertilizerInputFileId_ > 0);
}

// Check if the model run has a DON fertilizer file attached
bool DbModelRun::hasDonFertilizerFile()
{
	return (donFertilizerInputFileId_ && donFertilizerInputFileId_ > 0);
}

// Check if the model run has a deposition file attached
bool DbModelRun::hasDepositionFile()
{
	return (depositionInputFileId_ && depositionInputFileId_ > 0);
}

// Check if the model run has a land use periods file attached
bool DbModelRun::hasLandUsePeriodsFile()
{
	return (landUsePeriodsInputFileId_ && landUsePeriodsInputFileId_ > 0);
}

/**
 * Converts a string representation of a date to a date object
 *
 * @param[in]	startDate is the string representation of the parameter set start date
 * @return		a new date object
 */
dateCl::Date DbModelRun::dateFromString(std::string& startDate)
{
	short day, month, year;
	char c;

	std::istringstream is(startDate);

	is >> year >> c >> month >> c >> day;

	return dateCl::Date(month, day, year);
}
