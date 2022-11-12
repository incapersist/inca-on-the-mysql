//---------------------------------------------------------------------------
#include "CmdLine.h"
#include "CParSet.h"
#include "CInputSeries.h"
#include "CResultsGroup.h"
#include "CDepSeries.h"
#include "CFertSeries.h"
#include "CEffSeries.h"
#include "CAbsSeries.h"
#include "CGrowthSeries.h"
#include "CLandSeries.h"
#include "Model.h"
#include "Eqn.h"
#include "CInputSeriesContainer.h"
#include "version.h"
#include "CStructure.h"
#include "CObsContainer.h"
#include "utils.h"

#include <stdlib.h>
#include <cctype>
#include "MySQL/DbConnection.h"
#include "MySQL/ExitCodes.h"
#include "MySQL/DbRunStatus.h"
#include "MySQL/DbException.h"
#include "MySQL/Logger.h"
//---------------------------------------------------------------------------

void SortOutReaches(CInputSeriesContainer&, CParSet*, int*);
void ParseList(std::string, std::vector<std::string>&, int*, int);
void LoadSpatial(const char*, CInputSeriesContainer&, CParSet*, int*);
void LoadTimeSeriesFile(CInputSeriesContainer& inputs, CParSet* pars, std::string FileName);
void LoadObserved(std::string& FileName, CParSet* ParSet, CObsContainer& ObsSources);
void PrintError(void);
void PrintHelp(void);
void PrintVersion(void);
void updateLandModelProgress(float);
void updateReachModelProgress(float);
void resetModelProgress();
void storeModelProgress();

ExitCode RunClassic(CCmdLine& cmdLine, int output);
ExitCode RunMySql(CCmdLine& cmdLine, int output);

double landProgress = 0;
double reachProgress = 0;
double totalProgressCount = 0;
DbRunStatus *dbRunStatus = 0;

int main(int argc, char* argv[])
{
	// our cmd line parser object
	CCmdLine cmdLine;

	// parse argc,argv
	if (cmdLine.SplitLine(argc, argv) < 1)
	{
		// no switches were given on the command line, abort
		PrintError();
		return 1;
	}

	declaredStringType outstr;
	int output = 0;
    outstr = cmdLine.GetSafeArgument("-size", 0, "small");
    if (outstr == "none") output = -1;
    if (outstr == "small") output = 0;
    if (outstr == "medium") output = 1;
    if (outstr == "large") output = 2;
 
	ExitCode exitValue;

	if (cmdLine.HasSwitch("-mysql"))
	{
		exitValue = RunMySql(cmdLine, output);
	}
	else
	{
		exitValue = RunClassic(cmdLine, output);
	}

	return static_cast<int>(exitValue);
}

/**
 * Run the MySQL storage version of INCA-ON (THE)
 */
