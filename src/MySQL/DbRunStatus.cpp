#include "DbRunStatus.h"
#include <iostream>

using namespace mysqlx;

/**
 * The <code>DbRunStatus</code> class records model run progress and status in the MySQL database.
 *
 * @param[in]	session is the active MySQL connection
 * @param[in]	modelRunId is the DB ID of the model run for which to record status
 * @param[in]	threshold (%) describes how often to store model run progress (i.e. at every 10% change)
 */
DbRunStatus::DbRunStatus(std::shared_ptr<Session> session,
							unsigned int modelRunId,
							unsigned int threshold) :	session_(session),
														modelRunId_(modelRunId),
														threshold_((float)threshold)
{
	/// Reset the counter to zero
	lastPercentComplete_ = 0.0;

	/// Make sure that there are no progress records in the table for this model run
	reset();
}

DbRunStatus::~DbRunStatus()
{
}

/**
 * Delete from the database all run progress records for this model run
 */
void DbRunStatus::reset()
{
	/// Build a query object for the MySQL DELETE statement
	SqlStatement sql = session_->sql("DELETE FROM model_run_progress WHERE model_run_id = ?");

	/// Try to execute the query
	try
	{
		SqlResult result = sql.bind(modelRunId_).execute();
	}
	catch (mysqlx::Error& err)
	{
		throw;
	}
}

/**
 * Store the supplied run completion percentage for the model run. At the same time,
 * check if the run has been aborted by the user.
 *
 * @param[in]	percentComplete is the percentage amount to store in the database
 * @return		false if run has been aborted, true if not
 */
bool DbRunStatus::storeProgress(float percentComplete)
{
	bool ok = true;

	/// If the difference between the last STORED progress percentage and the current
	/// progress percentage is greater than the threshold...
	if ((percentComplete - lastPercentComplete_) > threshold_)
	{
		/// Add the current progress percentage to the database table
		insertProgress(percentComplete);

		/// Check if the run has been aborted by the user
		ok = !isAborted();

		/// Set the recorded last used percentage
		lastPercentComplete_ = percentComplete;
	}

	/// false if the run was aborted, true if not
	return ok;
}

/**
 * Store the supplied percentage in persist.model_run_progress
 *
 * @param[in]	percentComplete is the value to store in the table
 */
void DbRunStatus::insertProgress(float percentComplete)
{
	/// Build a query object for the MySQL INSERT statement
	SqlStatement sql = session_->sql("INSERT INTO model_run_progress (model_run_id, progress_percent) VALUES (?, ?)");

	/// Try to execute the query
	try
	{
		SqlResult result = sql.bind(modelRunId_).bind(percentComplete).execute();
	}
	catch (mysqlx::Error& err)
	{
		throw;
	}
}

/**
 * Check if the model run has been aborted by the user
 */
bool DbRunStatus::isAborted()
{
	/// Build a query object for the MySQL SELECT statement
	SqlStatement sql = session_->sql("SELECT id FROM model_run_status WHERE model_run_id = ? AND model_run_status_type_id = 2");

	/// Try to execute the query
	try
	{
		SqlResult result = sql.bind(modelRunId_).execute();

		/// Fetch the first record from the results (there should be at most one)
		auto row = result.fetchOne();

		/// Return the fetched result. If the abort status is not present in the database, then
		/// row = 0 which will be implicitly converted to false
		return row;
	}
	catch (mysqlx::Error& err)
	{
		throw;
	}
}

/**
 * Mark the model run as complete
 */
void DbRunStatus::complete()
{
	insertProgress(100.0);
}