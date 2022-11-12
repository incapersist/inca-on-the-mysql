/**
 * The <code>DbModelRun</code> class stores a single row of database model run meta data.
 */
#pragma once
#include <mysqlx/xdevapi.h>
#include "DATECL.h"
#include "Logger.h"

class DbModelRun
{
public:
	DbModelRun();
	~DbModelRun();

	/**
	 * Fetch a row from the persist.model_run table in MySQL
	 *
	 * @param[in]	session is the current active database connection
	 * @param[in]	modelRunId is the id of the model run meta data to fetch
	*/
	void fetch(std::shared_ptr<mysqlx::Session> session, unsigned int modelRunId);

	// Returns the stored ID for the model run
	inline unsigned int getId() const { return id_; }

	// Returns the stored parameter set ID for the model run
	inline unsigned int getParameterSetId() const { return parameterSetId_; }

	// Returns the stored input file ID for the model run
	inline unsigned int getInputFileId() const { return hydrologyInputFileId_; }

	// Returns the stored abstraction file ID for the model run
	inline unsigned int getAbstractionFileId() const { return abstractionInputFileId_;  }

	// Returns the stored effluent file ID for the model run
	inline unsigned int getEffluentFileId() const { return effluentInputFileId_; }

	// Returns the stored NO3 fertilizer file ID for the model run
	inline unsigned int getNo3FertilizerFileId() const { return no3FertilizerInputFileId_; }

	// Returns the stored NH4 fertilizer file ID for the model run
	inline unsigned int getNh4FertilizerFileId() const { return nh4FertilizerInputFileId_; }

	// Returns the stored DON fertilizer file ID for the model run
	inline unsigned int getDonFertilizerFileId() const { return donFertilizerInputFileId_; }

	// Returns the stored deposition file ID for the model run
	inline unsigned int getDepositionFileId() const { return depositionInputFileId_; }

	// Returns the stored land use periods file ID for the model run
	inline unsigned int getLandUsePeriodsFileId() const { return landUsePeriodsInputFileId_; }

	// Returns the stored stream network ID for the model run
	inline unsigned int getStreamNetworkId() const { return streamNetworkId_; }

	// Return the number of timesteps described by this model run
	inline unsigned int getTimeSteps() const { return timesteps_;  }

	inline dateCl::Date getStartDate() { return dateFromString(startDate_); }
	inline unsigned int getOffset() const { return offset_; }

	// Check if the model run is valid
	bool isValid();

	// Check if the model run has a parameter file attached
	bool hasParameterFile();

	// Check if the model run has an input data file attached
	bool hasHydrologyFile();

	// Check if the model run has an observed data file attached
	bool hasObservedFile();

	// Check if the model run has a reach structure file attached
	bool hasStructureFile();

	// Check if the model run has an abstraction file attached
	bool hasAbstractionFile();

	// Check if the model run has an effluent file attached
	bool hasEffluentFile();

	// Check if the model run has a NO3 fertilizer file attached
	bool hasNo3FertilizerFile();

	// Check if the model run has an NH4 fertilizer file attached
	bool hasNh4FertilizerFile();

	// Check if the model run has a DON fertilizer file attached
	bool hasDonFertilizerFile();

	// Check if the model run has a deposition file attached
	bool hasDepositionFile();

	// Check if the model run has a land use periods file attached
	bool hasLandUsePeriodsFile();

	// Find out if the reach structure was auto-generated for this model run
	inline bool isGeneratedStructure() { return isStructureGenerated_;  }

private:
	unsigned int id_;							/// Stores model_run.id
	unsigned int parameterSetId_;				/// Stores model_run.parmeter_set_id
	unsigned int hydrologyInputFileId_;			/// Stores model_run.input_hydrology_id
	unsigned int observedInputFileId_;			/// Stores model_run.observed_input_file_id
	unsigned int effluentInputFileId_;			/// Stores model_run.effluent_input_file_id
	unsigned int abstractionInputFileId_;		/// Stores model_run.abstraction_input_file_id
	unsigned int no3FertilizerInputFileId_;		/// Stores model_run.no3_fertilizer_input_file_id
	unsigned int nh4FertilizerInputFileId_;		/// Stores model_run.nh4_fertilizer_input_file_id
	unsigned int donFertilizerInputFileId_;		/// Stores model_run.don_fertilizer_input_file_id
	unsigned int depositionInputFileId_;		/// Stores model_run.deposition_input_file_id
	unsigned int landUsePeriodsInputFileId_;	/// Stores model_run.land_use_periods_input_file_id
	unsigned int streamNetworkId_;				/// Stores model_run.stream_network_id
	bool isStructureGenerated_;					/// Flag to indicate if the reach structure has been auto-generated
	unsigned int userId_;						/// Who created the model run
	std::string reference_;						/// User-supplied description of the model run
	std::string createDate_;					/// When the model run was created
	std::string startDate_;						/// Start date for the model run
	unsigned int timesteps_;					/// Run the model for this number of timesteps
	unsigned int offset_;
	unsigned int availableTimesteps_;

	Logger& logger_;	/// Logging object

	/**
	 * Converts a string representation of a date to a date object
	 *
	 * @param[in]	startDate is the string representation of the parameter set start date
	 * @return		a new date object
	 */
	dateCl::Date dateFromString(std::string& startDate);
};