ExitCode RunMySql(CCmdLine& cmdLine, int output)
{
	// get the required arguments
	declaredStringType user, password, host, port, schema, runId;
	try
	{
		// if any of these fail, we'll end up in the catch() block
		user = cmdLine.GetArgument("-user", 0);
		password = cmdLine.GetArgument("-password", 0);
		host = cmdLine.GetArgument("-host", 0);
		port = cmdLine.GetArgument("-port", 0);
		schema = cmdLine.GetArgument("-schema", 0);
		runId = cmdLine.GetArgument("-runid", 0);
	}
	catch (...)
	{
		// one of the required arguments was missing, abort
		return ExitCode::cli;
	}

	bool isDebug = cmdLine.HasSwitch("-debug");
	bool forceDelete = cmdLine.HasSwitch("-delete");
	bool forceSolarCalc = cmdLine.HasSwitch("-force_solar_calc");

	Logger& logger = Logger::GetInstance();
	logger.setDebug(isDebug);

	/// Try to make a new connection to the database, using the parameters from the CLI
	DbConnection *connection = 0;
	try
	{
		connection = new DbConnection(user, password, host, std::stoi(port), schema, isDebug);
	}
	catch (...)
	{
		if (connection) delete connection;
		return ExitCode::connection;
	}

	/// Try to fetch all the available data for the model run identified by the CLI parameter
	bool isConnected = false;
	try
	{
		isConnected = connection->fetchData(std::stoi(runId), forceSolarCalc);
	}
	catch (DbException& err)
	{
		return err.getCode();
	}

	/// If successfully connected and fetched data...
	if (isConnected)
	{
		/// Check if st least the requested results are available
		if (connection->isResultsAvailable(output))
		{
			/// If results are already available and we're not forcing delete, exit
			if (!forceDelete)
			{
				return ExitCode::resultsExist;
			}
		}

		/// Get the INCA-ON (THE) objects for a parameter set, input data and structure
		auto structure = connection->getStructure();
		auto inputs = connection->getInputs();
		auto pars = connection->getParameterSet();
		auto AbstractionSeriesList = connection->getAbstractions();
		auto EffluentSeriesList = connection->getEffluent();
		auto no3fert = connection->getNo3Fert();
		auto nh4fert = connection->getNh4Fert();
		auto donfert = connection->getDonFert();
		auto dep = connection->getDep();
		auto LandPeriods = connection->getLandPeriods();
		auto grow = connection->getGrowth();

		/// Create a new INCA-ON (THE) results group object, using fetched data
		CResultsGroup *results = new CResultsGroup(pars.TimeSteps, pars.Reaches, &pars, &structure, pars.LandCount());

		/// Create a new model run status object, to record run progress and status
		dbRunStatus = new DbRunStatus(connection->session(), connection->getModelRun().getId());

		logger.log("Begin model run...", false);

		bool stopRun = false;
		bool precip = false;
		bool useSnow = true;
		bool useCoup = true;
		bool useGrowth = false;

		std::vector<std::string> reachLog;

		totalProgressCount = (pars.TimeSteps * pars.Reaches * 2);

		/// Try to run the model
		CModel *model = new CModel(pars);

		try
		{
			if (model->Run(results,
				no3fert, nh4fert, donfert, dep,
				EffluentSeriesList, AbstractionSeriesList,
				stopRun, inputs,
				&LandPeriods, &grow,
				&structure,
				&resetModelProgress,
				&updateLandModelProgress,
				&updateReachModelProgress,
				precip,
				useSnow,
				useCoup,
				connection->getModelRun().hasNo3FertilizerFile(),
				connection->getModelRun().hasNh4FertilizerFile(),
				connection->getModelRun().hasDonFertilizerFile(),
				useGrowth,
				connection->getModelRun().hasLandUsePeriodsFile(),
				connection->getModelRun().hasDepositionFile(),
				(AbstractionSeriesList.size() > 0),
				(EffluentSeriesList.size() > 0),
				reachLog,
				0))
			{
				logger.log("model run complete...", true);
				dbRunStatus->complete();
			}
			else
			{
				logger.log("FAIL", true);
				return ExitCode::abort;
			}
		}
		catch (...)
		{
			return ExitCode::run;
		}

		if (model) delete model;

		/// If we're in debug mode, write the fetched parameter set to an XML-format parameter file
		/// and write the model results to file
		if (isDebug)
		{
			logger.log("complete");
			logger.log("Saving parameters to XML file...", false);

			CStructure* structurePtr = &structure;

			pars.SaveToFile("INCA_ON_THE_MySQL.par", &structurePtr);
			pars.writeXml("INCA_ON_THE_MySQL.xml", &structurePtr);

			logger.log("done");
			logger.log("Saving results to file...", false);

			CObsSet obs;
			results->SaveToFile("INCA_ON_THE_MySQL", &pars, output, 1, 0);

			logger.log("done");
		}

		/// Optionally store the model results in the database
		if (output > -1)
		{
			try
			{
				connection->storeResults(*results, output, forceDelete);
			}
			catch (DbException& err)
			{
				return err.getCode();
			}
		}

		if (results) delete results;
	}

//	if (connection) delete connection;

	return ExitCode::success;
}

