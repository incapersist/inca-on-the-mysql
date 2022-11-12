#pragma once
#include <mysqlx/xdevapi.h>

/**
 * The <code>DbRunStatus</code> class records model run progress and status in the MySQL database.
 *
 * @param[in]	session is the active MySQL connection
 * @param[in]	modelRunId is the DB ID of the model run for which to record status
 * @param[in]	threshold (%) describes how often to store model run progress (i.e. at every 10% change)
 */
class DbRunStatus
{
public:
	DbRunStatus(std::shared_ptr<mysqlx::Session> session, unsigned int modelRunId, unsigned int threshold = 10);
	~DbRunStatus();

	/**
	 * Store the supplied run completion percentage for the model run. At the same time,
	 * check if the run has been aborted by the user.
	 *
	 * @param[in]	percentComplete is the percentage amount to store in the database
	 * @return		false if run has been aborted, true if not
	 */
	bool storeProgress(float percentComplete);

	/**
	 * Mark the model run as complete
	 */
	void complete();

private:
	std::shared_ptr<mysqlx::Session> session_;	/// A pointer to the active MySQL connection

	unsigned int modelRunId_;	/// The Id of the model run for which to store progress/status
	float lastPercentComplete_;	/// Keeps track of the last requested progress percent - used to check threshold
	float threshold_;			/// How often to store progress (i.e. every 10%)

	/**
	 * Store the supplied percentage in persist.model_run_progress
	 *
	 * @param[in]	percentComplete is the value to store in the table
	 */
	void insertProgress(float percentComplete);

	/**
	 * Delete from the database all run progress records for this model run
	 */
	void reset();

	/**
	 * Check if the model run has been aborted by the user
	 */
	bool isAborted();
};

