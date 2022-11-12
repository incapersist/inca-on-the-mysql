#include "DbConnection.h"
#include "DbException.h"

#include <iostream>
#include <utility>
#include <sstream>
#include <fstream>
#include <ctime>

using namespace mysqlx;

DbConnection::DbConnection(	const std::string& user,
							const std::string& password,
							const std::string& host,
							unsigned int port,
							const std::string& schema,
							bool isDebug)				:	user_( user ),
															password_( password ),
															host_( host ),
															port_( port ),
															schema_( schema ),
															isDebug_(isDebug),
															logger_(Logger::GetInstance())
{
	Connect();
}

DbConnection::~DbConnection()
{
	session_->close();

	if (no3Fert_) delete no3Fert_;
	if (nh4Fert_) delete nh4Fert_;
	if (donFert_) delete donFert_;
	if (dep_) delete dep_;

	for (std::size_t i = 0; i < abs_.size(); ++i)
	{
		delete abs_[i];
	}
	abs_.clear();

	for (std::size_t i = 0; i < eff_.size(); ++i)
	{
		delete eff_[i];
	}
	eff_.clear();
}

/**
 * Connect to the database using the parameter supplied in the constructor, and store a pointer to the session
 */
void DbConnection::Connect()
{
	logger_.log("Connecting to MySQL...", false);

	try
	{
		session_ = std::shared_ptr<mysqlx::Session>(new Session(host_, port_, user_, password_, schema_));
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::connection);
	}

	logger_.log("done");
}

/**
 * Fetches model input data from the database and stores.
 * Fetched data includes reach structure, parameter set and driving data.
 *
 * @param[in]	modelRunId is the id of the model run in the database(model_run.id)
 *				for which to retrieve input data.
 * @param[in]	forceSolarCalc is a flag that indicates whether solar radiation should always be calculated
 * @return		boolean indicating success
 * @throw		simple throw on any error
 */