ExitCode RunClassic(CCmdLine& cmdLine, int output)
{
	if (cmdLine.HasSwitch("-help"))
	{
		PrintHelp();
		return ExitCode::success;
	}

	if (cmdLine.HasSwitch("-version"))
	{
		PrintVersion();
		return ExitCode::success;
	}

    // get the required arguments
    declaredStringType parfile, datfile;
    try
    {
        // if any of these fail, we'll end up in the catch() block
        parfile = cmdLine.GetArgument("-par", 0);
        datfile = cmdLine.GetArgument("-dat", 0);
    }
    catch (...)
    {
        // one of the required arguments was missing, abort
        PrintError();
		return ExitCode::cli;
	}

    // get the optional parameters

    // test for the presence of the '-headers' and '-noheaders' switch
	int headers;
    if (cmdLine.HasSwitch("-h")) headers = 1;
    if (cmdLine.HasSwitch("-noh")) headers = 0;

	// test for the presence of the '-xml' switch
	bool xmlFormat = false;
	if (cmdLine.HasSwitch("-XML")) xmlFormat = true;

    // convert to an int, default to '100'
	declaredStringType precip_option;
    precip_option = cmdLine.GetSafeArgument("-precip", 0, "AP");

	int precip = 2;
    if (precip_option == "HER") precip = 1;
    if (precip_option == "AP") precip = 2;
    if (precip_option == "hybrid") precip = 3;

    // convert to an int, default to '100'
	declaredStringType results_option;
    results_option = cmdLine.GetSafeArgument("-grouping", 0, "All");

	unsigned int resultsOption = 0;
    if (results_option == "all") resultsOption = 0;
    if (results_option == "stream") resultsOption = 1;
    if (results_option == "reach") resultsOption = 2;

	declaredStringType outfile, sfertfile, lfertfile, donfertfile, growfile, obsfile;
	declaredStringType spatialfile, depfile, landfile, structurefile;
    outfile = cmdLine.GetSafeArgument("-out", 0, "inca_out");
    sfertfile = cmdLine.GetSafeArgument("-no3", 0, "");
    lfertfile = cmdLine.GetSafeArgument("-nh4", 0, "");
    donfertfile = cmdLine.GetSafeArgument("-don", 0, "");
    growfile = cmdLine.GetSafeArgument("-grow", 0, "");
    depfile = cmdLine.GetSafeArgument("-dep", 0, "");
    landfile = cmdLine.GetSafeArgument("-land", 0, "");
    spatialfile = cmdLine.GetSafeArgument("-spatial", 0, "");
    structurefile = cmdLine.GetSafeArgument("-structure", 0, "");
    obsfile = cmdLine.GetSafeArgument("-obs", 0, "");

	declaredStringType errfile = cmdLine.GetSafeArgument("-errfile", 0, "stderr");

	declaredStringType log_fname, run_num, calc_E, splash_hybrid;
    log_fname = cmdLine.GetSafeArgument("-log", 0, "");

	int log = 0;
    if (log_fname != "") log = 1;

    run_num = cmdLine.GetSafeArgument("-ref", 0, "");

	int force_results = 0;
    if (cmdLine.HasSwitch("-res")) force_results = 1;

    // Open parameter file
    CParSet *pars = new CParSet;

    CStructure *structure = NULL;
    if (structurefile != "")
    {
        delete structure;
        structure = new CStructure;

        std::ifstream in(structurefile.c_str());
        in >> *structure;
    }

	if (xmlFormat) pars->readXml(parfile.c_str(), &structure);
	else pars->LoadFromFile(parfile.c_str(), &structure);

    int *spatial = new int[pars->Reaches];
	CInputSeriesContainer inputs;

    if (spatialfile != "")
    {
        LoadSpatial(spatialfile.c_str(), inputs, pars, spatial);
    }
    else
    {
        // Open time series file
        LoadTimeSeriesFile(inputs, pars, datfile);
    }

    SortOutReaches(inputs, pars, spatial);

	CResultsGroup* results = new CResultsGroup(pars->TimeSteps, pars->Reaches,
                                pars, structure, pars->LandCount());

	CFertSeries* sfert = NULL;
	if (sfertfile != "")
    {
        sfert = new CFertSeries(pars->TimeSteps, (int)pars->LandCount());
        sfert->LoadFromFile(sfertfile.c_str());
    }

	CFertSeries* lfert = NULL;
	if (lfertfile != "")
    {
        lfert = new CFertSeries(pars->TimeSteps, (int)pars->LandCount());
        lfert->LoadFromFile(lfertfile.c_str());
    }

	CFertSeries* donfert = NULL;
	if (donfertfile != "")
    {
        donfert = new CFertSeries(pars->TimeSteps, (int)pars->LandCount());
        donfert->LoadFromFile(donfertfile.c_str());
    }

	std::vector<CEffSeries*> EffluentSeriesList;
	EffluentSeriesList.clear();

    for (int i=0; i<cmdLine.GetArgumentCount("-eff"); ++i)
    {
		declaredStringType efffile = cmdLine.GetSafeArgument("-eff", i, "");

        CEffSeries* eff = new CEffSeries(pars->TimeSteps, 3);
        eff->LoadFromFile(efffile.c_str());

        EffluentSeriesList.push_back(eff);
    }

	std::vector<CAbsSeries*> AbstractionSeriesList;
	AbstractionSeriesList.clear();

    for (int i=0; i<cmdLine.GetArgumentCount("-abs"); ++i)
    {
		declaredStringType absfile = cmdLine.GetSafeArgument("-abs", i, "");

        CAbsSeries* abs = new CAbsSeries(pars->TimeSteps);
        abs->LoadFromFile(absfile.c_str());

        AbstractionSeriesList.push_back(abs);
    }

	CGrowthSeries* grow = NULL;
	if (growfile != "")
    {
        grow = new CGrowthSeries(pars->LandCount());
        grow->LoadFromFile(growfile.c_str());
    }

	CDepSeries* dep = NULL;
	if (depfile != "")
    {
        dep = new CDepSeries(pars->TimeSteps, 4);
        dep->LoadFromFile(depfile.c_str());
    }

	CLandSeries* LandPeriods = NULL;
	if (landfile != "")
    {
        LandPeriods = new CLandSeries(landfile.c_str(), pars->LandCount());
    }

	bool writeCoefficients = false;
	CObsContainer ObsSources;
	if (obsfile != "")
    {
        LoadObserved(obsfile, pars, ObsSources);
        writeCoefficients = true;
    }

    CModel *model = new CModel(*pars);
	bool stopRun = false;

	bool useSnow = false;
	bool useCoup = false;
	bool useNo3Fert = false;
	bool useNh4Fert = false;
	bool useDonFert = false;
	bool useGrowth = false;
	bool useLand = false;
	bool useDep = false;
	bool useEff = false;
	bool useAbs = false;

	std::vector<std::string> reachLog;

    try
    {
    	if (model->Run(results,
                            sfert, lfert, donfert, dep,
                            EffluentSeriesList, AbstractionSeriesList,
                            stopRun, inputs,
                            LandPeriods, grow,
                            structure,
							&resetModelProgress,
							&updateLandModelProgress,
							&updateReachModelProgress,
							precip,
							useSnow,
							useCoup,
							useNo3Fert,
							useNh4Fert,
							useDonFert,
							useGrowth,
							useLand,
							useDep,
							useEff,
							useAbs,
							reachLog,
							0))

        {
            if (writeCoefficients)
            {
                results->CalcErrors(pars, -1, true, ObsSources);
            }

            results->SaveToFile(outfile.c_str(), pars, output, headers, resultsOption);
        }
	}
    catch(EqnException& E)
    {
        if (force_results)
        {
            results->SaveToFile(outfile.c_str(), pars, output, headers, resultsOption);
        }

        if (log)
        {
            std::ofstream logfile(log_fname.c_str(), std::ios_base::app);
            logfile << "**************************************************\n\n";
            logfile << "Run " << run_num << std::endl << std::endl;
            logfile << E << std::endl;
        }

		return ExitCode::run;
    }
    catch(...)
    {
        if (force_results)
        {
            results->SaveToFile(outfile.c_str(), pars, output, headers, resultsOption);
        }

        if (log)
        {
            std::ofstream logfile(log_fname.c_str(), std::ios_base::app);
            logfile << "**************************************************\n\n";
            logfile << "Run " << run_num << std::endl << std::endl;
            logfile << "Unexpected floating-point error encountered\n";
        }

		return ExitCode::run;
	}

    delete model;
    delete sfert;
    delete lfert;
    delete donfert;

    for (unsigned int i=0; i<EffluentSeriesList.size(); i++)
    {
        CEffSeries* eff = EffluentSeriesList[i];
        delete eff;
    }

    for (unsigned int i=0; i<AbstractionSeriesList.size(); i++)
    {
        CAbsSeries* abs = AbstractionSeriesList[i];
        delete abs;
    }

    delete grow;
    delete dep;
    delete LandPeriods;
    delete results;
    delete pars;
    delete []spatial;

	return ExitCode::success;

}
//---------------------------------------------------------------------------
void LoadTimeSeriesFile(CInputSeriesContainer& inputs, CParSet* pars, std::string FileName)
{
 //       CInputSeries *InputSeries = new CInputSeries(pars->TimeSteps, 5, pars->Reaches, FileName.c_str());

 //       if (InputSeries->LoadFromFile(FileName.c_str()))
		if (inputs.LoadFromFile(FileName.c_str(), pars))
		{
    //        std::string fname(FileName.c_str());

    //        if (InputSeries->Missing())
    //        {
    //            reachIter it = pars->ReachPar.begin();
    //            InputSeries->CalcSolar(it->second.Par[9],
    //                                    it->second.Par[10],
    //                                    pars->StartDate);
    //        }

   //         InputSeries->Name(fname);
   //         inputs.Add(InputSeries);
        }
}
//---------------------------------------------------------------------------
void LoadSpatial(   const char* FileName,
                    CInputSeriesContainer& inputs,
                    CParSet* pars,
                    int* spatial)
{
    std::ifstream file(FileName);
    std::string list;
    std::string path;

    inputs.clear();

    while (std::getline(file, path))
    {
        LoadTimeSeriesFile(inputs, pars, utils::trim(path));

        std::getline(file, list);

        unsigned int count = inputs.size();
        inputs[count - 1]->SetReaches(list);
    }



}
//---------------------------------------------------------------------------

