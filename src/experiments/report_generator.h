#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <string>
#include <vector>
#include <fstream>
#include "experiment_runner.h"

class ReportGenerator {
public:
    ReportGenerator() = default;

    bool generateMarkdownReport(const std::vector<ExperimentResult>& results,
                               const std::string& output_path);

    bool exportToCSV(const std::vector<ExperimentResult>& results,
                    const std::string& csv_path);

    bool exportMatchDistances(const std::vector<ExperimentResult>& results,
                            const std::string& output_dir);

    bool saveExperimentResults(const std::vector<ExperimentResult>& results,
                              const std::string& output_dir);

private:
    std::string formatDuration(double ms) const;
    std::string generateSummaryTable(const std::vector<ExperimentResult>& results) const;
    std::string generateDetailedResults(const std::vector<ExperimentResult>& results) const;

    void writeCSVHeader(std::ofstream& file) const;
    void writeCSVRow(std::ofstream& file, const ExperimentResult& result) const;
};

#endif