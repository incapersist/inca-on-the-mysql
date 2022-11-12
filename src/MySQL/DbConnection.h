/**
 * The <code>DbConnection</code> class is an API to a MySQL database that stores PERSiST model inputs and results.
 *
 * As well as making and maintaining a connection to the database, DbConnection also retrieves input data and
 * stores results.
 *
 * @param[in]	user is the MySQL user name to use for the connection
 * @param[in]	password in the password for the user
 * @param[in]	host is the hostname for the MySQL server
 * @param[in]	port is the X Protocol port to use for the connection
 * @param[in]	schema is the name of the MySQL schema that contains the PERSiST data
 * @param[in]	isDebug, if true, writes debug info to the console
 */
#pragma once
#include <mysqlx/xdevapi.h>
#include <string>

#include "DbModelRun.h"
#include "DbModelIndexers.h"
#include "DbParameterSet.h"
#include "DbInputSeries.h"
#include "DbStructure.h"
#include "DbResults.h"
#include "DbAbsSeries.h"
#include "DbEffSeries.h"
#include "DbFertSeries.h"
#include "DbDepSeries.h"
#include "Logger.h"

#include "CResultsGroup.h"
#include "CAbsSeries.h"
#include "CEffSeries.h"
#include "CFertSeries.h"
#include "CDepSeries.h"
#include "CLandSeries.h"
#include "CGrowthSeries.h"

class DbConnection
{
public:
	/// Attempts to make a connection to a database using the supplied parameters
	DbConnection(	const std::string& user,
					const std::string& password,
					const std::string& host,
					unsigned int port,
					const std::string& schema,
					bool isDebug = false);

	/// Closes the connection
	~DbConnection();

	/**
	 * Fetches model input data from the database and stores.
	 * Fetched data includes reach structure, parameter set and driving data.
	 *
	 * @param[in]	modelRunId is the id of the model run in the database(persist.model_run.id)
	 *				for which to retrieve input data.
	 * @param[in]	forceSolarCalc is a flag that indicates whether solar radiation should always be calculated
	 * @return		boolean indicating success
	 * @throw		simple throw on any error
	 */
	bool fetchData(unsigned int modelRunId, bool forceSolarCalc);

	/**
	 * Accepts a PERSiST model results set and stores it in the connected MySQL database
	 *
	 * @param[in]	results is the PERSiST model results object
	 * @param[in]	output is the PERSiST CLI-compatible storage level
	 * @param[in]	forceDelete says whether or not to delete results from tables
	 * @throw		simple throw on any database error
	 */
	void storeResults(CResultsGroup& results, int output, bool forceDelete);

	bool isResultsAvailable(int outputLevel);

	/// Returns a copy of the reach structure that was retrieved from the database
	inline CStructure& getStructure() { return structure_;  }

	/// Returns a copy of the parameter set that was retrieved from the database
	inline CParSet& getParameterSet() { return parset_;  }

	/// Returns a copy of the input data that was retrieved from the database
	inline CInputSeriesContainer& getInputs() { return inputs_;  }

	/// Returns a copy of the abstraction data that was retrieved from the database
	inline std::vector<CAbsSeries*>& getAbstractions() { return abs_; }

	/// Returns a copy of the effluent data that was retrieved from the database
	inline std::vector<CEffSeries*>& getEffluent() { return eff_; }

	inline DbModelRun& getModelRun() { return modelRun_;  }

	inline CFertSeries* getNo3Fert() { return no3Fert_; }
	inline CFertSeries* getNh4Fert() { return nh4Fert_; }
	inline CFertSeries* getDonFert() { return donFert_; }
	inline CDepSeries* getDep() { return dep_; }
	inline CLandSeries& getLandPeriods() { return land_; }
	inline CGrowthSeries& getGrowth() { return growth_; }

	/// Returns a copy of the active database session
	inline std::shared_ptr<mysqlx::Session> session() { return session_; }

private:
	std::string user_;		/// The supplied MySQL username
	std::string password_;	/// The supplied MySQL password
	std::string host_;		/// The supplied Mysql server hostname
	unsigned int port_;		/// The supplied X Protocol port number
	std::string schema_;	/// The MySQL schema containing model data
	bool isDebug_;			/// Whether to output console message

	CStructure structure_;			/// Stores the reach strcuture that was retrieved from the database
	CParSet parset_;				/// Stores the parameter set that was retrieved from the database
	CInputSeriesContainer inputs_;	/// Stores the driving data that was retrieved from the database
	std::vector<CAbsSeries*> abs_;	/// Stores abstraction data that was retrieved from the database
	std::vector<CEffSeries*> eff_;	/// Stores effluent data that was retrieved from the database
	CFertSeries* no3Fert_;
	CFertSeries* nh4Fert_;
	CFertSeries* donFert_;
	CDepSeries* dep_;
	CLandSeries land_;
	CGrowthSeries growth_;

	std::shared_ptr<mysqlx::Session> session_;	/// Stores the active session
	DbModelRun modelRun_;						/// Stores the  model run id, used to access the correct data
	DbModelIndexers dbModelIndexers_;			/// Stores the indexers used in the model run

	Logger& logger_;		/// Logging object

	void Connect();	/// Connect to the database using the parameters supplied in the constructor
};

