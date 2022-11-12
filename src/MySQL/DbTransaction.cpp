#include "DbTransaction.h"
#include <ctime>
#include "DbException.h"

using namespace mysqlx;

DbTransaction::DbTransaction(std::shared_ptr<mysqlx::Session>& session) : session_(session),
																			logger_(Logger::GetInstance())
{
}

DbTransaction::~DbTransaction()
{
}

/**
 * Begin a transaction for INSERTing results, which might take a long time otherwise
 */
void DbTransaction::start()
{
	try
	{
		/// Should help make INSERTs faster for InnoDB tables
		SqlStatement sql = session_->sql("SET FOREIGN_KEY_CHECKS=0");
		SqlResult result = sql.execute();

		/// Should help make INSERTs faster for InnoDB tables
		sql = session_->sql("SET UNIQUE_CHECKS=0");
		result = sql.execute();

		/// Start the transaction
		session_->startTransaction();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::startTransaction);
	}
}

/**
 * COMMIT the transaction after the results have been INSERTed
 */
void DbTransaction::end(mysqlx::TableInsert& ti)
{
	logger_.log("COMMIT...", false);

	try
	{
		/// Start a timer so we can check performance of the COMMIT
		double duration;
		std::clock_t timer = std::clock();

		/// Execute the INSERT query
		ti.execute();

		/// COMMIT (and end the transaction)
		session_->commit();

		/// If we're in debug mode, write timing info to the console.
		duration = (std::clock() - timer) / (double)CLOCKS_PER_SEC;

		std::stringstream str;
		str << "completed in " << duration << " seconds";
		logger_.log(str.str());

		/// Turn the checks back on
		SqlStatement sql = session_->sql("SET FOREIGN_KEY_CHECKS=1");
		SqlResult result = sql.execute();

		sql = session_->sql("SET UNIQUE_CHECKS=1");
		result = sql.execute();
	}
	catch (Error& err)
	{
		throw DbException(err, ExitCode::commit);
	}
}
