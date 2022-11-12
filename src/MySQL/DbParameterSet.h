/**
 * The <code>DbParameterSet</code> class fetches a complete PERSiST parameter set from a MySQL database.
 *
 * @param[in]	session is the active MySQL database connection
 */
#pragma once
#include <string>
#include <mysqlx/xdevapi.h>
#include "DbModelRun.h"
#include "DbModelIndexers.h"
#include "CParSet.h"

class DbParameterSet
{
public:
	DbParameterSet(std::shared_ptr<mysqlx::Session> session);
	~DbParameterSet();

	/**
	 * Retrieve a complete PERSiST parameter set from a MySQL database
	 *
	 * @param[in]	modelRun is the meta data for the model run to use
	 * @param[in]	indexers is a complete set of indexes for the model run
	 * @return		a new PERSiST parameter set object
	 */
	CParSet fetch(DbModelRun& modelRun, DbModelIndexers &indexers);

private:
	std::shared_ptr<mysqlx::Session> session_;	/// The active database connection
	CParSet parset_;	/// Parameters fetched from the database will be stored here

	/**
	 * The following member variables store the meta data from the persist.parameter_set table
	 * for the parameter set identified by the modelRun object supplied to the fetch function
	 */
	unsigned int id_;					/// The parameter set ID
	unsigned int userId_;				/// The user token string
	std::string reference_;				/// The parameter set reference (name/description)
	std::string createDate_;			/// The parameter set creation date
	std::string startDate_;				/// The start date of the model run
	unsigned int timesteps_;			/// The number of time steps in the model run
	unsigned int modelVersionId_;		/// The parameter set is for this model version
	unsigned int modelVersionCoreId_;	/// The parameter set is for this model core version
	double baselineSmd_;				/// The baseline SMD parameters (used in calculating INCA inputs)
	unsigned int stepSize_;				/// The parameter set is for this model core version

	/**
	 * Fetch meta data for a parameter set
	 *
	 * @param[in]	paraeterSetId is the ID off the parameter set for which to fetch meta data
	 */
	void fetchMeta(unsigned int parameterSetId);

	/**
	 * Fetch the parameters, indexed by landscape units, from the persist.parameter_set_value_land database table
	 * and add the fetched parameters to the PERSiST parameter set object
	 *
	 * @param[in]	modelLandId is the identifier that PERSiST uses for a particular landscape unit
	 * @param[in]	dbLandId is the database ID of the same landscape unit
	 */
	void fetchLandParameters(unsigned int parameterSetId, unsigned int modelLandId, unsigned int dbLandId);

	/**
	 * Fetch the parameters, indexed by reach, from the persist.parameter_set_value_reach database table
	 *
	 * @param[in]	dbReachId is the database ID of the reach for which to fetch parameters
	 */
	void fetchReachParameters(unsigned int parameterSetId, unsigned int dbReachId, CReachPar& r, CSubPar& s);

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
	void fetchReachLandParameters(CSubPar& r, unsigned int parameterSetId, unsigned int dbReachId, unsigned int modelLandId, unsigned int dbLandId);
	void fetchStreamParameters(unsigned int parameterSetId, unsigned int modelStreamId, unsigned int dbStreamId);

	/**
	 * Converts a string representation of a date to a date object
	 *
	 * @param[in]	startDate is the string representation of the parameter set start date
	 * @return		a new date object
	 */
	dateCl::Date dateFromString(std::string& startDate);
};

