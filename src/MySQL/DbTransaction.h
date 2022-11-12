/**
 * The <code>DbTransaction</code> class is a simple utility class to start/stop MySQL a transaction, via a supplied session.
 *
 * @param[in]	session is the MySQL connection
 */
#include <mysqlx/xdevapi.h>
#include "Logger.h"
#pragma once

class DbTransaction
{
public:
	DbTransaction(std::shared_ptr<mysqlx::Session>& session);
	~DbTransaction();

	/**
	 * Begin a transaction for INSERTing results, which might take a long time otherwise
	 */
	void start();

	/**
	 * COMMIT the transaction after the results have been INSERTed
	 */
	void end(mysqlx::TableInsert& ti);

private:
	std::shared_ptr<mysqlx::Session> session_;	/// A local copy of a pointer to the active database session
	Logger& logger_;		/// Logging object;

};