void ParseList(std::string list, std::vector<std::string>& nums,
                int* spatial, int index)
{
    std::string numstr;

    nums.clear();

    for (std::string::size_type i = 0; i <= list.size(); ++i)
    {
        if (std::isdigit(list[i]))
        {
            numstr.push_back(list[i]);
        }
        else
        {
            nums.push_back(numstr);

            int num;
            std::istringstream istr(numstr);
            istr >> num;
            spatial[num - 1] = index;

            numstr.clear();
        }
    }
}
//---------------------------------------------------------------------------
void SortOutReaches(CInputSeriesContainer& inputs, CParSet* pars, int* spatial)
{
    int num;

    if (inputs.size() > 1)
    {
        for (unsigned int i=0; i<inputs.size(); i++)
        {
            for (unsigned int j=0; j<inputs[i]->Reaches.size(); j++)
            {
                sscanf(inputs[i]->Reaches[j].c_str(), "%d", &num);
                spatial[num-1] = i;
            }
        }
    }
    else
    {
        for (int i=0; i<pars->Reaches; i++)
        {
            spatial[i] = 0;
        }
    }
}
//---------------------------------------------------------------------------

void LoadObserved(std::string& FileName, CParSet* ParSet, CObsContainer& ObsSources)
{
    if (ParSet->TimeSteps)
    {
        if (ObsSources.size() > 0)
        {
            ObsSources.clear();
        }

    }
}
//---------------------------------------------------------------------------
void PrintHelp(void)
{
    std::cout << "Helper commands (no model run)" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "-help" << std::endl;
    std::cout << "-version" << std::endl;

    std::cout << std::endl;

    std::cout << "Required parameters" << std::endl;
    std::cout << "-------------------" << std::endl;
    std::cout << "-par <filename>" << std::endl;
    std::cout << "-dat <filename>" << std::endl;

    std::cout << std::endl;

    std::cout << "Optional parameters" << std::endl;
    std::cout << "-------------------" << std::endl;
    std::cout << "[-size <none | small | medium | large | all>]" << std::endl;
    std::cout << "[-precip <HER | AP | hybrid>]" << std::endl;
    std::cout << "[-grouping <all | stream | reach>]" << std::endl;
    std::cout << "[-h | -noh]" << std::endl;
    std::cout << "[-XML]" << std::endl;
    std::cout << "[-out <filename>]" << std::endl;
    std::cout << std::endl;
    std::cout << "[-dep <filename>]" << std::endl;
    std::cout << "[-no3 <filename>]" << std::endl;
    std::cout << "[-nh4 <filename>]" << std::endl;
    std::cout << "[-don <filename>]" << std::endl;
    std::cout << "[-eff <filename>]" << std::endl;
    std::cout << "[-abs <filename>]" << std::endl;
    std::cout << "[-grow <filename>]" << std::endl;
    std::cout << "[-land <filename>]" << std::endl;
    std::cout << std::endl;
    std::cout << "[-obs <filename>]" << std::endl;
    std::cout << "[-structure <filename>]" << std::endl;
    std::cout << "[-spatial <filename>]" << std::endl;
    std::cout << std::endl;
    std::cout << "[-log <filename>]" << std::endl;
    std::cout << "[-ref <number>]" << std::endl;
    std::cout << "[-res]" << std::endl;
    std::cout << "[-errfile <stderr | NULL | filename>]" << std::endl;
}
//---------------------------------------------------------------------------
void PrintError(void)
{
    std::cerr << "You did not supply the required command-line parameters.";
    std::cerr << std::endl;
    std::cerr << "Try: inca_on_the_cmd -help";
    std::cerr << std::endl << std::endl;

    PrintHelp();
}
//---------------------------------------------------------------------------
void PrintVersion(void)
{
    std::cout << "INCA-ON (THE) v";
    std::cout << VERSION;
    std::cout << std::endl;
}
//---------------------------------------------------------------------------
void updateLandModelProgress(float steps)
{
	landProgress = steps;
	storeModelProgress();
}
//---------------------------------------------------------------------------
void updateReachModelProgress(float steps)
{
	reachProgress = steps;
	storeModelProgress();
}
//---------------------------------------------------------------------------
void resetModelProgress()
{
	landProgress = 0;
	reachProgress = 0;
}
//---------------------------------------------------------------------------
void storeModelProgress()
{
	if (dbRunStatus && totalProgressCount > 0)
	{
		dbRunStatus->storeProgress(((landProgress + reachProgress) / totalProgressCount) * 100.0);
	}
}
//---------------------------------------------------------------------------
