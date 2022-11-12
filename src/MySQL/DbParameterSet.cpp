#include "DbParameterSet.h"
#include <iostream>
#include "DbException.h"
#include "Constant.h"

using namespace mysqlx;

/**
 * The <code>DbParameterSet</code> class fetches a complete PERSiST parameter set from a MySQL database.
 *
 * @param[in]	session is the active MySQL database connection
 */
DbParameterSet::DbParameterSet(std::shared_ptr<mysqlx::Session> session) : session_(session)
{
}

DbParameterSet::~DbParameterSet()
{
}

/**
 * Retrieve a complete PERSiST parameter set from a MySQL database
 *
 * @param[in]	modelRun is the meta data for the model run to use
 * @param[in]	indexers is a complete set of indexes for the model run
 * @return		a new PERSiST parameter set object
 */
CParSet DbParameterSet::fetch(DbModelRun& modelRun, DbModelIndexers &indexers)
{
	/// Try to fetch parameter set meta data for the parameter set identified by modelRun
	try
	{
		fetchMeta(modelRun.getParameterSetId());

		/// Member variable startDate_ is a string representation of a database date, but a PERSiST
		/// parameter set object requires start date as a dateCl::Date object, so get that
		auto startDate = dateFromString(startDate_);

		parset_.LongLand.clear();
		parset_.ShortLand.clear();
		unsigned int landCounter = 0;

		/// For each landscape unit in the model run...
		for (auto& land : indexers["Landscape units"])
		{
			/// Add the landscape unit name to the parameter set
			parset_.LongLand.push_back(land.getName());
			parset_.ShortLand.push_back(land.getName());

			/// Try to fetch parameters for the current landscape unit
			fetchLandParameters(modelRun.getParameterSetId(), landCounter, land.getId());

			++landCounter;
		}

//		parset_.Reaches.clear();
		parset_.LandCount(indexers["Landscape units"].size());

		/// For each reach in the model run...
		for (auto& reach : indexers["Reaches"])
		{
			/// Create a new PERSiST reach object
			CReachPar r;
			CSubPar s(indexers["Landscape units"].size());

			/// Try to fetch the parameters for the current reach
			fetchReachParameters(modelRun.getParameterSetId(), reach.getId(), r, s);

			/// Each reach includes a landscape unit percentage for each landscape unit in the model run,
			/// so let's try to fetch those
			unsigned int landCounter = 0;

			/// For each landscape unit in the model run...
			for (auto& land : indexers["Landscape units"])
			{
				/// Try to fetch landscape unit percentage for the current landscape unit in the current reach
				fetchReachLandParameters(s, modelRun.getParameterSetId(), reach.getId(), landCounter, land.getId());

				++landCounter;
			}
		
			/// Add the reach name to the PERSiST reach object
			r.Name(reach.getName());

			/// Add the new reach object to the parameter set
			parset_.ReachPar.insert(reachValueType(reach.getName(), r));
			parset_.SubPar.insert(subValueType(reach.getName(), s));
		}
	
		fetchStreamParameters(modelRun.getParameterSetId(), 0, 8);
			
		landCounter = 0;
		parset_.Reaches = parset_.ReachPar.size();

		/// Return a copy of the PERSiST parameter set object
		return parset_;
	}
	catch (DbException &err)
	{
		throw err;
	}
}

/**
 * Fetch meta data for a parameter set
 *
 * @param[in]	parameterSetId is the ID of the parameter set for which to fetch meta data
 */
