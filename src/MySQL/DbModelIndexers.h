#pragma once
#include <string>
#include <vector>
#include <map>
#include <mysqlx/xdevapi.h>

/**
 * The <code>DbModelIndex</code> class stores the id and name of a parameter set 
 * index from the MySQL persist.parameter_set_iterator table.
 *
 * @param[in]	id is the id of the index in the persist.parameter_set_iterator table
 * @param[in]	name is the name of the index in the persist.parameter_set_iterator table
 */
class DbModelIndex
{
public:
	DbModelIndex(unsigned int id, const std::string& name) : id_(id), name_(name) {}
	~DbModelIndex() {}

	/// Return a copy of this index's database ID
	unsigned int getId() { return id_;  }

	/// Return a copy of this index's database name
	std::string getName() { return name_; }

private:
	unsigned int id_;	/// The database ID of the index
	std::string name_;	/// The database name of the index
};

/**
 * The <code>DbModelIndexers</code> class stores - and provides access to - a vector of DbModelIndex.
 * Therefore, this represents all of the indexers in a PERSiST model run
*/
class DbModelIndexers
{
public:
	DbModelIndexers();
	~DbModelIndexers();

	/**
	 * Return the indexes for the provided indexer name. For exmaple, if the string 'reach'
	 * is passed, then the function will return all of the reach indexes
	 *
	 * @param[in]	needle is the 'key' indexer name
	 * @indexerName		vector of DbModelIndex for indexer 'needle'
	 */
	std::vector<DbModelIndex>& operator[] (const std::string& indexerName);

	/**
	 * Fetch (and store internally) all model indexers from the MySQL database for a particular model run
	 * 
	 * @param[in]	session is the active database connection
	 * @param[in]	modelRun identifies the correct model run to use
	 */
	void fetch(std::shared_ptr<mysqlx::Session> session, unsigned int modelRunId);

	/// Check if the retrieved model indexers are valid
	bool isValid() { return true; }

private:
	/**
	 * Retrieves all the indexers from the database for a particular indexer for a distinct model run
	 *
	 * @param[in]	session is the active database connection
	 * @param[in]	modelRunId is the model run id for which to retrieve indexers
	 * @param[in]	iteratorId id the DB ID of the interator for which to retrieve indexers
	 * @return		a vector of all the indexers for the iterator
	 */
	std::vector<DbModelIndex> getIndexes(std::shared_ptr<mysqlx::Session>& session, unsigned int modelRunId, unsigned int iteratorId);

	/// Stores all the indexers, as retrieved from the database, for a model run
	std::map < std::string, std::vector<DbModelIndex> > indexers_;
};