bool DbConnection::fetchData(unsigned int modelRunId, bool forceSolarCalc)
{
	/// Write out progress if we're debugging
	logger_.log("Fetching model run meta data...", false);

	try
	{
		/// Try to get model run meta data from the database. This should get a row from
		/// the model_run table where model_run.id = modelRunId
		modelRun_.fetch(session_, modelRunId);

		/// Write out progress if we're debugging
		logger_.log("done");

		/// If the model run has been set up correctly (i.e. if it has parameters and driving data)
		if (modelRun_.isValid())
		{
			/// Write out progress if we're debugging
			logger_.log("Fetching parameter set indexers...", false);

			/// Try to get the model indexers from the database for this model run
			dbModelIndexers_.fetch(session_, modelRunId);

			/// Write out progress if we're debugging
			logger_.log("done");

			/// If the model indexers make sense,
			if (dbModelIndexers_.isValid())
			{
				/// Write out progress if we're debugging
				logger_.log("Fetching reach structure...", false);

				/// Try to build a reach structure from data in the database
				DbStructure dbStructure(session_);
				structure_ = dbStructure.fetch(modelRun_, dbModelIndexers_);

				/// Write out progress if we're debugging
				logger_.log("done");
				logger_.log("Fetching parameter set...", false);

				/// Retrieve parameters from the database for this model run and store
				DbParameterSet dbParameterSet(session_);
				parset_ = dbParameterSet.fetch(modelRun_, dbModelIndexers_);

				parset_.StartDate = modelRun_.getStartDate();
				parset_.TimeSteps = modelRun_.getTimeSteps();

				/// Write out progress if we're debugging
				logger_.log("done");
				logger_.log("Fetching input hydrology...", false);

				/// Retrieve driving data from the database for this model run and store
				DbInputSeries dbInputSeries(schema_, session_, forceSolarCalc);
				dbInputSeries.fetch(inputs_, dbModelIndexers_, modelRun_, parset_);

				/// Write out progress if we're debugging
				logger_.log("done");

				if (modelRun_.hasAbstractionFile())
				{
					/// Write out progress if we're debugging
					logger_.log("Fetching abstraction data...", false);

					DbAbsSeries dbAbsSeries(session_, "abstraction_data");
					abs_ = dbAbsSeries.fetch(dbModelIndexers_, modelRun_);

					/// Write out progress if we're debugging
					logger_.log("done");
				}

				if (modelRun_.hasEffluentFile())
				{
					/// Write out progress if we're debugging
					logger_.log("Fetching effluent data...", false);

					DbEffSeries dbEffSeries(session_, "effluent_data");
					eff_ = dbEffSeries.fetch(dbModelIndexers_, modelRun_);

					/// Write out progress if we're debugging
					logger_.log("done");
				}

				if (modelRun_.hasNo3FertilizerFile())
				{
					/// Write out progress if we're debugging
					logger_.log("Fetching NO3 fertilizer data...", false);

					DbFertSeries dbFertSeries(session_, "no3_fertilizer_data");
					no3Fert_ = dbFertSeries.fetch(dbModelIndexers_, modelRun_, modelRun_.getNo3FertilizerFileId());

					/// Write out progress if we're debugging
					logger_.log("done");
				}

				if (modelRun_.hasNh4FertilizerFile())
				{
					/// Write out progress if we're debugging
					logger_.log("Fetching NH4 fertilizer data...", false);

					DbFertSeries dbFertSeries(session_, "nh4_fertilizer_data");
					nh4Fert_ = dbFertSeries.fetch(dbModelIndexers_, modelRun_, modelRun_.getNh4FertilizerFileId());

					/// Write out progress if we're debugging
					logger_.log("done");
				}

				if (modelRun_.hasDonFertilizerFile())
				{
					/// Write out progress if we're debugging
					logger_.log("Fetching DON fertilizer data...", false);

					DbFertSeries dbFertSeries(session_, "don_fertilizer_data");
					donFert_ = dbFertSeries.fetch(dbModelIndexers_, modelRun_, modelRun_.getDonFertilizerFileId());

					/// Write out progress if we're debugging
					logger_.log("done");
				}

				if (modelRun_.hasDepositionFile())
				{
					/// Write out progress if we're debugging
					logger_.log("Fetching deposition data...", false);

					DbDepSeries dbDepSeries(session_, "deposition_data");
					dep_ = dbDepSeries.fetch(dbModelIndexers_, modelRun_);

					/// Write out progress if we're debugging
					logger_.log("done");
				}
			}
			else
			{
				throw DbException(Error("Model indexers are not valid"), ExitCode::invalidIndexers);
			}
		}
		else
		{
			throw DbException(Error("Model meta data are not valid"), ExitCode::invalidModel);
		}
	}
	catch (const DbException &err)
	{
		throw err;
	}

	return true;
}

/**
 * Accepts a PERSiST model results set and stores it in the connected MySQL database
 *
 * @param[in]	results is the PERSiST model results object
 * @param[in]	output is the PERSiST CLI-compatible storage level
 * @param[in]	forceDelete says whether or not to delete results from tables
 * @throw		simple throw on any database error
 */
void DbConnection::storeResults(CResultsGroup& results, int output, bool forceDelete)
{
	/// Create a new database results object
	auto r = DbResults(schema_, session_, modelRun_);

	/// Try to store the supplied PERSiST results in the database
	try
	{
		r.storeResults(dbModelIndexers_, results, output, forceDelete);
	}
	catch (DbException& err)
	{
		throw err;
	}
}

bool DbConnection::isResultsAvailable(int outputLevel)
{
	/// Create a MySQL query to select landscape unit-indexed parameters from the database
	SqlStatement sql = session_->sql("SELECT is_result_level_available(?, ?) AS is_available");

	/// Try to execute the query
	SqlResult result;
	try
	{
		result = sql.bind(modelRun_.getId()).bind(outputLevel).execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::landParameterSqlError);
	}

	/// If the query executed and returned results...
	if (result.hasData())
	{
		/// Get the result
		auto row = result.fetchOne();

		return ((int)row[0] == 1);
	}

	return false;
}