void DbParameterSet::fetchMeta(unsigned int parameterSetId)
{
	/// MySQL SELECT statement to get meta data for the parameter set with the supplied ID
	const char * stmt = R"(	SELECT 
								id,
								user_id,
								reference,
								DATE_FORMAT(when_created, '%Y-%m-%d %H:%i:%S') AS when_created,
								DATE_FORMAT(start_date, '%Y-%m-%d') AS start_date,
								timesteps,
								model_version_id, 
								model_version_core_id, 
								baseline_smd,
								COALESCE(step_size, 86400) AS step_size
							FROM 
								parameter_set
							WHERE
								id = ?)";

	/// Create a statement object from the statement string
	SqlStatement sql = session_->sql(stmt);

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(parameterSetId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::parameterSetSqlError);
	}

	/// If the query executed and returned results...
	if (result.hasData())
	{
		/// Fetch the first row of data (there should only be one row)
		auto row = result.fetchOne();

		/// Copy results row data into the relevant member variables
		id_ = !row[0].isNull() ? int(row[0]) : 0;
		userId_ = !row[1].isNull() ? int(row[1]) : 0;
		reference_ = !row[2].isNull() ? std::string(row[2]) : "";
		createDate_ = !row[3].isNull() ? std::string(row[3]) : "";
		startDate_ = !row[4].isNull() ? std::string(row[4]) : "";
		timesteps_ = !row[5].isNull() ? int(row[5]) : 0;
		modelVersionId_ = !row[6].isNull() ? int(row[6]) : 0;
		modelVersionCoreId_ = !row[7].isNull() ? int(row[7]) : 0;
		baselineSmd_ = !row[8].isNull() ? double(row[8]) : 0;
		stepSize_ = !row[9].isNull() ? int(row[9]) : 0;
	}
	else
	{
		throw DbException(Error("No parameter set available"), ExitCode::noParameterSet);
	}
}

/**
 * Fetch the parameters, indexed by landscape units, from the persist.parameter_set_value_land database table
 * and add the fetched parameters to the PERSiST parameter set object
 *
 * @param[in]	modelLandId is the identifier that PERSiST uses for a particular landscape unit
 * @param[in]	dbLandId is the database ID of the same landscape unit
 */
void DbParameterSet::fetchLandParameters(unsigned int parameterSetId, unsigned int modelLandId, unsigned int dbLandId)
{
	/// Create a MySQL query to select landscape unit-indexed parameters from the database
	SqlStatement sql = session_->sql("SELECT * FROM parameter_set_value_land WHERE parameter_set_id = ? AND landscape_unit_id = ? ORDER BY landscape_unit_id, parameter_id");

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(parameterSetId).bind(dbLandId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::landParameterSqlError);
	}

	/// If the query executed and returned results...
	if (result.hasData())
	{
		/// Get all of the results
		auto rows = result.fetchAll();

		/// For each row in the results set
		for (auto row : rows)
		{
			/// Add fetched parameter to the correct place in the parameter set object, 
			/// for a single landscape unit.

			int index = (int)row[2];

			if (index <= NUM_CELL_INITIAL)
			{
				parset_.CellPar[modelLandId].Initial[index - 1] = (double)row[3];
			}
			else if (index > NUM_CELL_INITIAL && index <= (NUM_CELL_INITIAL + NUM_CELL_PARS))
			{
				parset_.CellPar[modelLandId].Par[index - (NUM_CELL_INITIAL + 1)] = (double)row[3];
			}
			else
			{
				parset_.CellPar[modelLandId].TC[index - (NUM_CELL_INITIAL + NUM_CELL_PARS + 1)] = (double)row[3];
			}
		}
	}
	else
	{
		throw DbException(Error("No land parameters available for parameter set"), ExitCode::noLandParameters);
	}
}

/**
 * Fetch the parameters, indexed by stream, from the persist.parameter_set_value_stream database table
 * and add the fetched parameters to the parameter set object
 *
 * @param[in]	modelLandId is the identifier that PERSiST uses for a particular landscape unit
 * @param[in]	dbLandId is the database ID of the same landscape unit
 */
void DbParameterSet::fetchStreamParameters(unsigned int parameterSetId, unsigned int modelStreamId, unsigned int dbStreamId)
{
	/// Create a MySQL query to select landscape unit-indexed parameters from the database
	SqlStatement sql = session_->sql("SELECT * FROM parameter_set_value_stream WHERE parameter_set_id = ? AND stream_id = ? ORDER BY stream_id, parameter_id");

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(parameterSetId).bind(dbStreamId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::landParameterSqlError);
	}

	/// If the query executed and returned results...
	if (result.hasData())
	{
		/// Get all of the results
		auto rows = result.fetchAll();

		/// For each row in the results set
		for (auto row : rows)
		{
			/// Add fetched parameter to the correct place in the parameter set object, 
			/// for a single stream.

			int index = (int)row[2];

			if (index < 125)
			{
				parset_.RiverPar.Initial[index-121] = (double)row[3];
			}
			else
			{
				parset_.RiverPar.Pars[0] = (double)row[3];
			}
		}
	}
	else
	{
		throw DbException(Error("No land parameters available for parameter set"), ExitCode::noLandParameters);
	}
}

