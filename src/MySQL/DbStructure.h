#pragma once
#include <mysqlx/xdevapi.h>
#include <string>
#include "DbModelRun.h"
#include "DbModelIndexers.h"
#include "CStructure.h"

/**
 * The <code>DbStream</code> class stores the id, Strahler order and name of a stream
 * from the MySQL persist.stream table, to build a reach structure.
 *
 * @param[in]	id is the id of the stream in the persist.stream table
 * @param[in]	order is the Strahler order of the stream in the persist.stream table
 * @param[in]	name is the name of the stream in the persist.stream table
 */
class DbStream
{
public:
	DbStream(unsigned int id, unsigned int order, const std::string& name) : id_(id), order_(order), name_(name) {}
	~DbStream() {}

	/// Return the database id of the stream
	inline unsigned int getId() { return id_; }
	/// Return the Strahler order of the stream
	inline unsigned int getOrder() { return order_; }
	/// Return the name of the stream
	inline std::string getName() { return name_; }

private:
	unsigned int id_;		/// The stream id (in the database)
	unsigned int order_;	/// The Strahler order of the stream
	std::string name_;		/// The name of the stream
};

/**
 * The <code>DbStructure</code> class stores the stream/reach structure for
 * a PERSiST model run. Data are pulled from a MySQL database.
 *
 * @param[in]	session is the active database connection
 */
class DbStructure
{
public:
	DbStructure(std::shared_ptr<mysqlx::Session> session);
	~DbStructure();

	/**
	 * Retrieve stream/reach data from the MySQL database and build a PERSiST
	 * structure object, which is stored as a member variable
	 *
	 * @param[in]	modelRun defines the model run to use to grab the correct data
	 * @param[in]	indexers describes the model run indexers/indexes
	 * @return		a new PERSiST CStructure object
	*/
	CStructure fetch(DbModelRun& modelRun, DbModelIndexers &indexers);

private:
	std::shared_ptr<mysqlx::Session> session_;	/// The active database connection
	CStructure structure_;	/// The PERSiST structure object, created by the fetch function

	/**
	 * Retrieves stream data from the persist.stream table, to build the PERSiST strucutre object
	 *
	 * @param[in]	modelRun tells the function which data to grab
	 * @return		a vector of streams, being all the streams in the model run
	 */
	std::vector<DbStream> fetchStreams(DbModelRun& modelRun);

	/**
	 * Retrieves all the reaches for a stream, from persist.reach_structure
	 *
	 * @param[in]	streamId is the database ID of the stream for which to fetch reaches
	 * @param[in]	parameterSetId is the database ID of the parameter set for which to fetch reaches
	 * @param[in]	streamNetworkId is the database ID of the stream network for which to fetch reach inputs
	 * @return		a vector of PERSiST CReach objects, being all the reaches for the stream
	 */
	std::vector<CReach> fetchReaches(unsigned int streamId, unsigned int parameterSetId, unsigned int streamNetworkId);

	/**
	 * Retrieves all the inputs for a reach, from persist.reach_structure_inputs
	 *
	 * @param[in]	reachId is the database ID of the reach for which to fetch inputs
	 * @param[in]	streamNetworkId is the database ID of the stream network for which to fetch reach inputs
	 * @return		a vector of string, being the references for all the inputs to the reach
	 */
	std::vector<std::string> fetchReachInputs(unsigned int reachId, unsigned int streamNetworkId);

	/**
	 * Splits a string into a vector of tokens
	 *
	 * @param[in]	str is the string to be split
	 * @param[in]	delimiter is the character to split on
	 * @return		a vector of strings, being all the tokens from the original string
	 */
	std::vector<std::string> split(std::string str, char delimiter);
};