/**
 * Fetch the parameters, indexed by reach, from the persist.parameter_set_value_reach database table
 *
 * @param[in]	dbReachId is the database ID of the reach for which to fetch parameters
 * @return		returns a PERSiST object containing parameters for the reach
 */
void DbParameterSet::fetchReachParameters(unsigned int parameterSetId, unsigned int dbReachId, CReachPar& r, CSubPar& s)
{
	/// Create a MySQL query to select reach-indexed parameters from the database
	SqlStatement sql = session_->sql("SELECT * FROM parameter_set_value_reach WHERE parameter_set_id = ? AND reach_id = ? ORDER BY reach_id, parameter_id");

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(parameterSetId).bind(dbReachId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::reachParameterSqlError);
	}

	/// If the query executed and returned results...
	if (result.hasData())
	{
		/// Get all of the results
		auto rows = result.fetchAll();

		/// For each row in the results set
		for (auto row : rows)
		{
			int index = (int)row[2];

			if (index <= 96)
			{
				r.Par[index - 77] = (double)row[3];
			}
			else
			{
				if (index < 116)
				{
					s.Par[index - 97] = (double)row[3];
				}
				else
				{
					s.Dep[index - 116] = (double)row[3];
				}
			}
		}
	}
	else
	{
		throw DbException(Error("No reach parameters available for parameter set"), ExitCode::noReachParameters);
	}
}

/**
 * Fetch the parameters, indexed by reaches and landscape units, from the
 * persist.parameter_set_value_reach_land database table. Fetched parameters are
 * added to the supplied PERSiST reach parameter object
 *
 * @param[in]	r is the PERSiST reach parameter object that fetched parameters will be added to
 * @param[in]	dbReachId is the database ID of the reach for which to fetch parameters
 * @param[in]	modelLandId is the identifier that PERSiST uses for a particular landscape unit
 * @param[in]	dbLandId is the database ID of the same landscape unit
 */
void DbParameterSet::fetchReachLandParameters(CSubPar& s, unsigned int parameterSetId, unsigned int dbReachId, unsigned int modelLandId, unsigned int dbLandId)
{
	/// Create a MySQL query to select reach and landscape unit-indexed parameters from the database
	SqlStatement sql = session_->sql("SELECT * FROM parameter_set_value_reach_land WHERE parameter_set_id = ? AND reach_id = ? AND landscape_unit_id = ? ORDER BY reach_id, landscape_unit_id, parameter_id");

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(parameterSetId).bind(dbReachId).bind(dbLandId).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::reachLandParameterSqlError);
	}

	/// If the query executed and returned results...
	if (result.hasData())
	{
		/// Get all of the results
		auto rows = result.fetchAll();

		/// For each row in the results set
		for (auto row : rows)
		{
			/// Add fetched parameter to the correct place in the PERSiST parameter set object, 
			/// for a single reach and landscape unit.
			/// persist.parameter.id identifies the parameters; this is what's used in the switch
			switch ((int)row[1])
			{
				case 120:	s.Land[modelLandId] = (float)row[4];
							break;
			}
		}
	}
	else
	{
		throw DbException(Error("No reach/land parameters available for parameter set"), ExitCode::noReachLandParameters);
	}
}

/**
 * Converts a string representation of a date to a date object
 *
 * @param[in]	startDate is the string representation of the parameter set start date
 * @return		a new date object
 */
dateCl::Date DbParameterSet::dateFromString(std::string& startDate)
{
	short day, month, year;
	char c;

	std::istringstream is(startDate);

	is >> year >> c >> month >> c >> day;

	return dateCl::Date(month, day, year);
}
